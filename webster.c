/***
*webster.c - general purpose blocked indexed file routines
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
*   This file contains the code for creating and maintaining
*   blocked indexed files, including scatter table files.
*
****************************************************************************/


#include <sys\types.h>
#include <sys\stat.h>

#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#include "scratch.h"
#include "webster.h"

int rdixb(RST*,uint*);
int wrixb(RST*);

enum ixerr
	{ixne,ixab,ixai,ixas,ixfc,ixfo,ixfr,ixfs,ixfw,ixif,ixps,ixrr,ixrs};

static char* ixmsg[13] =
	{ "no error",             /* ixne */
	  "alloc buffer failed",  /* ixab */
	  "alloc index  failed",  /* ixai */
	  "alloc struct failed",  /* ixas */
	  "file close failed",    /* ixfc */
	  "file open failed",     /* ixfo */
	  "file read failed",     /* ixfr */
	  "file seek failed",     /* ixfs */
	  "file write failed",    /* ixfw */
	  "index file full",      /* ixif */
	  "prime setup failed",   /* ixps */
	  "record range error",   /* ixrr */
	  "record status error"   /* ixrs */
	};


int clixf(ix)
IXF*  ix;
{
	if (!ix) return(0);

	if (ix->dirty)
		if (!flixf(ix)) return(0);

	if (close(ix->handle) == -1)
		return(ixerror(ix->path,ixfc,"clixf"));

	free(ix->index);
	free(ix);
	return(1);
}

int clixr(rst)
RST*  rst;
{
	if (!rst) return(0);

	if (rst->dirty)
		if (!wrixb(rst)) return(0);

	free(rst->buffer);
	free(rst);
	return(1);
}

int flixf(ix)
IXF*  ix;
{
	long  pos;
	int   len;

	pos = lseek(ix->handle,0L,SEEK_SET);
	if (pos == -1L) return(ixerror(ix->path,ixfs,"flixf"));

	len = write(ix->handle,(char*)ix->index,ix->idxsizb);
	if (len != ix->idxsizb) return(ixerror(ix->path,ixfw,"flixf"));

	ix->dirty = 0;
	return(1);
}

RST* ixaddr(ix,head,loc,tag)
IXF*  ix;
HEAD* head;
int   loc;
byte  tag;
{
	RST* rst;

	head->tag = tag;
	if ((rst = opixr(ix,loc,IX_WRITE))) {
		ixputb(rst,(byte*)head,sizeof(HEAD));
	}
	return(rst);
}

RST* ixdelr(ix,head,loc,tag)
IXF*  ix;
HEAD* head;
int   loc;
byte  tag;
{
	RST* rst;

	head->tag = tag | IX_DEL;
	if ((rst = opixr(ix,loc,IX_WRITE))) {
		ixputb(rst,(byte*)head,sizeof(HEAD));
	}
	return(rst);
}

int ixerror(path,code,func)
char* path;
int   code;
char* func;
{
	if (errno) perror("");
	fprintf(stderr,"%s on %s (%s)\n",ixmsg[code],path,func);
	return(0);
}

int ixgetb(rst,item,size)
RST*  rst;
byte* item;
int   size;
{
	IXF* ix;
	register int i = 0;
	register int s = size;


	if (!rst) return(0);

	ix = rst->ixf;
	rst->cbyte = rst->nbyte;

	do {
		if (rst->nbyte == ix->blksiz)
			rdixb(rst,rst->nxtblk);
		item[i++] = rst->buffer[rst->nbyte++];
	} while ( (i < s) || (!s && item[i-1]) );

	return(1);
}

int ixhash(string,prime,length)
char* string;
int   prime;
int   length;
{
	int   halfln;
	int   j;

	ulong k = 0L;
	ulong p = prime;
	uint* w = (uint*)scratch;


	length = max(length,MAX_STR-1);
	halfln = (length+1)/2;
	memset(scratch,0,halfln*2);
	strncpy(scratch,string,length);

	for (j = 0; j < halfln; j++)
		k = ((k << 16) + w[j]) % p;

	return((int)k);
}

