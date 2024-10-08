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

#ifndef __XDB_BMP_H__
#define __XDB_BMP_H__

typedef struct {
	uint64_t 		bmp4; 		// 64bit         	(bit>>6)&0x3f
	uint64_t 		bmp5[64];  // 64*64bit    	bit&0x3f
	uint16_t		count;
	bool			bMalloc;
} xdb_lv2bmp_t; // 4K

typedef struct {
	uint64_t 		bmp2; // 64bit				(bit>>18)&7
	uint64_t 		bmp3[64]; // 64*54bit		(bit>>12)&7
	xdb_lv2bmp_t 	*pLv2Bmp[64][64];
	bool			bMalloc;
} xdb_lv1bmp_t; // 4K 

typedef struct {
	uint64_t 		bmp1[2]; // 2*8bit			(bit>>30) (bit>>24)&0x3f
	xdb_lv1bmp_t 	*pLv1Bmp[2][64];

	xdb_lv1bmp_t 	lv1_bmp[1]; // [0][0]
	xdb_lv2bmp_t	lv2_bmp[128]; // 128*4K 512K rows
	uint16_t		lv1_alloc;
	uint16_t		lv2_alloc;
} xdb_bmp_t; // 128

typedef int (*xdb_bmp_cb) (uint32_t bit, void *pArg);

XDB_STATIC void 
xdb_bmp_init (xdb_bmp_t *pBmp);

XDB_STATIC void 
xdb_bmp_free (xdb_bmp_t *pBmp);

XDB_STATIC void 
xdb_bmp_iterate (xdb_bmp_t *pBmp, xdb_bmp_cb pCb, void *pArg);

XDB_STATIC int 
xdb_bmp_set (xdb_bmp_t *pBmp, uint32_t bit);

XDB_STATIC void 
xdb_bmp_clr (xdb_bmp_t *pBmp, uint32_t bit);

XDB_STATIC bool 
xdb_bmp_get (xdb_bmp_t *pBmp, uint32_t bit);

XDB_STATIC void 
xdb_lv2bmp_iterate (xdb_lv2bmp_t *pLv2Bmp, xdb_bmp_cb pCb, void *pArg);
XDB_STATIC void 
xdb_lv2bmp_init (xdb_lv2bmp_t *pLv2Bmp);
XDB_STATIC void 
xdb_lv2bmp_set (xdb_lv2bmp_t *pLv2Bmp, uint32_t bit);
XDB_STATIC void 
xdb_lv2bmp_clr (xdb_lv2bmp_t *pLv2Bmp, uint32_t bit);
XDB_STATIC bool 
xdb_lv2bmp_get (xdb_lv2bmp_t *pLv2Bmp, uint32_t bit);

#endif // __XDB_BMP_H__
