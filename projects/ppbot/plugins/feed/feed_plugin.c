/*
 * Copyright (c) 2007 Peter Postma <peter@pointless.nl>
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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "compat/compat.h"
#include "feed.h"
#include "plugin.h"
#include "queue.h"
#include "timer.h"

/*
 * Feed configuration file.
 */
static const char feed_config[] = "feed.conf";

/*
 * Maintain a list with feeds.
 */
struct feed_head {
	FeedConfig		fcp;
	FeedUpdater		fup;
	TAILQ_ENTRY(feed_head)	link;
};

static TAILQ_HEAD(, feed_head) feeds = TAILQ_HEAD_INITIALIZER(feeds);

/*
 * Local function prototypes (interface).
 */
static void feed_config_event(FeedConfig);
static void feed_updater_new_item(FeedConfig, FeedItem);

/*
 * feed_config_event --
 *	Executed when a configuration item is found.
 */
static void
feed_config_event(FeedConfig fcp)
{
	struct feed_head *fhp;

	fhp = xmalloc(sizeof(struct feed_head));
	fhp->fup = feed_updater_create(fcp);
	fhp->fcp = fcp;

	feed_updater_set_new_item(fhp->fup, feed_updater_new_item);

	TAILQ_INSERT_TAIL(&feeds, fhp, link);
}

/*
 * feed_updater_new_item --
 *	Executed when a new item in a feed is found.
 */
static void
feed_updater_new_item(FeedConfig fcp, FeedItem ip)
{
	FeedDestination fdp;

	for (fdp = feed_destination_first(fcp); fdp != NULL;
	     fdp = feed_destination_next(fdp)) {
		Connection conn = connection_find(feed_destination_id(fdp));
		if (conn == NULL) {
			log_debug(
			    "Connection %s not found; no feed output to %s.",
			    feed_destination_id(fdp),
			    feed_destination_channel(fdp));
			continue;
		}

		send_privmsg(conn, feed_destination_channel(fdp),
		    "%s) %s - %s", feed_config_name(fcp), feed_item_title(ip),
		    feed_item_link(ip));
	}
}

/*
 * feed_command --
 *	Handle the feed command.
 */
static void
feed_command(Connection conn, Message msg, const char *args)
{
	const char	 *id = connection_id(conn);
	const char	 *channel = message_parameter(msg, 0);
	const char	 *sender = message_sender(msg);
	struct feed_head *fhp;
	Feed		  fp;
	FeedItem	  ip;
	char		  buf[BUFSIZ];
	size_t		  count;

	/* Show usage if there are no arguments, otherwise show headlines. */
	if (strlen(args) == 0) {
		/* Print the available feeds. */
		snprintf(buf, sizeof(buf), "%s: available feeds: ", sender);

		count = 0;
		TAILQ_FOREACH(fhp, &feeds, link) {
			if (count++ > 0) {
				strlcat(buf, ", ", sizeof(buf));
			}
			strlcat(buf, feed_config_id(fhp->fcp), sizeof(buf));
		}

		send_privmsg(conn, channel, buf);

		/* Print all feeds that are announced in the current channel. */
		snprintf(buf, sizeof(buf),
		    "%s: feeds announced in this channel: ", sender);

		count = 0;
		TAILQ_FOREACH(fhp, &feeds, link) {
			FeedConfig fcp = fhp->fcp;
			FeedDestination fdp;

			for (fdp = feed_destination_first(fcp); fdp != NULL;
			     fdp = feed_destination_next(fdp)) {
				if (strcmp(id,
				    feed_destination_id(fdp)) != 0)
					continue;
				if (strcmp(channel,
				    feed_destination_channel(fdp)) != 0)
					continue;
				if (count++ > 0)
					strlcat(buf, ", ", sizeof(buf));
				strlcat(buf, feed_config_id(fcp), sizeof(buf));
			}
		}

		send_privmsg(conn, channel, buf);
	} else {
		/* Search for the requested feed ID. */
		TAILQ_FOREACH(fhp, &feeds, link) {
			if (strcasecmp(args, feed_config_id(fhp->fcp)) == 0)
				break;
		}
		if (fhp == NULL) {
			send_privmsg(conn, channel, "%s: no such feed.",
			    sender);
			return;
		}

		/* Fetch the feed. */
		fp = feed_fetch(fhp->fup);
		if (fp == NULL) {
			send_privmsg(conn, channel,
			    "%s: something went wrong when updating the feed.",
			    sender);
			return;
		}

		count = 0;
		/* Output the items in the feed. */
		for (ip = feed_item_first(fp); ip != NULL;
		     ip = feed_item_next(ip)) {
			if (++count > feed_config_showcount(fhp->fcp))
				break;

			send_privmsg(conn, channel, "%s) %s - %s",
			    feed_config_name(fhp->fcp), feed_item_title(ip),
			    feed_item_link(ip));
		}
	}
}

/*
 * feed_check_message --
 *	Handle the PRIVMSG message.
 */
static void
feed_check_message(Connection conn, Message msg)
{
	const char *command = "feed";
	const char *buf;

	/* Check if the message was for us. */
	if (message_to_me(conn, msg, &buf) == FALSE)
		return;

	if (strncmp(buf, command, strlen(command)) != 0)
		return;

	buf += strlen(command);
	if (*buf != '\0' && !isspace((unsigned char)*buf))
		return;

	while (*buf != '\0' && isspace((unsigned char)*buf))
		buf++;

	feed_command(conn, msg, buf);
}

/*
 * plugin_open --
 *	Called when the plugin is loaded/opened.  This function loads the
 *	configuration file and installs the timers for reading the RSS feed.
 */
void
plugin_open(Plugin p)
{
	struct feed_head *fhp;
	struct timeval    tv;

	feed_config_set_listener(feed_config_event);
	feed_config_parse(feed_config);

	/* Schedule a timer for each feed and run an initial update. */
	TAILQ_FOREACH(fhp, &feeds, link) {
		FeedUpdater fup = fhp->fup;
		FeedConfig fcp = fhp->fcp;

		if (feed_config_update(fcp)) {
			TV_SET(&tv, 60 * feed_config_refresh(fcp), 0);
			timer_schedule(tv, (void *)feed_update, fup, TIMER_POLL,
			    "feed_updater#%lx", (unsigned long)fup);

			feed_update(fup);
		}
	}

	/* Install the commands. */
	callback_register(p, "PRIVMSG", MSG_USER|MSG_DATA, 1,
	    feed_check_message);
}

/*
 * plugin_close --
 *	Called when the plugin is unloaded/closed.
 */
void
plugin_close(Plugin p)
{
	struct feed_head *fhp;

	/* Cancel the timer for each feed. */
	TAILQ_FOREACH(fhp, &feeds, link) {
		FeedUpdater fup = fhp->fup;
		FeedConfig fcp = fhp->fcp;

		if (feed_config_update(fcp)) {
			timer_cancel("feed_updater#%lx", (unsigned long)fup);
		}
	}

	/* Destroy all feeds. */
	while ((fhp = TAILQ_FIRST(&feeds)) != NULL) {
		TAILQ_REMOVE(&feeds, fhp, link);
		feed_updater_destroy(fhp->fup);
		feed_config_destroy(fhp->fcp);
		xfree(fhp);
	}

	/*
	 * Free memory allocated by yacc/lex/libxml2.
	 * This is needed because we unload the shared object and then load
	 * it again.  Allocated memory isn't freed upon unload and will not
	 * be used ever again so if we don't free it, it will leak.
	 */
	feed_config_scan_cleanup();
	feed_config_parse_cleanup();
	feed_parser_cleanup();
}
