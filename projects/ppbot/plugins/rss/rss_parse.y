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
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ircbot.h"
#include "rss.h"

extern size_t line;
extern FILE *yyrssin;

static struct rss_config *rcp;

int yyrssparse(void);
int yyrsslex(void);
int yyrsserror(const char *);
%}

%token <string> STRING
%token <string> CHANNEL
%token <number> NUMBER

%token LBRACE RBRACE
%token TOKEN_FEED TOKEN_LOCATION TOKEN_REFRESH TOKEN_NAME TOKEN_CHANNELS
%token TOKEN_UPDATE TOKEN_COUNT TOKEN_IGNORE

%union {
	char *string;
	int number;
}

%%

feeds		:
		| feeds feed
		;

feed		: TOKEN_FEED STRING LBRACE {
			rcp = xcalloc(1, sizeof(struct rss_config));
			rcp->id = $2;
		} feedopts RBRACE {
			if (rss_config_verify(rcp)) {
				rss_head_create(rcp);
			} else {
				rss_config_free(rcp);
				rcp = NULL;
			}
		}
		;

feedopts	:
		| feedopts feedopt
		;

feedopt		: TOKEN_LOCATION STRING {
			rcp->location = $2;
		}
		| TOKEN_REFRESH NUMBER {
			rcp->refresh = $2;
		}
		| TOKEN_COUNT NUMBER {
			rcp->showcount = $2;
		}
		| TOKEN_NAME STRING {
			rcp->name = $2;
		}
		| TOKEN_UPDATE STRING {
			if (strcasecmp("command", $2) == 0) {
				rcp->update = FALSE;
			} else if (strcasecmp("auto", $2) == 0) {
				rcp->update = TRUE;
			} else {
				yyrsserror("invalid update value");
				xfree($2);
				YYERROR;
			}
			xfree($2);
		}
		| TOKEN_IGNORE STRING {
			rcp->ignore = $2;
		}
		| TOKEN_CHANNELS channels
		;

channels	:
		| channels CHANNEL {
			struct rss_dest *rdp;
			char *p;

			if ((p = strchr($2, ':')) == NULL) {
				yyrsserror("invalid channel");
				xfree($2);
				YYERROR;
			}
			*p++ = '\0';

			rdp = xmalloc(sizeof(struct rss_dest));
			rdp->id = xstrdup($2);
			rdp->channel = xstrdup(p);
			LIST_INSERT_HEAD(&rcp->channels, rdp, link);

			xfree($2);
		}
		;
%%

/*
 * rss_config_parse --
 *	Parses the configuration file.
 */
int
rss_config_parse(const char *file)
{
	FILE	*fp;
	int	 error;

	fp = fopen(file, "r");
	if (fp == NULL) {
		log_warn("Unable to open '%s' for reading", file);
		return (FALSE);
	}
	yyrssin = fp;
	line = 1;

	error = yyrssparse();

	fclose(fp);

	if (error) {
		log_warnx("Failed to parse configuration file '%s'.", file);
		return (FALSE);
	}

	return (TRUE);
}

/*
 * rss_config_verify --
 *	Checks if the configuration values are correct.
 */
static int
rss_config_verify(struct rss_config *r)
{
	if (r->id == NULL) {
		log_warnx("No ID defined for feed '%s'.", r->id);
		return (FALSE);
	}

	if (r->location == NULL) {
		log_warnx("No location defined for feed '%s'.", r->id);
		return (FALSE);
	}

	if (r->name == NULL) {
		log_warnx("No name defined for feed '%s', using ID.", r->id);
		r->name = xstrdup(r->id);
	}

	if (r->refresh == 0)
		r->refresh = 30;

	if (r->showcount == 0)
		r->showcount = 5;

	return (TRUE);
}

/*
 * rss_config_free --
 *	Free all space in a rss_config structure.
 */
void
rss_config_free(struct rss_config *r)
{
	struct rss_dest *rdp;

	/* Clean up all channels. */
	while ((rdp = LIST_FIRST(&r->channels)) != NULL) {
		LIST_REMOVE(rdp, link);
		xfree(rdp->id);
		xfree(rdp->channel);
		xfree(rdp);
	}

	xfree(r->ignore);
	xfree(r->location);
	xfree(r->id);
	xfree(r->name);
	xfree(r);
}

/*
 * rss_yacc_cleanup --
 *	Free memory allocated by the yacc generated file.
 */
void
rss_yacc_cleanup(void)
{
#ifndef HAVE_BISON
	if (yyss != NULL)
		free(yyss);
	if (yyvs != NULL)
		free(yyvs);
#endif /* !HAVE_BISON */
}
