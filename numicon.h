/***
*numicom.h - include file for aiwass metaqabalah
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

#define MAX_FILE     8

#define NUM_BLK     16
#define NUM_IDX   4096
#define NUM_KEY      0
#define NUM_PRM      0

int  num_init     (int,char**);
int  num_add_word (int,int,char*);
int  num_del_word (int,int,char*);
int  num_evaluate (char*);
int  num_explicate(char*,int,int,int);
int  num_index    (void);
void num_term     (void);

int  use_check(int,char**);
int  use_open(int,char**,int);
int  use_set(int,char**);
int  use_show(void);
