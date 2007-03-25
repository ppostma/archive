/*
 * Copyright (c) 2006 Peter Postma <peter@pointless.nl>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

%{
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ircbot.h"
#include "rss.h"
#include "rss_parse.h"

#define YY_NO_UNPUT

size_t line;

int  yyrsserror(const char *);
int  yyrsslex(void);
%}

%%
feed			{ return (TOKEN_FEED); }
location		{ return (TOKEN_LOCATION); }
refresh			{ return (TOKEN_REFRESH); }
update			{ return (TOKEN_UPDATE); }
name			{ return (TOKEN_NAME); }
channels		{ return (TOKEN_CHANNELS); }
count			{ return (TOKEN_COUNT); }
ignore			{ return (TOKEN_IGNORE); }

[A-Za-z]*\:\#[A-Za-z0-9_]* { /* channel */
				yyrsslval.string = xstrdup(yyrsstext);
				return (CHANNEL);
			}
[0-9]+			{ /* number */
				yyrsslval.number = atoi(yyrsstext);
				return (NUMBER);
			}
[A-Za-z0-9_\/\-\.\:]*	{ /* string */
				yyrsslval.string = xstrdup(yyrsstext);
				return (STRING);
			}
\"[^"\n]*["\n]		{ /* quoted string */
				yyrsslval.string = xstrdup(yyrsstext + 1);
				if (yyrsslval.string[yyrssleng - 2] != '"')
					yyrsserror("improperly terminated string");
				else
					yyrsslval.string[yyrssleng - 2] = '\0';
				return (STRING);
			}
\{			{ return (LBRACE); }
\}			{ return (RBRACE); }
[ \t]+			; /* ignore whitespace */
\n			| /* count lines */
^#.*\n			|
;.*\n			{ line++; }
.			{ yyrsserror("invalid character"); }
%%

void
rss_lex_cleanup(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
}

int
yyrsswrap(void)
{
	return (1);
}

int
yyrsserror(const char *str)
{
	log_warnx("%s at \"%s\", line %lu", str, yyrsstext,
	    (unsigned long)line);

	return (0);
}
