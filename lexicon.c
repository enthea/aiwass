/***
*lexicon.c - lexicon routines (for aiwass metaqabalah)
*
*   Copyright (C) 1993, 2022 Skye Love Hill.
*
*   This file is part of Aiwass v. 1.
*
*   Aiwass is free software: you can redistribute it and/or modify it under
*   the terms of the GNU General Public License as published by the Free
*   Software Foundation, either version 3 of the License, or (at your option)
*   any later version.
*
*   Aiwass is distributed in the hope that it will be useful, but WITHOUT ANY
*   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
*   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
*   details.
*
*   You should have received a copy of the GNU General Public License along
*   with Aiwass. If not, see <https://www.gnu.org/licenses/>.
*
*Purpose:
*   This file contains the code for maintaining a lexicon file
*   (for aiwass metaqabalah), such as adding, deleting, and
*   looking up words and their definitions.
*
****************************************************************************/


#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#include "lexicon.h"
#include "scratch.h"
#include "webster.h"

typedef byte
  CODE[MAX_STR];

static IXF* lex;
static char lex_drive[_MAX_DRIVE];
static char lex_dir  [_MAX_DIR];
static char lex_fname[_MAX_FNAME];
static char lex_ext  [_MAX_EXT];

int lex_decode(char**,CODE);
int lex_encode(CODE,char*);


int lex_init(ntok,token)
int   ntok;
char* token[];
{
	_splitpath(token[0],lex_drive,lex_dir,lex_fname,lex_ext);
	if (!lex_fname[0]) strcpy(lex_fname,"aiwass");
	if (!lex_ext[0])   strcpy(lex_ext,  "lex");

	_makepath(scratch,lex_drive,lex_dir,lex_fname,lex_ext);
	lex = opixf(scratch,LEX_BLK,LEX_IDX,LEX_KEY,LEX_PRM);
	if (lex == NULL) return(0);
	return(!atexit(lex_term));
}

int lex_add_word(clang,string,defn,dodef)
byte  clang;
char* string;
char* defn;
int   dodef;
{
	RST* rst;
	HEAD head;
	CODE code;
	int  found;
	int  loc;
	int  ncode;
	int  struck;
	int  unknown;
	byte mark;

	found = ixlocr(lex,&head,&loc,MASK,clang,string);
	if (found < 0) return(-1);

	struck  = found && ((head.tag & IX_DEL) != 0);
	unknown = found && ((head.tag & MASK)   != clang);

	if (dodef) lex_encode(code,defn);

	if (!found || dodef || struck || unknown) {
		mark = (head.tag & IX_DEF);
		rst = ixaddr(lex,&head,loc,clang|mark);
		if (rst == NULL) return(-1);

		if (!found || dodef) {
			ixputb(rst,(byte*)string,0);

			if (dodef) {
				ixputb(rst,(byte*)&code[0],1);
				if (code[0])
					ixputb(rst,(byte*)&code[1],code[0]);
			}
		}
		clixr(rst);
	}
	return(loc);
}

int lex_del_word(clang,string)
byte  clang;
char* string;
{
	RST* rst;
	HEAD head;
	int  loc;
	int  result;

	result = ixlocr(lex,&head,&loc,MASK,clang,string);
	if (result < 0) return(-1);

	if ((result > 0) && !(head.tag & IX_DEF)) {
		rst = ixdelr(lex,&head,loc,clang);
		if (rst == NULL) return(-1);
		clixr(rst);
	}
	return(loc);
}

int lex_decode(defn,code)
char** defn;
CODE   code;
{
	HEAD  head;
	int   i,j;
	int   loc;

	for (i=1, j=0; i <= code[0]; i++) {
		if (code[i] & '\x80') {
			loc  = ((int)(code[i]&'\x7f') << 8);
			loc |= code[++i];
			if (lex_get_word(LOCSET,&loc,&head,&scratch[j]))
				j += strlen(&scratch[j]);
		}
		else
			scratch[j++] = code[i];
	}
	scratch[j] = '\0';
	*defn = scratch;
	return(j);
}

int lex_encode(code,defn)
CODE  code;
char* defn;
{
	RST*  rst;
	HEAD  head;
	char* token;

	int   i;
	int   loc;
	int   result;

	int   ncode = 0;
	byte  tag   = 0;

	strtok(defn,"\n");
	strncpy(scratch,defn,MAX_STR-1);
	for (i=0; isspace(defn[i]); i++)
		;

	while (defn[i]) {
		if (isalpha(defn[i])) {
			token = strtok(&scratch[i]," ,.;()[]{}?!\"\t\n");
			result = ixlocr(lex,&head,&loc,MASK,tag,token);
			if (result == -2) printf(" %s - error\n",token);
			if (result == -1) printf(" %s - full\n",token);
			if (result >=  0)
				if (!(head.tag & IX_DEF)) {
					rst = ixaddr(lex,&head,loc,head.tag|IX_DEF);
					if (rst == NULL) return(-1);
					ixputb(rst,(byte*)token,0);
					clixr(rst);
				}
			code[++ncode] = (byte)((loc >> 8)) | '\x80';
			code[++ncode] = (byte)(loc & 0xFF);
			i += strlen(token);
		}
		else code[++ncode] = defn[i++];
	}
	code[0] = (byte)ncode;
	return(ncode);
}

int lex_get_data(clang,loc,head,string,defn)
byte   clang;
int*   loc;
HEAD*  head;
char*  string;
char** defn;
{
	RST* rst;
	CODE code;
	int  result;

	if (clang != LOCSET) {
		result = ixlocr(lex,head,loc,MASK,clang,string);
		if (result != 1) return(0);
	}

	if (ixempty(lex,*loc)) return(0);
	rst = opixr(lex,*loc,IX_READ);
	if (rst == NULL) return(0);

	ixgetb(rst,(byte*)head,sizeof(HEAD));
	ixgetb(rst,(byte*)string,0);
	ixgetb(rst,(byte*)&code[0],1);
	if (code[0])
		ixgetb(rst,(byte*)&code[1],code[0]);
	lex_decode(defn,code);
	clixr(rst);
	return(1);
}

int lex_get_word(clang,loc,head,string)
byte  clang;
int*  loc;
HEAD* head;
char* string;
{
	RST* rst;
	int  result;

	if (clang != LOCSET) {
		result = ixlocr(lex,head,loc,MASK,clang,string);
		if (result != 1) return(0);
	}

	if (ixempty(lex,*loc)) return(0);
	rst = opixr(lex,*loc,IX_READ);
	if (rst == NULL) return(0);

	ixgetb(rst,(byte*)head,6);
	ixgetb(rst,(byte*)string,0);
	clixr(rst);
	return(1);
}

void lex_term()
{
	clixf(lex);
}
