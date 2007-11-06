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

/*
 * Auto ops plugin for ppbot.
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "plugin.h"
#include "queue.h"

/*
 * Name & version of the plugin.
 */
#define PLUGIN_NAME	"auto ops"
#define PLUGIN_VERSION	"20071104"

/*
 * File with operators.
 */
static const char ops_file[] = "ops.conf";

/*
 * Local function prototypes.
 */
static void handle_join(Plugin, Connection, Message);
static void handle_privmsg(Plugin, Connection, Message);

/*
 * List with operators.
 */
struct operator {
	char			*userhost;
	LIST_ENTRY(operator)	 link;
};

static LIST_HEAD(, operator) operators = LIST_HEAD_INITIALIZER(&operators);

/*
 * plugin_open --
 *	Called when the plugin is loaded/opened.
 */
void
plugin_open(Plugin p)
{
	struct operator	*op;
	FILE		*fp;
	char		 buf[BUFSIZ];
	char		*ch;
	
	/* Read the operators from the config file. */
	fp = fopen(ops_file, "r");
	if (fp == NULL) {
		log_plugin(LOG_WARNING, p, "Unable to open '%s' for reading",
		    ops_file);
	} else {
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			if ((ch = strchr(buf, '\n')) != NULL)
				*ch = '\0';
			if (strlen(buf) == 0)
				continue;

			/* Add the operator to the list. */
			op = xmalloc(sizeof(struct operator));
			op->userhost = xstrdup(buf);
			LIST_INSERT_HEAD(&operators, op, link);
		}
		fclose(fp);
	}

	callback_register(p, "JOIN", MSG_USER, 0, handle_join);
	callback_register(p, "PRIVMSG", MSG_USER|MSG_DATA, 1, handle_privmsg);

	log_plugin(LOG_INFO, p, "Loaded plugin %s v%s.",
	    PLUGIN_NAME, PLUGIN_VERSION);
}

/*
 * plugin_close --
 *	Called when the plugin is unloaded/closed.
 */
void
plugin_close(Plugin p)
{
	struct operator	*op;

	/* Destroy the operator list. */
	while ((op = LIST_FIRST(&operators)) != NULL) {
		LIST_REMOVE(op, link);
		xfree(op->userhost);
		xfree(op);
	}
}

/*
 * is_operator --
 *	Checks if the userhost is an operator.
 */
static int
is_operator(const char *userhost)
{
	struct operator	*op;

	LIST_FOREACH(op, &operators, link) {
		if (strcasecmp(op->userhost, userhost) == 0)
			return (TRUE);
	}

	return (FALSE);
}

/*
 * handle_join --
 *	Called when a JOIN message is received.
 */
static void
handle_join(Plugin p, Connection conn, Message msg)
{
	const char *channel;

	channel = (message_parameter(msg, 0) != NULL) ?
	    message_parameter(msg, 0) : message_data(msg);
	if (channel == NULL) {
		log_plugin(LOG_DEBUG, p, "No channel in JOIN message (%s).",
		    connection_id(conn));
		return;
	}

	/* If the userhost is operator, then op him. */
	if (is_operator(message_userhost(msg))) {
		log_plugin(LOG_DEBUG, p,
		    "Giving operator status to '%s' in %s (%s).",
		    message_sender(msg), channel, connection_id(conn));

		send_raw(conn, "MODE %s +o %s", channel, message_sender(msg));
	}
}

/*
 * handle_privmsg --
 *      Called when a PRIVMSG message is received.
 */
static void
handle_privmsg(Plugin p, Connection conn, Message msg)
{
	const char *sender = message_sender(msg);
	const char *channel = message_parameter(msg, 0);
	const char *buf;

	/* Check if the message was for us. */
	if (message_to_me(conn, msg, &buf) == FALSE)
		return;

	if (strcasecmp(buf, "opme") != 0 &&
	    strcasecmp(buf, "op me") != 0)
		return;

	/* If the userhost is operator, then op him. */
	if (is_operator(message_userhost(msg))) {
		log_plugin(LOG_DEBUG, p,
		    "Giving operator status to '%s' in %s (%s).",
		    sender, channel, connection_id(conn));

		send_raw(conn, "MODE %s +o %s", channel, sender);
	}
}
