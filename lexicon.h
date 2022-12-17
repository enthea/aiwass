/***
*lexicon.h - include file for aiwass metaqabalah
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

#define LEX_BLK     16
#define LEX_IDX  32752
#define LEX_KEY     40
#define LEX_PRM     64

#define LOCSET (byte)0xff
#define MASK   (byte)0x1f

int  lex_init(int,char**);
int  lex_add_word(byte,char*,char*,int);
int  lex_del_word(byte,char*);
int  lex_get_data(byte,int*,HEAD*,char*,char**);
int  lex_get_word(byte,int*,HEAD*,char*);
void lex_term(void);
