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
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "feed.h"
#include "ircbot.h"
#include "queue.h"

extern size_t line;
extern FILE *yyfeedin;

/*
 * Configuration for a feed.
 */
struct feed_config {
	char			*id;
	char			*name;
	char			*location;
	int			 refresh;
	int			 update;
	unsigned int		 showcount;
	LIST_HEAD(, feed_dest)	 channels;
};

/*
 * Feed destination (IRC channel).
 */
struct feed_dest {
	char			*id;
	char			*channel;
	LIST_ENTRY(feed_dest)	 link;
};

/*
 * Temporary feed configuration storage.
 */
static struct feed_config *tmp_fcp = NULL;

/*
 * Send events to this listener (if non-NULL).
 */
static void (*feed_listener)(FeedConfig) = NULL;

int yyfeedparse(void);
int yyfeedlex(void);
int yyfeederror(const char *);
%}

%token <string> STRING
%token <string> CHANNEL
%token <number> NUMBER

%token LBRACE RBRACE
%token TOKEN_FEED TOKEN_LOCATION TOKEN_REFRESH TOKEN_NAME TOKEN_CHANNELS
%token TOKEN_UPDATE TOKEN_COUNT

%union {
	char *string;
	int number;
}

%%

feeds		:
		| feeds feed
		;

feed		: TOKEN_FEED STRING LBRACE {
			tmp_fcp = xcalloc(1, sizeof(struct feed_config));
			tmp_fcp->id = $2;
		} feedopts RBRACE {
			if (feed_config_verify(tmp_fcp)) {
				feed_config_event(tmp_fcp);
			} else {
				feed_config_destroy(tmp_fcp);
			}
			tmp_fcp = NULL;
		}
		;

feedopts	:
		| feedopts feedopt
		;

feedopt		: TOKEN_LOCATION STRING {
			tmp_fcp->location = $2;
		}
		| TOKEN_REFRESH NUMBER {
			tmp_fcp->refresh = $2;
		}
		| TOKEN_COUNT NUMBER {
			tmp_fcp->showcount = $2;
		}
		| TOKEN_NAME STRING {
			tmp_fcp->name = $2;
		}
		| TOKEN_UPDATE STRING {
			if (strcasecmp("command", $2) == 0) {
				tmp_fcp->update = FALSE;
			} else if (strcasecmp("auto", $2) == 0) {
				tmp_fcp->update = TRUE;
			} else {
				yyfeederror("invalid update value");
				xfree($2);
				YYERROR;
			}
			xfree($2);
		}
		| TOKEN_CHANNELS channels
		;

channels	:
		| channels CHANNEL {
			struct feed_dest *fdp;
			char *p;

			if ((p = strchr($2, ':')) == NULL) {
				yyfeederror("invalid channel");
				xfree($2);
				YYERROR;
			}
			*p++ = '\0';

			fdp = xmalloc(sizeof(struct feed_dest));
			fdp->id = xstrdup($2);
			fdp->channel = xstrdup(p);
			LIST_INSERT_HEAD(&tmp_fcp->channels, fdp, link);

			xfree($2);
		}
		;
%%

/*
 * feed_config_set_listener --
 *	Sets the listener for configuration items, found when parsing.
 */
void
feed_config_set_listener(void (*listener)(FeedConfig))
{
	feed_listener = listener;
}

/*
 * feed_config_parse --
 *	Parses the configuration file.
 */
int
feed_config_parse(const char *file)
{
	FILE	*fp;
	int	 error;

	fp = fopen(file, "r");
	if (fp == NULL) {
		log_warn("Unable to open '%s' for reading", file);
		return (FALSE);
	}

	yyfeedin = fp;
	line = 1;

	error = yyfeedparse();

	fclose(fp);

	if (error) {
		log_warnx("Failed to parse configuration file '%s'.", file);
		return (FALSE);
	}

	return (TRUE);
}

