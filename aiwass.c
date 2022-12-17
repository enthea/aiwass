/***
*aiwass.c - command processor for aiwass metaqabalah
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
*   This file contains the code for recognizing the commands
*   for aiwass metaqabalah, calling the required command
*   functions, and reporting syntax errors and the like.
*
****************************************************************************/


#include <search.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aiwass.h"
#include "command.h"
#include "lexicon.h"
#include "numicon.h"
#include "qabalah.h"
#include "scratch.h"

typedef struct {
	char* name;
	char* parm;
	int   amin;
	int   amax;
	int (*func)(int,char**);
} CTAB;

#define MAX_COMMAND 18
int max_com = MAX_COMMAND;

CTAB ctab[MAX_COMMAND] = {
	{ "add",      "key word*",          2, 9, add      },
	{ "add/def",  "key word*",          2, 9, add      },
	{ "addgloss", "key path*",          2, 9, addgloss },
	{ "alpha",    "[ OFF | ON ]",       0, 1, alpha    },
	{ "def",      "[ OFF | ON ]",       0, 1, def      },
	{ "del",      "key word*",          2, 9, del      },
	{ "eval",     "word*",              1, 9, eval     },
	{ "index",    "keykey",             1, 1, indexx   },
	{ "keys",     "",                   0, 0, keys     },
	{ "q",        "( word | num )*",    1, 9, qbl      },
	{ "qbl",      "( word | num )*",    1, 9, qbl      },
	{ "quit",     "",                   0, 0, quit     },
	{ "range",    "from to",            2, 2, range    },
	{ "setkey",   "key [ name ext ]",   1, 3, setkey   },
	{ "show",     "word*",              1, 9, show     },
	{ "sticky",   "[ OFF | ON ]",       0, 1, sticky   },
	{ "tick",     "[ option ]",         0, 1, tick     },
	{ "use",      "[ keykey* ]",        0, 9, use      }
};

static char command[MAX_STR];


main(ntok,token)
int   ntok;
char* token[];
{
	FILE* stream = NULL;
	CTAB* curcom;
	char* result;
	char* string;

	int   indent = 0;
	int   value;


	/* initialize the system */

	if (!qbl_init(ntok,token)) exit(1);

	/* initialize the lexicon */

	ntok = 1;      token[0] = getenv("LEXICON");
	if (!token[0]) token[0] = getenv("AIWASS");
	if (!token[0]) token[0] = "c:\\aiwass\\";
	if (!lex_init(ntok,token)) exit(1);

	/* initialize the numicon */

	ntok = 1;      token[0] = getenv("NUMICON");
	if (!token[0]) token[0] = getenv("AIWASS");
	if (!token[0]) token[0] = "c:\\aiwass\\";
	if (!num_init(ntok,token)) exit(1);

	/* attempt to open aiwass.ini */

	stream = fopen("aiwass.ini","r");
	if (!stream) errno = 0;

	/* set up to handle ctrl-c */
	if (signal(SIGINT,handler) == SIG_ERR) {
		printf("Couldn't set SIGINT\n");
		exit(1);
	}


	/* main command loop */

	for (done=0; !done; ) {

		/* check for ctrl-c */

		if (ctlc) {
			ctlc = 0;
			if (stream) {
				fclose(stream);
				stream = NULL;
			}
		}

		/* output user prompt */

		printf("aiwass>");

		/* get command from stream, if open */

		if (stream) {
			result = fgets(&command[indent],MAX_STR-indent,stream);
			if (result) printf(command);
			else {
				fclose(stream);
				stream = NULL;
			}
		}

		/* get command from stdin, if closed */

		if (!stream) {
			result = fgets(command,MAX_STR,stdin);
			if (!result) break;
		}

		/* parse the command line */

		strncpy(scratch,command,MAX_STR-1);
		token[0] = strtok(scratch," \t\n");
		for (ntok = 0; token[ntok]; )
			token[++ntok] = strtok(NULL," \t\n");

		/* handle redirection command */

		if (*token[0] == '@' || *token[ntok-1] == '<') {
			if (stream) printf(" @ not allowed from file\n");
			else {
				indent = token[ntok-1] - scratch;
				stream = fopen(++token[ntok-1],"r");
				if (!stream) perror(token[ntok-1]);
			}
		}

		/* else lookup and execute command */

		else if (ntok) {
			curcom = (CTAB*)lfind((char*)token,(char*)ctab,
								  &max_com,sizeof(CTAB),compare);

			if (!curcom) system(command);
			else if (curcom->amin <= ntok-1 && ntok-1 <= curcom->amax) {
				if (value = curcom->func(ntok,token))
					printf(" %s returned %i\n",curcom->name,value);
			} else printf(" usage: %s %s\n",curcom->name,curcom->parm);
		}
	}

	printf(" exiting aiwass\n");
	return(0);
}

int compare(arg1,arg2)
char** arg1;
char** arg2;
{
	return(strcmpi(*arg1,*arg2));
}

void handler()
{
	signal(SIGINT, SIG_IGN);
	if (ctlc++) {
		printf("\n exiting aiwass\n");
		exit(1);
	}
	signal(SIGINT, handler);
}

int toggle(flag,ntok,token)
int*  flag;
int   ntok;
char* token[];
{
	int error = 0;

	if (ntok == 1)
		printf(" %s is %s\n",strupr(token[0]),*flag ? "ON" : "OFF");
	else if (ntok == 2) {
		if      (strcmpi(token[1],"OFF") == 0) *flag = 0;
		else if (strcmpi(token[1],"ON")  == 0) *flag = 1;
		else error = -1;
	} else error = -1;
	return(error);
}
