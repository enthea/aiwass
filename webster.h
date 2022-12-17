/***
*webster.h - include file for aiwass metaqabalah
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
****************************************************************************/

#pragma once

/* type definitions for indexed file routines */

#define IX_READ  1
#define IX_WRITE 2

#define IX_DEF   '\x40'
#define IX_DEL   '\x80'

#define IX_OPEN  O_BINARY|O_RDWR
#define IX_CREAT O_CREAT|O_EXCL|IX_OPEN

typedef struct {
	char  path[_MAX_PATH];
	int   handle;
	int   blksiz;
	int   idxsiz;
	int   keysiz;
	uint  idxsizb;

	uint  dirty;
	uint* index;
	uint* primes;
} IXF;

typedef struct {
	IXF*  ixf;
	int   mode;
	int   rec;

	uint  blk;
	uint  dirty;
	uint* nxtblk;

	byte* buffer;
	int   cbyte;
	int   nbyte;
} RST;

typedef struct {
	byte  tag;
	byte  ordp;
	int   rem1;
	int   rem2;
} HEAD;


int      clixf(IXF*);
int      clixr(RST*);
int      flixf(IXF*);
RST*     ixaddr(IXF*,HEAD*,int,byte);
RST*     ixdelr(IXF*,HEAD*,int,byte);
#define  ixempty(ix,rec) (ix->index[rec+1] == 0)
int      ixerror(char*,int,char*);
int      ixgetb(RST*,byte*,int);
int      ixhash(char*,int,int);
int      ixlocr(IXF*,HEAD*,int*,byte,byte,char*);
int      ixmodb(RST*,byte*,int);
int      ixprime(uint*,uint,int);
int      ixputb(RST*,byte*,int);
IXF*     opixf(char*,int,int,int,int);
RST*     opixr(IXF*,int,int);
