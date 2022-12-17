/***
*qabalah.c - qabalistic processor (for aiwass metaqabalah)
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
*   This file contains the code for maintaining and using
*   multiple qabalistic keys (for aiwass metaqabalah), such
*   as loading key files, and computing the qabalistic value
*   of a given word by a given key.
*
****************************************************************************/


#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#include "qabalah.h"
#include "scratch.h"
#include "webster.h"

typedef struct {
	char str[4];
	int  ord;
	int  val;
} QBL;

typedef struct {
	char name[_MAX_FNAME];
	char ext [_MAX_EXT];
	int  maxlen;
	QBL* qbl;
} KEY;

static KEY  key   [MAX_KEYS];
static uint primes[QBL_PRIMES+1];

static char qbl_drive[_MAX_DRIVE];
static char qbl_dir  [_MAX_DIR];
static char qbl_fname[_MAX_FNAME];
static char qbl_ext  [_MAX_EXT];

static IXF* sys;


int qbl_init(ntok,token)
int   ntok;
char* token[];
{
	RST*  rst;
	int   ckey;
	char* path;

	/* open the system file */

	_splitpath(token[0],qbl_drive,qbl_dir,qbl_fname,qbl_ext);
	_makepath(scratch,qbl_drive,qbl_dir,"aiwass","sys");
	sys = opixf(scratch,SYS_BLK,SYS_IDX,SYS_KEY,SYS_PRM);
	if (!sys) {
		printf(" error opening system file\n");
		return(0);
	}

	/* load the environment variables */

	path = getenv("QABALAH");
	if (!path) path = getenv("AIWASS");
	if (!path) path = "c:\\aiwass\\";
	_splitpath(path,qbl_drive,qbl_dir,qbl_fname,qbl_ext);

	/* set up the list of primes for qbl_find */

	ixprime(primes,QBL_CELLS,QBL_PRIMES);

	/* load the qabalah data */

	strcpy(key[0].name,"unknown");
	for (ckey=1; ckey<MAX_KEYS; ckey++) {
		if ((rst = opixr(sys,ckey,IX_READ))) {
			ixgetb(rst,key[ckey].name,0);
			ixgetb(rst,key[ckey].ext,0);
			qbl_load(ckey);
			clixr(rst);
		}
	}

	/* set up to close at termination */

	return(!atexit(qbl_term));
}

char* qbl_etoi(ckey, string)
int ckey;
char string[];
{
}

int qbl_eval(ckey, string)
int   ckey;
char* string;
{
	QBL*  qbl = key[ckey].qbl;
	int   len = key[ckey].maxlen;
	int   i,j;
	int   found;
	int   loc;
	int   value;
	char  c;


	value = 0;
	for (i=0; string[i]; ) {

		for (j=i+1; j<i+len; j++) {
			if (!string[j])         break;
			if (isupper(string[j])) break;
		}

		for ( ; j>i; j--) {
			c = string[j]; string[j] = 0;
			found = qbl_find(ckey,&loc,&string[i]);
			string[j] = c; if (found == 1) break;
		}
		if (found == 1) value += qbl[loc].val;

		i = max(j,i+1);
	}
	return(value);
}

int qbl_find(ckey,loc,string)
int   ckey;
int*  loc;
char* string;
{
	QBL*  qbl = key[ckey].qbl;
	long  str1 = 0L;
	long  str2;
	int   i;

	strncpy((char*)&str1,string,3);
	for (i=1; i<primes[0]; i++) {
		*loc = 1 + str1 % primes[i];
		str2 = *((long*)qbl[*loc].str);
		if (str2 == str1) return(1);
		if (str2 == 0L)   return(0);
	}
	return(-1);
}

int qbl_key(ckey,string)
int*  ckey;
char* string;
{
	int flag;

	if (flag = (isalpha(string[0]) && !string[1]))
		*ckey = toupper(string[0]) - '@';
	else
		printf(" %s - not a key code\n",string);

	return(flag);
}

int qbl_load(ckey)
int   ckey;
{
	KEY*  k = &key[ckey];
	QBL*  qbl;
	FILE* stream;
	char* token[64];
	char* ext;
	char* name;

	int    ctok;
	int    loc;
	size_t nbytes;
	int    ntok;
	int    ord;
	int    val;


	if (!qbl_name(ckey,&name,&ext)) return(0);
	if (qbl_loaded(ckey)) free(k->qbl);

	/* allocate space for the key, return if not available */

	nbytes = QBL_CELLS*sizeof(QBL);
	qbl = k->qbl = (QBL*)calloc(nbytes,1);
	if (!qbl) return(0);

	/* open key file for input, return if open fails */

	_makepath(scratch,qbl_drive,qbl_dir,k->name,"key");
	stream = fopen(scratch,"r");
	if (!stream) {
		k->qbl = NULL;
		free(qbl);
		return(0);
	}

	/* read the keyfile, and set up the key arrays */

	ord = 0; k->maxlen = 0;
	while (fgets(scratch,128,stream)) {

		token[0] = strtok(scratch," \t\n");
		for (ntok = 0; token[ntok]; )
			token[++ntok] = strtok(NULL," \t\n");

		ord++; val = atoi(token[0]);
		for (ctok=1; ctok<ntok; ctok++) {
			nbytes = strlen(token[ctok]);
			k->maxlen = max(k->maxlen,nbytes);
			qbl_set(ckey,ord,val,token[ctok]);
			if (qbl_find(ckey,&loc,strlwr(token[ctok])) == 0)
				qbl_set(ckey,ord,val,token[ctok]);
		}
	}
	fclose(stream);
	return(1);
}

int qbl_loaded(ckey)
int   ckey;
{
	return(key[ckey].qbl != NULL);
}

int qbl_name(ckey,name,ext)
int    ckey;
char** name;
char** ext;
{
	*name = key[ckey].name;
	*ext  = key[ckey].ext;
	return(**name);
}

int qbl_set(ckey,ord,val,string)
int   ckey;
int   ord;
int   val;
char* string;
{
	QBL*  qbl = key[ckey].qbl;
	int   found;
	int   loc;
	char* str;

	found = qbl_find(ckey,&loc,string);
	if (found < 0) return(found);
	str = qbl[loc].str;

	memset(str,0,4);
	strncpy(str,string,3);
	qbl[loc].val = val;
	qbl[loc].ord = ord;
	return(loc);
}

int qbl_setname(ckey,name,ext)
int   ckey;
char* name;
char* ext;
{
	RST* rst;

	if (qbl_loaded(ckey)) free(key[ckey].qbl);
	strncpy(key[ckey].name,name,_MAX_FNAME-1);
	strncpy(key[ckey].ext, ext, _MAX_EXT-1);
	if ((rst = opixr(sys,ckey,IX_WRITE))) {
		ixputb(rst,key[ckey].name,0);
		ixputb(rst,key[ckey].ext, 0);
		clixr(rst);
	}
	return(*name && *ext);
}

void qbl_term()
{
	int ckey;

	for (ckey=0; ckey<MAX_KEYS; ckey++)
		if (key[ckey].qbl)
			free(key[ckey].qbl);
	clixf(sys);
}
