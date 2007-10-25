/*
 * Copyright (c) 2006-2007 Peter Postma <peter@pointless.nl>
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

#include "config_parse.h"
#include "ircbot.h"

#define YY_NO_UNPUT

/*
 * Line counter.
 */
static size_t line;

/*
 * Lex function prototypes.
 */
int yyerror(const char *);
int yylex(void);
%}

%%
server			{ return (TOKEN_SERVER); }
address			{ return (TOKEN_ADDRESS); }
port			{ return (TOKEN_PORT); }
password		{ return (TOKEN_PASSWORD); }
ident			{ return (TOKEN_IDENT); }
realname		{ return (TOKEN_REALNAME); }
nickname		{ return (TOKEN_NICKNAME); }
altnick			{ return (TOKEN_ALTNICK); }
channel			{ return (TOKEN_CHANNEL); }
key			{ return (TOKEN_KEY); }
plugin			{ return (TOKEN_PLUGIN); }
logfile			{ return (TOKEN_LOGFILE); }
debug			{ return (TOKEN_DEBUG); }

false|no		{ /* boolean */
				yylval.boolean = FALSE;
				return (BOOLEAN);
			}
true|yes		{ /* boolean */
				yylval.boolean = TRUE;
				return (BOOLEAN);
			}
\#[A-Za-z0-9_\-\.]*	{ /* channel */
				yylval.string = xstrdup(yytext);
				return (CHANNEL);
			}
[A-Za-z0-9_\/\-\.]*	{ /* string */
				yylval.string = xstrdup(yytext);
				return (STRING);
			}
\"[^"\n]*["\n]		{ /* quoted string */
				yylval.string = xstrdup(yytext + 1);
				if (yylval.string[yyleng - 2] != '"')
					yyerror("improperly terminated string");
				else
					yylval.string[yyleng - 2] = '\0';
				return (STRING);
			}
\{			{ return (LBRACE); }
\}			{ return (RBRACE); }
[ \t]+			; /* ignore whitespace */
\n			| /* count lines */
^#.*\n			|
;.*\n			{ line++; }
.			{ yyerror("invalid character"); }
%%

/*
 * config_scan_reset --
 *	Reset the line counter.
 */
void
config_scan_reset(void)
{
	line = 1;
}

/*
 * yyerror --
 *	Lex error routine.
 */
int
yyerror(const char *str)
{
	log_warnx("%s at \"%s\", line %lu", str, yytext, (unsigned long)line);

	return (0);
}

/*
 * yywrap --
 *	Required by the Lex interface.
 */
int
yywrap(void)
{
	return (1);
}
