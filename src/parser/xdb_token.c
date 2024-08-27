/******************************************************************************
* Copyright (c) 2024-present JC Wang. All rights reserved
*
*   https://crossdb.org
*   https://github.com/crossdb-org/crossdb
*
* This program is free software: you can use, redistribute, and/or modify it under
* the terms of the GNU Lesser General Public License, version 3 or later ("LGPL"),
* as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
* FOR A PARTICULAR PURPOSE. See the GNU published General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with 
* this program. If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#define XDB_TOK_INIT(sql)	{.tk_type=XDB_TOK_INV, .tk_sql=sql}

typedef struct {
	xdb_token_type	tk_type;
	char 			*tk_sql;
	char			tk_nxt;
	//xdb_str_t 	token;
	int				tk_len;
	bool			bFloat;
	char			*token;
} xdb_token_t;

static char s_tok_type[256] = {
	['\0'] = XDB_TOK_EOF,
	[';'] = XDB_TOK_END,
	['('] = XDB_TOK_LP,
	[')'] = XDB_TOK_RP,
	['{'] = XDB_TOK_LB,
	['}'] = XDB_TOK_RB,
	['['] = XDB_TOK_LBK,
	[']'] = XDB_TOK_RBK,
	[','] = XDB_TOK_COMMA,
	['.'] = XDB_TOK_DOT,
	['\\'] = XDB_TOK_ESC,
	['\''] = XDB_TOK_STR,
	['"'] = XDB_TOK_STR,
	['`'] = XDB_TOK_STR,
	[' '] = XDB_TOK_SP,
	['\t'] = XDB_TOK_SP,
	['\r'] = XDB_TOK_SP,
	['\n'] = XDB_TOK_SP,
	['='] = XDB_TOK_EQ,
	['<'] = XDB_TOK_LT,
	['>'] = XDB_TOK_GT,
	['!'] = XDB_TOK_NEG,
	['+'] = XDB_TOK_ADD,
	['-'] = XDB_TOK_SUB,
	['*'] = XDB_TOK_MUL,
	['/'] = XDB_TOK_DIV,
	['?'] = XDB_TOK_QM,
	['0'] = XDB_TOK_NUM,
	['1'] = XDB_TOK_NUM,
	['2'] = XDB_TOK_NUM,
	['3'] = XDB_TOK_NUM,
	['4'] = XDB_TOK_NUM,
	['5'] = XDB_TOK_NUM,
	['6'] = XDB_TOK_NUM,
	['7'] = XDB_TOK_NUM,
	['8'] = XDB_TOK_NUM,
	['9'] = XDB_TOK_NUM,
};

XDB_STATIC xdb_token_type 
xdb_next_token (xdb_token_t *pTkn)
{
	char *str, ch;
	// last is id, check if next is not space
	if (XDB_TOK_ID == pTkn->tk_type || XDB_TOK_NUM == pTkn->tk_type) {
		ch = s_tok_type[(uint8_t)pTkn->tk_nxt];
		if (XDB_TOK_SP != ch) {
			switch (ch) {
			case XDB_TOK_GT:
			case XDB_TOK_LT:
			case XDB_TOK_NEG:
				pTkn->tk_sql--;
				goto parse_ch;
			default:
				break;
			}
			pTkn->tk_type = ch;
			return pTkn->tk_type;
		}
	}
    // skip white space
    while (XDB_TOK_SP == s_tok_type[(uint8_t)*pTkn->tk_sql]) {
        pTkn->tk_sql++;
	}

	ch = s_tok_type[(uint8_t)*pTkn->tk_sql];

parse_ch:
	switch (ch) {
	case XDB_TOK_ID:
		pTkn->tk_type = XDB_TOK_ID;
		pTkn->token = pTkn->tk_sql++;
		ch = s_tok_type[(uint8_t)*pTkn->tk_sql];
		while ((XDB_TOK_ID == ch) || (XDB_TOK_NUM == ch)) {
			pTkn->tk_sql++;
			ch = s_tok_type[(uint8_t)*pTkn->tk_sql];
		}
		pTkn->tk_len = pTkn->tk_sql - pTkn->token;
		pTkn->tk_nxt = *pTkn->tk_sql;
		*pTkn->tk_sql++ = '\0';
		return XDB_TOK_ID;
	case XDB_TOK_SUB:
		if (XDB_TOK_NUM != s_tok_type[(uint8_t)*(pTkn->tk_sql+1)]) {
			break;
		}
		break;
	case XDB_TOK_NUM:
		pTkn->tk_type = XDB_TOK_NUM;
		pTkn->token = pTkn->tk_sql;
		pTkn->bFloat = false;
		if (XDB_TOK_SUB == s_tok_type[(uint8_t)*pTkn->tk_sql]) {
			pTkn->tk_sql++;
		}
		while (XDB_TOK_NUM == s_tok_type[(uint8_t)*pTkn->tk_sql]) {
			pTkn->tk_sql++;
		}
		if (XDB_TOK_DOT == s_tok_type[(uint8_t)*pTkn->tk_sql]) {
			pTkn->bFloat = true;
			pTkn->tk_sql++;
			while (XDB_TOK_NUM == s_tok_type[(uint8_t)*pTkn->tk_sql]) {
				pTkn->tk_sql++;
			}
		}
		pTkn->tk_len = pTkn->tk_sql - pTkn->token;
		pTkn->tk_nxt = *pTkn->tk_sql;
		*pTkn->tk_sql++ = '\0';
		return XDB_TOK_NUM;
	case XDB_TOK_STR:
		pTkn->tk_type = XDB_TOK_STR;
		pTkn->token = pTkn->tk_sql++;
		str = pTkn->tk_sql;
		while (*pTkn->tk_sql && *pTkn->token != *pTkn->tk_sql) {
			if (xdb_unlikely ('\\' == *pTkn->tk_sql)) {
				switch (pTkn->tk_sql[1]) {
				case 'n': *str = '\n'; break;
				case 'r': *str = '\r'; break;
				case 't': *str = '\t'; break;
				case '\'':
				case '"':
				case '`':
				case '\\':
				case '%':
				case '_':
					*str = pTkn->tk_sql[1];
					break;
				default: 	return XDB_TOK_INV;
				}
				str++;
				pTkn->tk_sql += 2;				
			} else {
				*str++ = *pTkn->tk_sql++;
			}
		}
		if ('\0' == *pTkn->tk_sql) {
			return XDB_TOK_INV;
		}
		pTkn->token++;
		pTkn->tk_len = str - pTkn->token;
		*str = '\0';
		pTkn->tk_sql++;
		return XDB_TOK_STR;
	case XDB_TOK_GT:
		ch = s_tok_type[(uint8_t)*(pTkn->tk_sql+1)];
		if (XDB_TOK_EQ == ch) {
			pTkn->tk_sql += 2;
			pTkn->tk_type = XDB_TOK_GE;
			return XDB_TOK_GE;
		}
		pTkn->tk_sql++;
		pTkn->tk_type = XDB_TOK_GT;
		return XDB_TOK_GT;
	case XDB_TOK_LT:
		ch = s_tok_type[(uint8_t)*(pTkn->tk_sql+1)];
		if (XDB_TOK_EQ == ch) {
			pTkn->tk_sql += 2;
			pTkn->tk_type = XDB_TOK_LE;
			return XDB_TOK_LE;
		} else if (XDB_TOK_GT == ch) {
			pTkn->tk_sql += 2;
			pTkn->tk_type = XDB_TOK_NE;
			return XDB_TOK_NE;
		}
		pTkn->tk_sql++;
		pTkn->tk_type = XDB_TOK_LT;
		return XDB_TOK_LT;
		break;
	case XDB_TOK_NEG:
		ch = s_tok_type[(uint8_t)*(pTkn->tk_sql+1)];
		if (XDB_TOK_EQ == ch) {
			pTkn->tk_sql += 2;
			pTkn->tk_type = XDB_TOK_NE;
			return XDB_TOK_NE;
		}
		break;
	default:
		break;
	}

	pTkn->token = pTkn->tk_sql;
	pTkn->tk_type = s_tok_type[(uint8_t)*pTkn->tk_sql++];
	return pTkn->tk_type;
}
