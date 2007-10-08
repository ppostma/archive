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

#include "feed.h"
#include "feed_config_parse.h"
#include "ircbot.h"

#define YY_NO_UNPUT

size_t line;

int  yyfeederror(const char *);
int  yyfeedlex(void);
%}

%%
feed			{ return (TOKEN_FEED); }
location		{ return (TOKEN_LOCATION); }
refresh			{ return (TOKEN_REFRESH); }
update			{ return (TOKEN_UPDATE); }
name			{ return (TOKEN_NAME); }
channels		{ return (TOKEN_CHANNELS); }
count			{ return (TOKEN_COUNT); }

[A-Za-z]*\:\#[A-Za-z0-9_]* { /* channel */
				yyfeedlval.string = xstrdup(yyfeedtext);
				return (CHANNEL);
			}
[0-9]+			{ /* number */
				yyfeedlval.number = atoi(yyfeedtext);
				return (NUMBER);
			}
[A-Za-z0-9_\/\-\.\:]*	{ /* string */
				yyfeedlval.string = xstrdup(yyfeedtext);
				return (STRING);
			}
\"[^"\n]*["\n]		{ /* quoted string */
				yyfeedlval.string = xstrdup(yyfeedtext + 1);
				if (yyfeedlval.string[yyfeedleng - 2] != '"')
					yyfeederror("improperly terminated string");
				else
					yyfeedlval.string[yyfeedleng - 2] = '\0';
				return (STRING);
			}
\{			{ return (LBRACE); }
\}			{ return (RBRACE); }
[ \t]+			; /* ignore whitespace */
\n			| /* count lines */
^#.*\n			|
;.*\n			{ line++; }
.			{ yyfeederror("invalid character"); }
%%

/*
 * feed_config_scan_cleanup --
 *	Clean up used resources.
 */
void
feed_config_scan_cleanup(void)
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
}

int
yyfeedwrap(void)
{
	return (1);
}

int
yyfeederror(const char *str)
{
	log_warnx("%s at \"%s\", line %lu\n", str, yyfeedtext,
	    (unsigned long)line);

	return (0);
}
