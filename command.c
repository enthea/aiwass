/***
*command.c - command routines for aiwass metaqabalah
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
*   This file contains the code for executing the commands
*   for aiwass metaqabalah, performing the required command
*   functions, and returning error conditions.
*
****************************************************************************/


#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "types.h"

#include "aiwass.h"
#include "command.h"
#include "lexicon.h"
#include "numicon.h"
#include "qabalah.h"
#include "scratch.h"
#include "webster.h"


int  alpha_on  = 0;
int  define_on = 1;
int  sticky_on = 0;


int add(ntok,token)
int   ntok;
char* token[];
{
	HEAD  head;
	int   clang;
	int   ctok;
	int   def;
	char* defn;
	int   loc;


	if (!qbl_key(&clang,token[1])) return(-1);

	for (ctok = 2; ctok < ntok; ctok++) {

		def = (strcmpi(token[0],"add/def") == 0);
		if (lex_get_data(clang,&loc,&head,token[ctok],&defn)) {
			strncpy(scratch,defn,MAX_STR-1);
			if (def && scratch[0]) {
				printf(" current defn is: %s\n",scratch);
				printf(" change defn [y/n]? ");
				fgets(scratch,128,stdin);
				def = (tolower(scratch[0]) == 'y');
			}
		} else scratch[0] = '\0';

		if (def) {
			printf(" enter definition: ");
			fgets(scratch,128,stdin);
		}

		loc = lex_add_word(clang,token[ctok],scratch,def);
		num_add_word(clang,loc,token[ctok]);
	}
	return(0);
}

int addgloss(ntok,token)
int   ntok;
char* token[];
{
	int   clang;
	int   loc;
	FILE* stream;
	char* word;

	if (!qbl_key(&clang,token[1])) return(-1);

	stream = fopen(token[2],"r");
	while (fgets(scratch,128,stream)) {
		if (ctlc) goto RETURN;
		word = strtok(scratch," \t\n");
		loc = lex_add_word(clang,word,&scratch[strlen(word)+1],1);
		num_add_word(clang,loc,word);
	}

RETURN:
	if (ctlc) ctlc = 0;
	fclose(stream);
	return(0);
}

int alpha(ntok,token)
int   ntok;
char* token[];
{
	return(toggle(&alpha_on,ntok,token));
}

int def(ntok,token)
int   ntok;
char* token[];
{
	return(toggle(&define_on,ntok,token));
}

int del(ntok,token)
int   ntok;
char* token[];
{
	int clang;
	int ctok;
	int loc;

	if (!qbl_key(&clang,token[1])) return(-1);

	for (ctok = 2; ctok < ntok; ctok++) {
		loc = lex_del_word(clang,token[ctok]);
		num_del_word(clang,loc,token[ctok]);
	}
	return(0);
}

int eval(ntok,token)
int   ntok;
char* token[];
{
	int ctok;

	for (ctok = 1; ctok < ntok; ctok++)
		num_evaluate(token[ctok]);
	return(0);
}

int indexx(ntok,token)
int   ntok;
char* token[];
{
	/* this routine needs expansion  */
	/* index lk, must set as uselist */

	num_index();
	return(0);
}

int keys(ntok,token)
int   ntok;
char* token[];
{
	char* name;
	char* ext;
	int   ckey;

	for (ckey=1; ckey<MAX_KEYS; ckey++) {
		if (qbl_name(ckey,&name,&ext)) {
			printf(" %c is %s",'@'+ckey,name);
			printf("\t%s",ext);
			printf("\t%s",qbl_loaded(ckey) ? "KEY" : "");
			printf("\n");
		}
	}
	return(0);
}

int qbl(ntok,token)
int   ntok;
char* token[];
{
	int clang;
	int ctok;
	int value;

	for (ctok = 1; ctok < ntok; ctok++)
		num_explicate(token[ctok],define_on,sticky_on,0);
	return(0);
}

int quit(ntok,token)
int   ntok;
char* token[];
{
	done = 1;
	return(0);
}

int range(ntok,token)
int   ntok;
char* token[];
{
	int from;
	int inc;
	int to;
	int value;

	from = atoi(token[1]);
	to   = atoi(token[2]);
	if (!from || !to) return(-1);

	inc = (from < to) ? 1 : -1;
	for (value=from; value!=to+inc; value += inc) {
		if (ctlc) goto RETURN;
		itoa(value,scratch,10);
		num_explicate(scratch,define_on,1,1);
	}

RETURN:
	if (ctlc) ctlc = 0;
	return(0);
}

int setkey(ntok,token)
int   ntok;
char* token[];
{
	int   ckey;
	char* name;
	char* ext;

	if (!qbl_key(&ckey,token[1])) return(-1);

	name = (ntok >= 3) ? token[2] : "";
	ext  = (ntok >= 4) ? token[3] : "";
	qbl_setname(ckey,name,ext);
	qbl_load(ckey);
	return(0);
}

int show(ntok,token)
int   ntok;
char* token[];
{
	HEAD  head;
	char* defn;
	int   ctok;
	int   defnode;
	int   deleted;
	char* ext;
	int   loc;
	char* name;
	int   result;

	for (ctok = 1; ctok < ntok; ctok++) {

		loc = atoi(token[ctok]);

		if (!loc) {
			strncpy(scratch,token[ctok],MAX_STR-1);
			result = lex_get_data(0,&loc,&head,scratch,&defn);
		}
		else
			result = lex_get_data(LOCSET,&loc,&head,scratch,&defn);

		if (result) {
			deleted  = head.tag & IX_DEL;
			defnode  = head.tag & IX_DEF;
			head.tag = head.tag & MASK;
			qbl_name(head.tag,&name,&ext);
			printf(" loc  = %i\n",loc);
			printf(" rem1 = %i\n",head.rem1);
			printf(" rem2 = %i\n",head.rem2);
			printf(" ordp = %i\n",head.ordp);
			printf(" lang = %s\n",name);
			printf(" stat = %s %s\n",(deleted)?"del":"act",
											 (defnode)?"def":"   ");
			printf(" word = %s\n",scratch);
			printf(" defn = %s\n",defn);
		}
		else printf(" %s - no entry\n",token[ctok]);
	}
	return(0);
}

int sticky(ntok,token)
int   ntok;
char* token[];
{
	return(toggle(&sticky_on,ntok,token));
}

int tick(ntok,token)
int   ntok;
char* token[];
{
	struct tm *t;
	time_t aclock;
	char   str[5];
	int    value;

	/* get the time */
	time(&aclock);
	t = localtime(&aclock);
	value = 100*(t->tm_hour%13) + t->tm_min;

	/* set q value */
	token[0] = "q";
	token[1] = itoa(value,str,10);
	ntok = 2;

	/* and call qbl */
	qbl(ntok,token);
	return(0);
}

int use(ntok,token)
int   ntok;
char* token[];
{
	int cuse;

	if (ntok == 1) use_show();

	else if (!use_open(ntok,token,1)) {
		printf(" use list not changed\n");
		return(-1);
	}
	return(0);
}
