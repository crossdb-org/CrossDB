/******************************************************************************
* Copyright (c) 2024-present JC Wang. All rights reserved
*
*   https://crossdb.org
*   https://github.com/crossdb-org/crossdb
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
******************************************************************************/

#if XDB_LOG_FLAGS & XDB_LOG_IDX
#define xdb_fkeylog(...)	xdb_print(__VA_ARGS__)
#else
#define xdb_fkeylog(...)
#endif

XDB_STATIC int 
xdb_create_fkey (xdb_stmt_fkey_t *pStmt)
{
	xdb_conn_t		*pConn = pStmt->pConn;

	xdb_dbglog ("create fkeys %s on %s %d", pStmt->fkey_name, XDB_OBJ_NAME(pStmt->pTblm), pStmt->fld_count);

	int rc = -1;
	xdb_tblm_t	*pTblm = pStmt->pTblm;
	xdb_fkeym_t *pFkeym = NULL;

	XDB_EXPECT (XDB_OBJM_COUNT (pTblm->fkey_objm) < XDB_MAX_FKEY, XDB_E_FULL, "Can create at most %d foreign keys", XDB_MAX_FKEY);

	pFkeym = xdb_calloc (sizeof(xdb_fkeym_t) * 2);
	if (NULL == pFkeym) {
		goto error;
	}

	pFkeym->pTblm = pStmt->pTblm;
	pFkeym->pRefTblm = pStmt->pRefTblm;

	pFkeym->on_del_act = pStmt->on_del_act;
	pFkeym->on_upd_act = pStmt->on_upd_act;
	pFkeym->fld_count  = pStmt->fld_count;

	uint8_t	fld_bmp[XDB_MAX_COLUMN/8];
	memset (fld_bmp, 0, (pStmt->pRefTblm->fld_count+7)>>3);
	for (int i = 0; i < pFkeym->fld_count; ++i) {
		xdb_field_t 	*pField = xdb_find_field (pStmt->pTblm, pStmt->fkey_col[i], 0);
		XDB_EXPECT (pField != NULL, XDB_E_STMT, "Can't find field '%s'", pStmt->fkey_col[i]);
		pFkeym->filter.pFilters[i] = &pFkeym->fkey_filters[i];
		pFkeym->filter.pFilters[i]->val.pField = pField;
		pFkeym->filter.pFilters[i]->pField = pStmt->pRefFlds[i];
		pFkeym->filter.pFilters[i]->cmp_op = XDB_TOK_EQ;
		uint16_t ref_fld_id = pStmt->pRefFlds[i]->fld_id;
		fld_bmp[ref_fld_id>>3] |= (1<<(ref_fld_id&7));
	}
	pFkeym->filter.filter_count = pFkeym->fld_count;
	xdb_find_idx (pStmt->pRefTblm, &pFkeym->filter, fld_bmp);

	xdb_strcpy (XDB_OBJ_NAME(pFkeym), pStmt->fkey_name);
	XDB_OBJ_ID(pFkeym) = -1;
	xdb_objm_add (&pTblm->fkey_objm, pFkeym);

	xdb_fkeym_t *pRefFkeym = pFkeym + 1;
	*pRefFkeym = *pFkeym;
	if (strlen (pFkeym->obj.obj_name) < XDB_NAME_LEN - 16) {
		sprintf (pRefFkeym->obj.obj_name, "%s_%d", pFkeym->obj.obj_name, XDB_OBJ_ID(pTblm));
	}
	XDB_OBJ_ID(pRefFkeym) = -1;
	// registered on the REFERENCED (parent) table so a DELETE there can find
	// which child tables/rows still reference it
	xdb_objm_add (&pStmt->pRefTblm->fkeyref_objm, pRefFkeym);

	return XDB_OK;

error:
	xdb_free (pFkeym);
	return rc;
}

XDB_STATIC int
xdb_fkey_insert_check (xdb_conn_t *pConn, xdb_tblm_t *pTblm, void *pRow)
{
	for (int i = 0; i < XDB_OBJM_COUNT(pTblm->fkey_objm); ++i) {
		xdb_fkeym_t *pFkeym = XDB_OBJM_GET (pTblm->fkey_objm, i);
		for (int j = 0; j < pFkeym->fld_count; ++j) {
			xdb_value_t *pVal = &pFkeym->filter.pFilters[j]->val;
			xdb_row_getVal (pRow, pVal);
			pVal->val_type = pVal->sup_type;
		}

		xdb_rowset_t row_set;
		xdb_rowset_init (&row_set);
		row_set.limit	= 1;
		xdb_sql_query2 (pConn, pFkeym->pRefTblm, &row_set, &pFkeym->filter);
		XDB_EXPECT (row_set.count > 0, XDB_E_CONSTRAINT, "Foreign Key doesn't exist in '%s'", XDB_OBJ_NAME(pFkeym->pRefTblm));
	}

	return XDB_OK;

error:
	return -1;
}

XDB_STATIC int
xdb_fkey_delete_check (xdb_conn_t *pConn, xdb_tblm_t *pTblm, void *pRow)
{
	for (int i = 0; i < XDB_OBJM_COUNT(pTblm->fkeyref_objm); ++i) {
		xdb_fkeym_t *pRefFkeym = XDB_OBJM_GET (pTblm->fkeyref_objm, i);
		xdb_filter_t	filters[XDB_MAX_MATCH_COL], *pFilters[XDB_MAX_MATCH_COL];
		xdb_singfilter_t sigFlt;

		// the stored filter matches child->parent (insert-check direction);
		// swap field roles to match parent->child for the delete-check
		for (int j = 0; j < pRefFkeym->fld_count; ++j) {
			xdb_filter_t *pFldFlt = pRefFkeym->filter.pFilters[j];
			filters[j].cmp_op = XDB_TOK_EQ;
			filters[j].pField = pFldFlt->val.pField; // child's FK column
			filters[j].val.pField = pFldFlt->pField; // parent's referenced column
			xdb_row_getVal (pRow, &filters[j].val);
			filters[j].val.val_type = filters[j].val.sup_type;
			pFilters[j] = &filters[j];
		}
		memset (&sigFlt, 0, sizeof(sigFlt));
		memcpy (sigFlt.pFilters, pFilters, sizeof(pFilters[0]) * pRefFkeym->fld_count);
		sigFlt.filter_count = pRefFkeym->fld_count;

		xdb_rowset_t row_set;
		xdb_rowset_init (&row_set);
		row_set.limit	= 1;
		xdb_sql_query2 (pConn, pRefFkeym->pTblm, &row_set, &sigFlt);
		XDB_EXPECT (0 == row_set.count, XDB_E_CONSTRAINT, "Foreign Key still referenced by '%s'", XDB_OBJ_NAME(pRefFkeym->pTblm));
	}

	return XDB_OK;

error:
	return -1;
}