int ixlocr(ix,head,loc,mask,tag,string)
IXF*  ix;
HEAD* head;
int*  loc;
byte  mask;
byte  tag;
char* string;
{
	RST* rst;
	int  del;
	int  match;
	int  rem0;
	int  rem1;
	int  rem2;
	byte test;

	int  dloc = -1;
	byte ordp =  1;

	rem0 = ixhash(string,ix->primes[1],ix->keysiz);
	rem1 = ixhash(string,ix->primes[2],ix->keysiz);
	rem2 = ixhash(string,ix->primes[3],ix->keysiz);
	*loc = rem0;

	while (!ixempty(ix,rem0)) {
		if (!(rst = opixr(ix,rem0,IX_READ))) return(-2);
		ixgetb(rst,(byte*)head,sizeof(HEAD));
		clixr(rst);

		del = ((head->tag & IX_DEL) != 0);
		if (del && (dloc == -1)) dloc = rem0;

		test  = head->tag & mask;
		match = !tag || !test || (tag == test);

		if (match
		 && (head->ordp == ordp)
		 && (head->rem1 == rem1)
		 && (head->rem2 == rem2))
			return(1);

		if (ordp+2 == (byte)ix->primes[0]) {
			if (dloc == -1) return(-1);
			break;
		}

		ordp++;
		rem0 = rem1;
		rem1 = rem2;
		rem2 = ixhash(string,ix->primes[ordp+2],ix->keysiz);
		*loc = rem0;
	}

	if (dloc != -1) {
		if (!(rst = opixr(ix,dloc,IX_READ))) return(-2);
		ixgetb(rst,(byte*)head,sizeof(HEAD));
		clixr(rst);
	}
	else {
		head->tag  = tag;
		head->ordp = ordp;
		head->rem1 = rem1;
		head->rem2 = rem2;
	}
	return(0);
}

int ixmodb(rst,item,size)
RST*  rst;
byte* item;
int   size;
{
	int i;

	if (!rst) return(0);

	for (i=0; i<size; i++)
		rst->buffer[rst->cbyte+i] = item[i];

	rst->dirty = 1;
	return(1);
}

int ixprime(table,maxp,nump)
uint* table;
uint  maxp;
int   nump;
{
	uint  i,j;
	uint  limit;
	char* sieve;


	/* allocate space for sieve */

	sieve = (char*)calloc(maxp+1,sizeof(char));
	if (!sieve) return(0);

	/* raise flag for non-primes in sieve */

	limit = maxp/2;
	for (i=2; i<=limit; i++)
		if (!sieve[i])
			for (j=i+i; j<=maxp; j+=i)
				sieve[j]=1;

	/* transfer top primes to table */

	for (i=maxp, j=0; (i>1) && (j<nump); i--)
		if(!sieve[i])
			table[++j] = i;
	table[0] = j;
	free(sieve);
	return(1);
}

int ixputb(rst,item,size)
RST*  rst;
byte* item;
int   size;
{
	IXF* ix;
	register int i = 0;
	register int s = size;


	if (!rst) return(0);

	ix = rst->ixf;
	rst->cbyte = rst->nbyte;

	do {
		if (rst->nbyte == ix->blksiz) {
			rdixb(rst,rst->nxtblk);
			rst->dirty = 1;
		}
		rst->buffer[rst->nbyte++] = item[i++];
	} while ( (i < s) || (!s && item[i-1]) );

	rst->dirty = 1;
	return(1);
}

