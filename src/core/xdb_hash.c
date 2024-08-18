/******************************************************************************
* Copyright (c) 2024-present JC Wang. All rights reserved
*
*   https://crossdb.org
*   https://github.com/crossdb-org/crossdb
*
* This program is free software: you can redistribute it and/or modify it under
* the terms of the GNU General Public License, version 3 or later, as published 
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with 
* this program. If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

XDB_STATIC int 
xdb_hash_rehash (xdb_idxm_t *pIdxm, xdb_rowid old_max)
{
	xdb_hashHdr_t 		*pHashHdr = pIdxm->pHashHdr;
	xdb_rowid 			*pHashSlot = pIdxm->pHashSlot;
	xdb_hashNode_t		*pHashNode = pIdxm->pHashNode;
	xdb_rowid			rid, max_rid = pIdxm->node_cap, next_rid;
	xdb_rowid			new_slot, slot, slot_id, fisrt_rid;
	uint32_t			hash_mask = pIdxm->slot_mask;
	xdb_hashNode_t 		*pCurNode, *pNxtNode, *pPreNode;

	//xdb_dbglog ("Rehash %s old_max %d new_max %d node %d rid %d Begin\n", XDB_OBJ_NAME(pIdxm), old_max, new_max, pIdxm->node_cap, pHashHdr->row_count);

	for (slot = 0; slot < old_max; ++slot) {
		rid = pHashSlot[slot];
		
		for (; XDB_ROWID_VALID (rid, max_rid); rid = next_rid) {
			pCurNode = &pHashNode[rid];
			next_rid = pCurNode->next;
			new_slot = pCurNode->hash_val & hash_mask;
			if (new_slot != slot) {
			//xdb_dbglog ("  -- Move rid %d hashval 0x%x from %d to %d\n", rid,  pCurNode->hash_val, slot, new_slot);
				// remove from old slot
				if (pCurNode->next) {
					//xdb_dbglog ("  rid %d has next %d, point to its prev %d\n", rid, pCurNode->next, pCurNode->prev & (~XDB_ROWID_MSB));
					pNxtNode = &pHashNode[pCurNode->next];
					pNxtNode->prev = pCurNode->prev;
				}
				if (pCurNode->prev & XDB_ROWID_MSB) {
					// is the first top rid
					slot_id = pCurNode->prev & XDB_ROWID_MASK;
					pHashSlot[slot_id] = pCurNode->next;
					if (0 == pCurNode->next) {
						pHashHdr->slot_count--;
					}
					//xdb_dbglog ("  rid %d's next %d is 1st top rid for slot %d\n", rid, pCurNode->next, slot_id);
				} else if (pCurNode->prev) {
					//xdb_dbglog ("  rid %d has prev %d, point to it's next %d\n", rid, pCurNode->prev, pCurNode->next);
					pPreNode = &pHashNode[pCurNode->prev];
					pPreNode->next = pCurNode->next;
		 		}

				// move to new_slot
				fisrt_rid = pHashSlot[new_slot];
				if (0 == fisrt_rid) {
					// first rid
					//xdb_dbglog ("  insert first %d in slot %d \n", rid, new_slot);
					pHashSlot[new_slot] = rid;
					pCurNode->next = 0;
					pCurNode->prev = XDB_ROWID_MSB | new_slot;
					pHashHdr->slot_count++;
				} else {
					// insert head to current slot
					//xdb_dbglog ("  Insert %d in slot %d at head %d\n", rid, new_slot, fisrt_rid);
					pHashSlot[new_slot] = rid;
					pCurNode->next = fisrt_rid;
					pCurNode->prev = XDB_ROWID_MSB | new_slot;
					pNxtNode = &pHashNode[fisrt_rid];
					pNxtNode->prev = rid;
				}
			}
		}
	}

	//xdb_dbglog ("Rehash %s old_max %d new_max %d node %d rid %d End\n", XDB_OBJ_NAME(pIdxm), old_max, new_max, pIdxm->node_cap, pHashHdr->row_count);

	return 0;
}

XDB_STATIC int 
xdb_hash_add (xdb_conn_t *pConn, xdb_idxm_t* pIdxm, xdb_rowid new_rid, void *pRow)
{
	uint32_t hash_val = xdb_row_hash (pRow, pIdxm->pFields, pIdxm->fld_count);
	uint32_t slot_id = hash_val & pIdxm->slot_mask;

	xdb_stgmgr_t	*pStgMgr	= &pIdxm->pTblm->stg_mgr;

	if (new_rid > pIdxm->node_cap) {
		xdb_stg_truncate (&pIdxm->stg_mgr, pIdxm->node_cap<<1);
		pIdxm->node_cap = XDB_STG_CAP(&pIdxm->stg_mgr);
		pIdxm->pHashHdr  = (xdb_hashHdr_t*)pIdxm->stg_mgr.pStgHdr;
		pIdxm->pHashNode = pIdxm->stg_mgr.pBlkDat1;
	}

	if ((pIdxm->slot_cap>>1) < pIdxm->pHashHdr->slot_count) {
		xdb_rowid old_cap = pIdxm->slot_cap;
		//xdb_dbglog ("rehash slot_cap %d slot_count %d\n", pIdxm->slot_cap, pIdxm->pHashHdr->slot_count);
		xdb_stg_truncate (&pIdxm->stg_mgr2, pIdxm->slot_cap<<1);
		pIdxm->slot_cap		= XDB_STG_CAP(&pIdxm->stg_mgr2);
		pIdxm->slot_mask	= pIdxm->slot_cap - 1;
		pIdxm->pHashSlot	= pIdxm->stg_mgr2.pBlkDat;
		xdb_hash_rehash (pIdxm, old_cap);
#if 0
	if (xdb_hash_check (pConn, pIdxm, 0) < 0) { exit(-1); }
#endif
	}

	xdb_dbglog ("add rid %d hash %x slot %d\n", new_rid, hash_val, slot_id);

	xdb_hashHdr_t	*pHashHdr  = pIdxm->pHashHdr;	
	xdb_rowid		*pHashSlot = pIdxm->pHashSlot;	
	xdb_hashNode_t	*pHashNode = pIdxm->pHashNode;
	xdb_rowid		rid;

	xdb_hashNode_t	*pNewNode = &pHashNode[new_rid];
	pNewNode->hash_val = hash_val;

	xdb_rowid first_rid = pHashSlot[slot_id];

	if (0 == first_rid) {
		//xdb_dbglog ("  insert first %d in slot %d \n", new_rid, slot_id);
		pHashSlot[slot_id] = new_rid;
		pNewNode->next = pNewNode->sibling = 0;
		pNewNode->prev = XDB_ROWID_MSB | slot_id;
		pHashHdr->slot_count++;
		pHashHdr->node_count++;
	} else {
		xdb_hashNode_t	*pTopNode = NULL, *pCurNode;
		for (rid = first_rid; rid > 0; rid = pTopNode->next) {
			pTopNode = &pHashNode[rid];
			if (pTopNode->hash_val != hash_val) {
				continue;
			}
			void *pRowDb = XDB_IDPTR(pStgMgr, rid);
			bool eq = xdb_row_isequal2 (pRow, pRowDb, pIdxm->pFields, pIdxm->fld_count);
			if (eq) {
				if (pIdxm->bUnique && xdb_row_valid (pConn, pIdxm->pTblm, pRowDb, rid)) {
					goto error;
				}
				break;
			}
		}
		if (rid) {
			if (pIdxm->bUnique) {
				for (rid = pTopNode->sibling; rid > 0; rid = pCurNode->next) {
					pCurNode = &pHashNode[rid];
					void *pRowDb = XDB_IDPTR(pStgMgr, rid);
					if (xdb_likely (xdb_row_valid (pConn, pIdxm->pTblm, pRowDb, rid))) {
						goto error;
					}
				}
			}

			//xdb_dbglog ("  Duplicate insert %d in slot %d to rid %d as 1st sibling\n", new_rid, slot_id, rid);
			xdb_rowid sibling_rid = pTopNode->sibling;
			pTopNode->sibling = new_rid;
			pNewNode->prev  = 0;
			pNewNode->next = sibling_rid;
			pNewNode->sibling = rid;
			if (sibling_rid) {
				xdb_dbglog ("  1st sibling %d replace previous 1st silbing %d\n", new_rid, sibling_rid);
				xdb_hashNode_t *pSiblingNode = &pHashNode[sibling_rid];
				pSiblingNode->prev = new_rid;
				pSiblingNode->sibling = XDB_ROWID_MSB;
			}
		} else {
			//xdb_dbglog ("  Insert %d in slot %d at head %d\n", new_rid, slot_id, first_rid);
			pHashSlot[slot_id] = new_rid;
			pNewNode->next = first_rid;
			pNewNode->prev = XDB_ROWID_MSB | slot_id;
			pNewNode->sibling = 0;
			xdb_hashNode_t *pNxtNode = &pHashNode[first_rid];
			pNxtNode->prev = new_rid;
			pHashHdr->node_count++;
		}
	}

	pHashHdr->row_count++;

#if 0
	if (xdb_hash_check (pConn, pIdxm, 0) < 0) { exit(-1); }
#endif

	return 0;

error:
	XDB_SETERR(XDB_E_EXISTS, "Duplicate insert for index '%s'", XDB_OBJ_NAME(pIdxm));
	return -XDB_E_EXISTS;
}

XDB_STATIC int xdb_hash_rem (xdb_idxm_t* pIdxm, xdb_rowid rid, void *pRow)
{
	xdb_hashNode_t *pTopNode, *pCurNode, *pNxtNode, *pPreNode, *pSiblingNode, *pNxtSiblingNode;
	uint32_t slot_id;

	xdb_hashHdr_t	*pHashHdr = pIdxm->pHashHdr;
	xdb_rowid		*pHashSlot = pIdxm->pHashSlot;
	xdb_hashNode_t *pHashNode = pIdxm->pHashNode;

	pCurNode = &pHashNode[rid];

	xdb_dbglog ("del rid %d hash %x slot %d\n", rid, pCurNode->hash_val, slot_id);

	if (xdb_unlikely (! (pCurNode->next + pCurNode->prev + pCurNode->sibling))) {
		// not in index
		return 0;
	}

	if (xdb_likely (! (pCurNode->sibling & XDB_ROWID_MASK))) {
		// normal node, either not 1st sibling or no sibling top
		if (pCurNode->next) {
			xdb_dbglog ("  rid %d has next %d, point to its prev %d\n", rid, pCurNode->next, pCurNode->prev & XDB_ROWID_MASK);
			pNxtNode = &pHashNode[pCurNode->next];
			pNxtNode->prev = pCurNode->prev;
		}
		if (pCurNode->prev & XDB_ROWID_MSB) {
			// is the first top record
			slot_id = pCurNode->prev & XDB_ROWID_MASK;
			pHashSlot[slot_id] = pCurNode->next;
			if (0 == pCurNode->next) {
				pHashHdr->slot_count--;
			}
			xdb_dbglog ("  rid %d's next %d is 1st top record for slot %d\n", rid, pCurNode->next, slot_id);
		} else if (pCurNode->prev) {
			xdb_dbglog ("  rid %d has prev %d, point to it's next %d\n", rid, pCurNode->prev, pCurNode->next);
			pPreNode = &pHashNode[pCurNode->prev];
			pPreNode->next = pCurNode->next;
 		}
		if (! (pCurNode->sibling & XDB_ROWID_MSB)) {
			pHashHdr->node_count--;
		}
	} else {
		if (0 == pCurNode->prev) {
			// It's the 1st sibling
			pTopNode = &pHashNode[pCurNode->sibling];
			pTopNode->sibling = pCurNode->next;
			if (pCurNode->next) {
				xdb_dbglog ("  1st sibling %d has next sibling %d, prompt to 1st siblinig\n", rid, pCurNode->next);
				pNxtSiblingNode = &pHashNode[pCurNode->next];
				pNxtSiblingNode->prev = 0;
				pNxtSiblingNode->sibling = pCurNode->sibling;
			}
		} else {
			// It's the top node which has sibling
			xdb_dbglog ("  Top rid %d has sibling %d, prompt to top\n", rid, pCurNode->sibling);
			pSiblingNode = &pHashNode[pCurNode->sibling];
			// if has next silbing, prompt to first sibling
			if (pSiblingNode->next) {
				xdb_dbglog ("  1st Sibling %d has next sibling %d, prompt to 1st sibling\n", pCurNode->sibling, pSiblingNode->next);
				pNxtSiblingNode = &pHashNode[pSiblingNode->next];
				pNxtSiblingNode->sibling = pNxtSiblingNode->prev;
				pNxtSiblingNode->prev = 0;
			}
			pSiblingNode->sibling = pSiblingNode->next;
			pSiblingNode->next = pCurNode->next;
			if (pCurNode->next) {
				xdb_dbglog ("  rid %d has next %d, point to it's sibling %d\n", rid, pCurNode->next, pCurNode->sibling);
				pNxtNode = &pHashNode[pCurNode->next];
				pNxtNode->prev = pCurNode->sibling;
			}
			pSiblingNode->prev = pCurNode->prev;
			if (pCurNode->prev & XDB_ROWID_MSB) {
				slot_id = pCurNode->prev & XDB_ROWID_MASK;
				pHashSlot[slot_id] = pCurNode->sibling;
				xdb_dbglog ("  rid %d's 1st sibing %d prompt to 1st top for slot %d\n", rid, pCurNode->sibling, slot_id);
			} else if (pCurNode->prev) {
				xdb_dbglog ("  rid %d has prev %d, point to it's sibling %d\n", rid, pCurNode->prev, pCurNode->sibling);
				pPreNode = &pHashNode[pCurNode->prev];
				pPreNode->next = pCurNode->sibling;
			}
		}
	}

	// reset curnode
	pCurNode->next		= 0;
	pCurNode->prev		= 0;
	pCurNode->sibling	= 0;

	pHashHdr->row_count--;

#if 0
	if (xdb_hash_check (NULL, pIdxm, 0) < 0) { exit(-1); }
#endif

	return 0;
}

XDB_STATIC xdb_rowid 
xdb_hash_query (xdb_conn_t *pConn, xdb_idxm_t* pIdxm, xdb_value_t **ppValue, xdb_filter_t **ppFilter, int count, xdb_rowset_t *pRowSet)
{
	uint32_t hash_val = xdb_val_hash (ppValue, pIdxm->fld_count);
	uint32_t slot_id = hash_val & pIdxm->slot_mask;

	xdb_hashHdr_t	*pHashHdr  = pIdxm->pHashHdr;	
	xdb_rowid		*pHashSlot = pIdxm->pHashSlot;	
	xdb_hashNode_t	*pHashNode = pIdxm->pHashNode;
	xdb_stgmgr_t	*pStgMgr	= &pIdxm->pTblm->stg_mgr;

	//affect multi-thead performance
#if !defined (XDB_HPO)
	pHashHdr->query_times++;
#endif

	xdb_rowid rid = pHashSlot[slot_id];

	//xdb_dbglog ("hash_get_slot hash 0x%x mod 0x%x slot %d 1st row %d\n", hash_val, pIdxm->slot_mask, slot_id, rid);

	xdb_hashNode_t	*pCurNode;
	for (; rid > 0; rid = pCurNode->next) {
		pCurNode = &pHashNode[rid];
		xdb_prefetch (pCurNode);
		void *pRow = XDB_IDPTR(pStgMgr, rid);
		xdb_prefetch (pRow);
		if (xdb_unlikely (pCurNode->hash_val != hash_val)) {
			continue;
		}
		if (xdb_unlikely (! xdb_row_isequal (pRow, pIdxm->pFields, ppValue, pIdxm->fld_count))) {
			continue;
		}
		if (xdb_likely (xdb_row_valid (pConn, pIdxm->pTblm, pRow, rid))) {
			// Compare rest fields
	 		if ((0 == count) || xdb_row_and_match (pRow, ppFilter, count)) {
				if (xdb_unlikely (-XDB_E_FULL == xdb_rowset_add (pRowSet, rid, pRow))) {
					return XDB_OK;
				}
			}
		}

		for (rid = pCurNode->sibling; rid > 0; rid = pCurNode->next) {
			pCurNode = &pHashNode[rid];
			pRow = XDB_IDPTR(pStgMgr, rid);
			if (xdb_likely (xdb_row_valid (pConn, pIdxm->pTblm, pRow, rid))) {
				// Compare rest fields
				if ((0 == count) || xdb_row_and_match (pRow, ppFilter, count)) {
					if (xdb_unlikely (-XDB_E_FULL == xdb_rowset_add (pRowSet, rid, pRow))) {
						return XDB_OK;
					}
				}
			}
		}

		return XDB_OK;
	}

	return XDB_OK;
}

XDB_STATIC int 
xdb_hash_close (xdb_idxm_t *pIdxm)
{
	char path[XDB_PATH_LEN + 32];
	xdb_tblm_t *pTblm = pIdxm->pTblm;

	xdb_sprintf (path, "%s/T%06d/I%02d.idx", pTblm->pDbm->db_path, XDB_OBJ_ID(pTblm), XDB_OBJ_ID(pIdxm));
	xdb_stg_close (&pIdxm->stg_mgr);

	xdb_sprintf (path, "%s/T%06d/I%02d.hash", pTblm->pDbm->db_path, XDB_OBJ_ID(pTblm), XDB_OBJ_ID(pIdxm));
	xdb_stg_close (&pIdxm->stg_mgr2); 

	return XDB_OK;
}

XDB_STATIC int 
xdb_hash_drop (xdb_idxm_t *pIdxm)
{
	char path[XDB_PATH_LEN + 32];
	xdb_tblm_t *pTblm = pIdxm->pTblm;

	xdb_sprintf (path, "%s/T%06d/I%02d.idx", pTblm->pDbm->db_path, XDB_OBJ_ID(pTblm), XDB_OBJ_ID(pIdxm));
	xdb_stg_drop (&pIdxm->stg_mgr, path);
	
	xdb_sprintf (path, "%s/T%06d/I%02d.hash", pTblm->pDbm->db_path, XDB_OBJ_ID(pTblm), XDB_OBJ_ID(pIdxm));
	xdb_stg_drop (&pIdxm->stg_mgr2, path); 

	return XDB_OK;
}

XDB_STATIC int 
xdb_hash_create (xdb_idxm_t *pIdxm)
{
	xdb_tblm_t *pTblm = pIdxm->pTblm;
	char path[XDB_PATH_LEN + 32];
	xdb_sprintf (path, "%s/T%06d/I%02d.idx", pTblm->pDbm->db_path, XDB_OBJ_ID(pTblm), XDB_OBJ_ID(pIdxm));

	xdb_stghdr_t stg_hdr = {.stg_magic = 0xE7FCFDFB, .blk_flags=1, .blk_size = sizeof(xdb_hashNode_t), 
							.ctl_off = 0, .blk_off = XDB_OFFSET(xdb_hashHdr_t, hash_node)};
	pIdxm->stg_mgr.pOps = pTblm->stg_mgr.pOps;
	pIdxm->stg_mgr.pStgHdr	= &stg_hdr;
	int rc = xdb_stg_open (&pIdxm->stg_mgr, path, NULL, NULL);
	if (rc != XDB_OK) {
		xdb_errlog ("Failed to create index %s", XDB_OBJ_NAME(pIdxm));
	}

	xdb_sprintf (path, "%s/T%06d/I%02d.hash", pTblm->pDbm->db_path, XDB_OBJ_ID(pTblm), XDB_OBJ_ID(pIdxm));

	// don't manage, clear when expand
	xdb_stghdr_t stg_hdr2 = {.stg_magic = 0xE7FCFDFB, .blk_flags=0x3, .blk_size = sizeof(xdb_rowid), 
							.ctl_off = 0, .blk_off = XDB_OFFSET(xdb_hashSlot_t, hash_slot)};
	pIdxm->stg_mgr2.pOps = pTblm->stg_mgr.pOps;
	pIdxm->stg_mgr2.pStgHdr	= &stg_hdr2;
	rc = xdb_stg_open (&pIdxm->stg_mgr2, path, NULL, NULL);
	if (rc != XDB_OK) {
		xdb_errlog ("Failed to create index %s", XDB_OBJ_NAME(pIdxm));
	}

	pIdxm->pHashHdr = (xdb_hashHdr_t*)pIdxm->stg_mgr.pStgHdr;
	pIdxm->pHashNode = pIdxm->stg_mgr.pBlkDat1;
	pIdxm->node_cap = XDB_STG_CAP(&pIdxm->stg_mgr);

	pIdxm->slot_cap = XDB_STG_CAP(&pIdxm->stg_mgr2);
	pIdxm->slot_mask = pIdxm->slot_cap - 1;
	pIdxm->pHashSlot  = pIdxm->stg_mgr2.pBlkDat;

	return XDB_OK;
}


static xdb_idx_ops s_xdb_hash_ops = {
	.idx_add 	= xdb_hash_add,
	.idx_rem 	= xdb_hash_rem,
	.idx_query 	= xdb_hash_query,
	.idx_create = xdb_hash_create,
	.idx_drop 	= xdb_hash_drop,
	.idx_close 	= xdb_hash_close
};