/***
*numicon.c - numeric index routines (for aiwass metaqabalah)
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
*   This file contains the code for maintaining the numeric
*   index(es) to the lexicon (for aiwass metaqabalah), such
*   as adding, deleting, and listing words by qabalistic
*   value.
*
****************************************************************************/


#include <ctype.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#include "lexicon.h"
#include "numicon.h"
#include "qabalah.h"
#include "scratch.h"
#include "scriber.h"
#include "webster.h"

typedef char
	USENAM[_MAX_EXT];

typedef struct {
	int lang;
	int key;
} HANDLE;

static USENAM uselst[MAX_FILE];
static HANDLE handle[MAX_FILE];
static IXF*   ixf   [MAX_FILE];
static char   num_drive[_MAX_DRIVE];
static char   num_dir  [_MAX_DIR];
static char   num_fname[_MAX_FNAME];
static char   num_ext  [_MAX_EXT];
static int    nfile;
static int    nuse;


int num_init(ntok,token)
int   ntok;
char* token[];
{
	_splitpath(token[0],num_drive,num_dir,num_fname,num_ext);
	return(!atexit(num_term));
}

int num_add_word(clang,loc,string)
int   clang;
int   loc;
char* string;
{
	int   cfile;
	int   ckey;
	int   item;
	int   num;
	RST*  rst;

	/* loc = loc + 1; */
	for (cfile = 0; cfile < nfile; cfile++) {
		if (handle[cfile].lang == clang) {
			ckey = handle[cfile].key;

			rst = opixr(ixf[cfile],qbl_eval(ckey,string),IX_WRITE);

			if (rst != NULL) {

				do {
					ixgetb(rst,(byte*)&item,2);
				} while (item && (item != loc));

				if (!item) ixmodb(rst,(byte*)&loc,2);

				clixr(rst);
			}
			printf(" %s added to %s\n",string,ixf[cfile]->path);
		}
	}
	return(1);
}

int num_del_word(clang,loc,string)
int   clang;
int   loc;
char* string;
{
	RST*  rst1;
	RST*  rst2;
	int   cfile;
	int   ckey;
	int   item;
	int   value;

	/* loc = loc + 1; */
	for (cfile = 0; cfile < nfile; cfile++) {
		if (handle[cfile].lang == clang) {
			ckey = handle[cfile].key;

			value = qbl_eval(ckey,string);
			rst1 = opixr(ixf[cfile],value,IX_READ);
			rst2 = opixr(ixf[cfile],value,IX_WRITE);

			if ((rst1 != NULL) && (rst2 != NULL)) {

				do {
					ixgetb(rst1,(byte*)&item,2);
					ixgetb(rst2,(byte*)&item,2);
				} while (item && (item != loc));

				if (item) {
					ixgetb(rst1,(byte*)&item,2);
					ixmodb(rst2,(byte*)&item,2);
				}

				while (item) {
					ixgetb(rst1,(byte*)&item,2);
					ixputb(rst2,(byte*)&item,2);
				}

				clixr(rst2);
				clixr(rst1);
			}
			printf(" %s struck from %s\n",string,ixf[cfile]->path);
		}
	}
	return(1);
}

int num_evaluate(string)
char* string;
{
	int cfile;
	int value;

	char* tagg = "   ";

	set_margin(9);
	for (cfile = 0; cfile < nfile; cfile++) {
		tagg[1] = '@' + handle[cfile].lang;
		tagg[2] = '@' + handle[cfile].key;
		value = qbl_eval(handle[cfile].key,string);
		scribe_word(tagg," ");
		scribe_num(value,4," ");
		scribe_word(string,"");
		scribe_eol();
	}
}

int num_explicate(string,define_on,sticky_on,range_flag)
char* string;
int   define_on;
int   sticky_on;
int   range_flag;
{
	RST*  rst;
	HEAD  head;
	char* defn;

	int   cfile;
	int   flag;
	int   item;
	int   loc;
	int   value;

	char*  tagg = "   ";

	flag = 0;
	set_margin(9);

	for (cfile = 0; cfile < nfile; cfile++) {
		tagg[1] = '@' + handle[cfile].lang;
		tagg[2] = '@' + handle[cfile].key;

		if ((!sticky_on) || (cfile == 0)) {
			value = atoi(string);
			if (!value)
				value = qbl_eval(handle[cfile].key,string);
		}

		rst = opixr(ixf[cfile],value,IX_READ);
		if (rst != NULL) {

			ixgetb(rst,(byte*)&item,2);

			if (!item) {
				if (!range_flag || (nfile > 1)) {
					scribe_word(tagg," ");
					scribe_num(value,4," ");
					scribe_word("no entries","");
					scribe_eol();
				}
			}
			else {

				scribe_word(tagg," ");
				scribe_num(value,4," ");

				flag = 0;
				do {

					if (flag) {
						scribe_eol();
						scribe_mar();
					}

					defn = "";
					loc = item /* - 1 */;
					if (define_on)
						lex_get_data(LOCSET,&loc,&head,scratch,&defn);
					else
						lex_get_word(LOCSET,&loc,&head,scratch);

					if (!define_on || !defn[0])
						scribe_word(scratch,", ");
					else {
						scribe_word(scratch," - ");
						scribe_word(defn,"");
						flag = 1;
					}

					ixgetb(rst,(byte*)&item,2);
				} while (item);
				scribe_eol();
			}
			clixr(rst);
		}
	}
	if (flag || (nfile > 1))
		scribe_eol();
	return(1);
}

