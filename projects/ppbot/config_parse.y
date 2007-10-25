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

#include <stdio.h>
#include <string.h>

#include "ircbot.h"

/*
 * Temporary storage during parsing.
 */
static Connection conn;
static Channel chan;
static ChannelList chanlist;

/*
 * Yacc/Lex function prototypes.
 */
int yyparse(void);
int yylex(void);
int yyerror(const char *);
%}

%token <string> STRING
%token <string> CHANNEL
%token <boolean> BOOLEAN
%token <number> NUMBER

%token LBRACE RBRACE
%token TOKEN_SERVER TOKEN_ADDRESS TOKEN_PORT TOKEN_PASSWORD TOKEN_IDENT
%token TOKEN_REALNAME TOKEN_NICKNAME TOKEN_ALTNICK TOKEN_CHANNEL TOKEN_KEY
%token TOKEN_PLUGIN TOKEN_LOGFILE TOKEN_DEBUG

%union {
	char *string;
	int boolean;
	int number;
}

%%

commands	:
		| commands command
		;

command		: global
		| server
		;

global		: TOKEN_PLUGIN STRING {
			plugin_add($2);
			xfree($2);
		}
		| TOKEN_LOGFILE STRING {
			logfile_set($2);
			xfree($2);
		}
		| TOKEN_DEBUG BOOLEAN {
			set_logdebug($2);
		}
		;

server		: TOKEN_SERVER STRING LBRACE {
			conn = connection_find($2);
			if (conn != NULL && !connection_active(conn)) {
				xfree($2);
				yyerror("server already defined");
				YYERROR;
			} else if (conn == NULL) {
				conn = connection_create($2);
			}
			/*
			 * If the connection is active, then just continue
			 * with using the current connection.
			 */
		} serveropts RBRACE {
			xfree($2);
			conn = NULL;
		}
		;

serveropts	:
		| serveropts serveropt
		| serveropts channel
		;

serveropt	: TOKEN_ADDRESS STRING {
			connection_set_address(conn, $2);
			xfree($2);
		}
		| TOKEN_PORT STRING {
			connection_set_port(conn, $2);
			xfree($2);
		}
		| TOKEN_PASSWORD STRING {
			connection_set_password(conn, $2);
			xfree($2);
		}
		| TOKEN_IDENT STRING {
			connection_set_ident(conn, $2);
			xfree($2);
		}
		| TOKEN_REALNAME STRING {
			connection_set_realname(conn, $2);
			xfree($2);
		}
		| TOKEN_NICKNAME STRING {
			connection_set_nick(conn, $2);
			xfree($2);
		}
		| TOKEN_ALTNICK STRING {
			connection_set_alternate_nick(conn, $2);
			xfree($2);
		}
		;

channel		: TOKEN_CHANNEL CHANNEL {
			chanlist = connection_channels(conn);
			chan = channel_lookup(chanlist, $2);
			if (chan == NULL)
				channel_create(chanlist, $2, TRUE);
			xfree($2);
			chan = NULL;
		}
		| TOKEN_CHANNEL CHANNEL LBRACE {
			chanlist = connection_channels(conn);
			chan = channel_lookup(chanlist, $2);
			if (chan == NULL)
				chan = channel_create(chanlist, $2, TRUE);
		} channelopts RBRACE {
			xfree($2);
			chan = NULL;
		}
		;

channelopts	:
		| channelopts channelopt
		;

channelopt	: TOKEN_KEY STRING {
			channel_set_key(chan, $2);
			xfree($2);
		}
		| TOKEN_LOGFILE STRING {
			channel_set_logfile(chan, $2);
			xfree($2);
		}
		;
%%

/*
 * config_parse --
 *	Parse the configuration file.
 */
int
config_parse(const char *file)
{
	FILE *fp;
	int error;

	fp = fopen(file, "r");
	if (fp == NULL) {
		log_warn("Unable to open '%s' for reading", file);
		return (FALSE);
	}

	config_scan_initialize(fp);

	error = yyparse();

	fclose(fp);

	if (error) {
		log_warnx("Failed to parse configuration file '%s'.", file);
		return (FALSE);
	}

	return (TRUE);
}

/*
 * config_verify --
 *	Check if all needed values are set to connect to an IRC server.
 *	If the port is empty then it will be set to the default port (6667).
 *	If the ident or realname settings aren't set, then the value of
 *	the nickname will be used.
 *	Return TRUE if all values are OK, FALSE otherwise.
 */
int
config_verify(Connection c)
{
	int rv = TRUE;

	if (connection_address(c) == NULL) {
		log_warnx("[%s] No address defined.", connection_id(c));
		rv = FALSE;
	}
	if (connection_nick(c) == NULL) {
		log_warnx("[%s] No nickname defined.", connection_id(c));
		rv = FALSE;
	}
	if (rv != FALSE) {
		if (connection_port(c) == NULL) {
			/* Use default IRC port. */
			connection_set_port(c, "6667");
		}
		if (connection_ident(c) == NULL) {
			/* Use nickname. */
			connection_set_ident(c, connection_nick(c));
		}
		if (connection_realname(c) == NULL) {
			/* Use nickname. */
			connection_set_realname(c, connection_nick(c));
		}
	}

	return (rv);
}
