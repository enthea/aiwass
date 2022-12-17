/***
*scriber.c - output processor for aiwass metaqabalah
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
*   This file contains the code for presenting the output
*   (for aiwass metaqabalah), such as displaying numbers,
*   words, and definitions, performing line wrapping,
*   maintaining margins, and so forth.
*
****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scratch.h"
#include "scriber.h"

int   llen;
int   lmar=8;
char  null='\0';
char* punc=&null;


void convert(string1,string2)
char* string1;
char* string2;
{
	int i,j;

	for (i=0,j=0; string2[j] != '\0'; i++,j++) {
		if (string2[j+1] == ':') {
			switch(string2[j]) {
				case 'a': string1[i] = 131; break;
				case 'e': string1[i] = 136; break;
				case 'i': string1[i] = 140; break;
				case 'o': string1[i] = 147; break;
				case 'u': string1[i] = 150; break;
				default : string1[i] = string2[j]; j--;
			}
			j++;
		} else string1[i] = string2[j];
	}
	string1[i++] = '\0';
}

void scribe_eol()
{
	printf("\n");
	punc = &null;
	llen = 0;
}

void scribe_mar()
{
	int i;

	for (i=1; i<=lmar; i++)
		printf(" ");
}

void scribe_num(num,width,string2)
int   num;
int   width;
char* string2;
{
	int  len;
	char string[17];
	char string1[MAX_STR];

	len = strlen(itoa(num,string,10));
	if (len < width) {
		memset(string1,' ',width-len);
		strcpy(&string1[width-len],string);
	}
	else strcpy(string1,string);
	scribe_word(string1,string2);
}

void scribe_word(string1,string2)
char* string1;
char* string2;
{
	int i;
	int len;

	if (!colon_on)
		convert(scratch,string1);
	else
		strcpy(scratch,string1);

	len = strlen(scratch);
	llen += strlen(punc)+len;
	if (llen < 78) {
		printf("%s%s",punc,scratch);
	}
	else {
		printf("%s\n",punc);
		llen = lmar + len;
		for (i=1; i<=lmar; i++)
			printf(" ");
		printf("%s",scratch);
	}
	punc = string2;
}

void set_margin(n)
int n;
{
	lmar = n;
}