int num_index()
{
	HEAD  head;
	int   cfile;
	int   clang;
	int   ckey;
	int   loc;


	for (cfile = 0; cfile < nfile; cfile++) {
		clang = handle[cfile].lang;
		ckey  = handle[cfile].key;

		printf(" recreating %s index\n",ixf[cfile]->path);
		for (loc = 0; loc < LEX_IDX; loc++) {
			if (lex_get_word(LOCSET,&loc,&head,scratch)) {
				if ((head.tag & MASK) == clang)
					num_add_word(clang,loc,scratch);
			}
		}
		flixf(ixf[cfile]);
	}
}

void num_term()
{
	int cfile;

	for (cfile = 0; cfile < nfile; cfile++)
		clixf(ixf[cfile]);
	nfile = 0;
}


int use_check(ntok,token)
int   ntok;
char* token[];
{
	int   clang;
	int   ckey;
	int   ctok;
	char* ext;
	char* name;
	int   star0;
	int   star1;
	char  t[3];

	int error = 0;


	for (ctok=1; ctok < ntok; ctok++) {

		if (strlen(token[ctok]) > 2) {
			error = 1;
			break;
		}

		strcpy(t,token[ctok]);
		if (!t[1]) t[1] = t[0];
		star0 = (t[0] == '*');
		star1 = (t[1] == '*');

		if (( star0 &&  star1)         ||
			 (!star0 && !isalpha(t[0])) ||
			 (!star1 && !isalpha(t[1]))) {
				error = 1;
				break;
		}

		if (!star0) {
			clang = toupper(t[0]) - '@';
			qbl_name(clang,&name,&ext);
			if (!name[0]) {
				error = 2;
				break;
			}
		}

		if (!star1) {
			ckey  = toupper(t[1]) - '@';
			qbl_name(ckey,&name,&ext);
			if (!ext[0]) {
				error = 3;
				break;
			}
			if (!qbl_loaded(ckey)) {
				error = 4;
				break;
			}
		}
	}

	if (error) {
		printf(" error: %s ",token[ctok]);
		switch (error) {
			case 1: printf("ill-formed\n");       break;
			case 2: printf("lang not defined\n"); break;
			case 3: printf("key not defined\n");  break;
			case 4: printf("key not loaded\n");   break;
		}
	}
	return(!error);
}

int use_open(ntok,token,setlist)
int   ntok;
char* token[];
int   setlist;
{
	int   cfile;
	int   clang;
	int   ckey;
	int   ctok;
	char* ext;
	char* junk;
	char* name;
	char  t[3];


	if (!use_check(ntok,token)) return(0);
	if (setlist) use_set(ntok,token);

	for (cfile=0; cfile<nfile; cfile++)
		clixf(ixf[cfile]);
	nfile = 0;

	for (ctok = 1; ctok < ntok; ctok++) {

		strcpy(t,token[ctok]);
		if (!t[1]) t[1] = t[0];
		clang = toupper(t[0]) - '@';
		ckey  = toupper(t[1]) - '@';

		if      (t[0] == '*') {
		}

		else if (t[1] == '*') {
		}

		else {
			qbl_name(clang,&name,&junk);
			qbl_name(ckey, &junk,&ext);
			_makepath(scratch,num_drive,num_dir,name,ext);
			ixf[nfile] = opixf(scratch,NUM_BLK,NUM_IDX,NUM_KEY,NUM_PRM);

			if (ixf[nfile] != NULL) {
				handle[nfile].lang = clang;
				handle[nfile].key  = ckey;
				nfile++;
			}
			else printf(" %s: file not opened\n",scratch);
		}
	}
}

int use_set(ntok,token)
int   ntok;
char* token[];
{
	int ctok;

	for (ctok = 1; ctok < ntok; ctok++)
		strcpy(uselst[ctok-1],token[ctok]);
	nuse = ntok - 1;
}

int use_show()
{
	int cuse;

	printf(" use list:");
	for (cuse = 0; cuse < nuse; cuse++)
		printf(" %s",uselst[cuse]);
	printf("\n");
}
