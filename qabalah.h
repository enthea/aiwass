/***
*qabalah.h - include file for aiwass metaqabalah
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

#define MAX_KEYS    27

#define QBL_CELLS  128
#define QBL_PRIMES   8

#define SYS_BLK     16
#define SYS_IDX     32
#define SYS_KEY      0
#define SYS_PRM      0

int   qbl_init    (int,char**);
char* qbl_etoi    (int,char*);
int   qbl_eval    (int,char*);
int   qbl_find    (int,int*,char*);
int   qbl_key     (int*,char*);
int   qbl_load    (int);
int   qbl_loaded  (int);
int   qbl_name    (int,char**,char**);
int   qbl_set     (int,int,int,char*);
int   qbl_setname (int,char*,char*);
void  qbl_term    (void);