/*
 * feed_config_event --
 *	Send the 'configuration item found' event through the listener.
 */
static void
feed_config_event(struct feed_config *fcp)
{
	if (feed_listener == NULL) {
		log_debug("No listener active; feed configuration %s trashed.",
		    fcp->id);
		/* No listener, destroy the resources. */
		feed_config_destroy(fcp);
		return;
	}

	/*
	 * Pass it to the listener. The listener is now responsible for
	 * destroying the resources.
	 */
	(*feed_listener)(fcp);
}

/*
 * feed_config_verify --
 *	Checks if the configuration values are correct.
 *	Fill in defaults for non-existing values.
 */
static int
feed_config_verify(struct feed_config *fcp)
{
	if (fcp->id == NULL)
		return (FALSE);

	if (fcp->location == NULL)
		return (FALSE);

	if (fcp->name == NULL)
		fcp->name = xstrdup(fcp->id);

	if (fcp->refresh == 0)
		fcp->refresh = 30;

	if (fcp->showcount == 0)
		fcp->showcount = 5;

	return (TRUE);
}

/*
 * feed_config_destroy --
 *	Free resources for a FeedConfig object.
 */
void
feed_config_destroy(FeedConfig fcp)
{
	struct feed_dest *fdp;

	/* Clean up all channels. */
	while ((fdp = LIST_FIRST(&fcp->channels)) != NULL) {
		LIST_REMOVE(fdp, link);
		xfree(fdp->id);
		xfree(fdp->channel);
		xfree(fdp);
	}

	xfree(fcp->location);
	xfree(fcp->id);
	xfree(fcp->name);
	xfree(fcp);
}

/*
 * feed_config_id --
 *	Accessor function for the 'id' member.
 */
const char *
feed_config_id(FeedConfig fcp)
{
	return (fcp->id);
}

/*
 * feed_config_name --
 *	Accessor function for the 'name' member.
 */
const char *
feed_config_name(FeedConfig fcp)
{
	return (fcp->name);
}

/*
 * feed_config_location --
 *	Accessor function for the 'location' member.
 */
const char *
feed_config_location(FeedConfig fcp)
{
	return (fcp->location);
}

/*
 * feed_config_refresh --
 *	Accessor function for the 'refresh' member.
 */
int
feed_config_refresh(FeedConfig fcp)
{
	return (fcp->refresh);
}

/*
 * feed_config_update --
 *	Accessor function for the 'update' member.
 */
int
feed_config_update(FeedConfig fcp)
{
	return (fcp->update);
}

/*
 * feed_config_showcount --
 *	Accessor function for the 'showcount' member.
 */
unsigned int
feed_config_showcount(FeedConfig fcp)
{
	return (fcp->showcount);
}

/*
 * feed_destination_first --
 *	Returns the first FeedDestination element in a FeedConfig.
 */
FeedDestination
feed_destination_first(FeedConfig fcp)
{
	return (LIST_FIRST(&fcp->channels));
}

/*
 * feed_destination_next --
 *	Returns the next FeedDestination element.
 */
FeedDestination
feed_destination_next(FeedDestination fdp)
{
	return (LIST_NEXT(fdp, link));
}

/*
 * feed_destination_id --
 *	Accessor function for the 'id' member.
 */
const char *
feed_destination_id(FeedDestination fdp)
{
	return (fdp->id);
}

/*
 * feed_destination_channel --
 *	Accessor function for the 'channel' member.
 */
const char *
feed_destination_channel(FeedDestination fdp)
{
	return (fdp->channel);
}

/*
 * feed_yacc_cleanup --
 *	Free memory allocated by the yacc generated file.
 */
void
feed_yacc_cleanup(void)
{
#ifndef HAVE_BISON
	if (yyss != NULL)
		free(yyss);
	if (yyvs != NULL)
		free(yyvs);
#endif /* !HAVE_BISON */
}