IXF* opixf(path,blksiz,idxsiz,keysiz,nprimes)
char* path;
int   blksiz;
int   idxsiz;
int   nprimes;
{
	IXF*   ix;
	int    create;
	size_t nbytes;
	size_t nbyte2;

	enum ixerr errcode;

	/* allocate space for IXF struct and primes */

	nbytes = sizeof(IXF) + sizeof(uint)*(nprimes+1);
	if (!(ix = (IXF*)calloc(nbytes,1))) {
		errcode = ixas;
		goto error;
	}

	/* allocate space for the IXF index */

	ix->index = (uint*)calloc(idxsiz,sizeof(uint));
	if (!ix->index) {
		errcode = ixai;
		goto error;
	}

	/* open or create the IXF file */

	ix->handle = open(path,IX_OPEN);
	if (create = (ix->handle == -1))
		ix->handle = open(path,IX_CREAT,S_IREAD|S_IWRITE);
	if (ix->handle == -1) {
		errcode = ixfo;
		goto error;
	}

	/* write or read the IXF index */

	nbytes = idxsiz*sizeof(uint);
	if (create)
		nbyte2 = write(ix->handle,(char*)ix->index,nbytes);
	else
		nbyte2 = read (ix->handle,(char*)ix->index,nbytes);
	if (nbyte2 != nbytes) {
		errcode = create ? ixfw : ixfr;
		goto error;
	}

	/* perhaps calculate the primes */

	ix->primes = (uint*)((char*)ix + sizeof(IXF));
	if (nprimes) {
		if (!ixprime(ix->primes,idxsiz-1,nprimes)) {
			errcode = ixps;
			goto error;
		}
	}

	/* finally, fill in the IXF struct */

	strncpy(ix->path,path,_MAX_PATH-1);
	ix->blksiz  = blksiz;
	ix->idxsiz  = idxsiz-1;
	ix->keysiz  = keysiz;
	ix->idxsizb = nbytes;
	return(ix);

error:
	switch (errcode) {
		case ixps:
		case ixfw:
		case ixfr: close(ix->handle);
		case ixfo: free(ix->index);
		case ixai: free(ix);
	}
	ixerror(path,errcode,"opixf");
	return(NULL);
}

RST* opixr(ix,rec,mode)
IXF*  ix;
int   rec;
int   mode;
{
	RST* rst;

	enum ixerr errcode;


	if (!ix) return(NULL);

	if ((rec < 0) || (rec > ix->idxsiz-1)) {
		errcode = ixrr;
		goto error;
	}

	rst = (RST*)calloc(sizeof(RST)+ix->blksiz,1);
	if (!rst) {
		errcode = ixab;
		goto error;
	}

	rst->ixf    = ix;
	rst->mode   = mode;
	rst->rec    = rec;

	rst->blk    = 0;
	rst->buffer = (byte*)rst+sizeof(RST);
	rst->nxtblk = (int*)rst->buffer;
	rdixb(rst,&ix->index[rec+1]);
	return(rst);

error:
	ixerror(ix->path,errcode,"opixr");
	return(NULL);
}

int rdixb(rst,n)
RST*   rst;
uint*  n;
{
	IXF*  ix;
	long  pos;
	int   end;
	int   len;
	int   i;

	ix = rst->ixf;
	end = (*n == 0);
	if (end && (rst->mode == IX_WRITE)) {

		if ((*n = ix->index[0] + 1)) ++ix->index[0];
		else return(ixerror(ix->path,ixif,"rdixb"));

		if (n == rst->nxtblk)
			rst->dirty = 1;
		else
			ix->dirty = 1;
	}

	if (rst->dirty)
		if (!wrixb(rst)) return(0);

	rst->blk   = *n;
	rst->nbyte = 2;
	rst->cbyte = 2;

	if (end) {
		for (i=0; i<ix->blksiz; i++) rst->buffer[i] = 0;
		if  (rst->mode == IX_WRITE)  rst->dirty     = 1;
	}
	else {
		pos = lseek(ix->handle,(*n-1L)*ix->blksiz+ix->idxsizb,SEEK_SET);
		if (pos == -1L) return(ixerror(ix->path,ixfs,"rdixb"));

		len = read(ix->handle,rst->buffer,ix->blksiz);
		if (len != ix->blksiz) return(ixerror(ix->path,ixfr,"rdixb"));
	}
	return(1);
}

int wrixb(rst)
RST*  rst;
{
	IXF*  ix;
	long  pos;
	int   len;
	int   n;

	ix = rst->ixf;
	if (!(n = rst->blk))
		return(ixerror(ix->path,ixrs,"wrixb"));

	pos = lseek(ix->handle,(n-1L)*ix->blksiz+ix->idxsizb,SEEK_SET);
	if (pos == -1L) return(ixerror(ix->path,ixfs,"wrixb"));

	len = write(ix->handle,rst->buffer,ix->blksiz);
	if (len != ix->blksiz) return(ixerror(ix->path,ixfw,"wrixb"));

	rst->dirty = 0;
	return(1);
}
