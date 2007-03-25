/*
 * Copyright (c) 2005-2007 Peter Postma <peter@pointless.nl>
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

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "compat/compat.h"
#include "ircbot.h"
#include "queue.h"

/*
 * Channel and user objects.
 */
struct channel {
	char			*name;
	char			*key;
	char			*topic;
	char			*logfile;
	FILE			*logfp;
	int			 joined;
	int			 config;
	LIST_HEAD(, user)	 users;
	TAILQ_ENTRY(channel)	 link;
};

struct user {
	char			*nickname;
	LIST_ENTRY(user)	 link;
};

/*
 * Channel list head.  The connection object keeps an opaque pointer to the
 * head of the channel list.
 */
TAILQ_HEAD(channel_head, channel);

/*
 * Local function prototypes.
 */
static struct user	*user_lookup(struct channel *, const char *);
static void		 user_destroy(struct user *);

/*
 * channel_init --
 *	Initialize a list of channels.
 */
ChannelList
channel_init(void)
{
	struct channel_head *list;

	list = xmalloc(sizeof(struct channel_head));
	TAILQ_INIT(list);

	return (list);
}

/*
 * channel_deinit --
 *	De-initialize a list of channels.
 */
void
channel_deinit(ChannelList list)
{
	struct channel *chan;

	/* Remove all channels. */
	while ((chan = TAILQ_FIRST(list)) != NULL) {
		TAILQ_REMOVE(list, chan, link);
		channel_destroy(chan);
	}
	xfree(list);
}

/*
 * channel_create --
 *	Create an instance of Channel.
 */
Channel
channel_create(ChannelList list, const char *name, int config)
{
	struct channel *chan;

	chan = xcalloc(1, sizeof(struct channel));
	chan->name    = xstrdup(name);
	chan->joined  = FALSE;
	chan->config  = config;
	LIST_INIT(&chan->users);
	TAILQ_INSERT_TAIL(list, chan, link);

	return (chan);
}

/*
 * channel_destroy --
 *	Destroy a Channel instance.
 */
void
channel_destroy(Channel chan)
{
	struct user *usr;

	/* Close the log file if open. */
	if (chan->logfp != NULL)
		fclose(chan->logfp);

	/* Destroy all users. */
	while ((usr = LIST_FIRST(&chan->users)) != NULL) {
		LIST_REMOVE(usr, link);
		user_destroy(usr);
	}

	/* Free space used for the channel. */
	xfree(chan->logfile);
	xfree(chan->topic);
	xfree(chan->key);
	xfree(chan->name);
	xfree(chan);
}

/*
 * channel_destroy_all --
 *	Destroy all Channel instances in a ChannelList.
 */
void
channel_destroy_all(ChannelList list)
{
	struct channel *chan;

	while ((chan = TAILQ_FIRST(list)) != NULL) {
		TAILQ_REMOVE(list, chan, link);
		channel_destroy(chan);
	}
}

/*
 * channel_lookup --
 *	Check if a channel exists in the list.  Return NULL if the channel
 *	doesn't exist or a pointer to the channel if it exists.
 */
Channel
channel_lookup(ChannelList list, const char *name)
{
	struct channel *chan;

	TAILQ_FOREACH(chan, list, link)
		if (strcasecmp(chan->name, name) == 0)
			return (chan);
	return (NULL);
}

/*
 * channel_name --
 *	Accessor function for the 'name' member.
 */
const char *
channel_name(Channel chan)
{
	return (chan->name);
}

/*
 * channel_key --
 *	Accessor function for the 'key' member.
 */
const char *
channel_key(Channel chan)
{
	return (chan->key);
}

/*
 * channel_topic --
 *	Accessor function for the 'topic' member.
 */
const char *
channel_topic(Channel chan)
{
	return (chan->topic);
}

/*
 * channel_set_joined --
 *	Set a channel to joined.
 */
void
channel_set_joined(Channel chan)
{
	chan->joined = TRUE;
}

/*
 * channel_set_key --
 *	Set the key on a channel.
 */
void
channel_set_key(Channel chan, const char *key)
{
	xstrdup2(&chan->key, key);
}

/*
 * channel_set_logfile --
 *	Set the log file for a channel.
 */
void
channel_set_logfile(Channel chan, const char *logfile)
{
	xstrdup2(&chan->logfile, logfile);
}

/*
 * channel_set_parted --
 *	Set a channel to parted.
 */
void
channel_set_parted(ChannelList list, Channel chan)
{
	chan->joined = FALSE;

	/* If the channel is not part of the configuration, remove it. */
	if (!chan->config) {
		TAILQ_REMOVE(list, chan, link);
		channel_destroy(chan);
	}
}

/*
 * channel_set_parted_all --
 *	Set all channels to parted.
 */
void
channel_set_parted_all(ChannelList list)
{
	struct channel *chan, *n;

	for (chan = TAILQ_FIRST(list); chan != NULL; chan = n) {
		n = TAILQ_NEXT(chan, link);

		channel_set_parted(list, chan);
	}
}

/*
 * channel_set_topic --
 *	Set the topic on a channel.
 */
void
channel_set_topic(Channel chan, const char *topic)
{
	xstrdup2(&chan->topic, topic);
}

/*
 * channel_join_all --
 *	Join all channels in a connection.
 */
void
channel_join_all(Connection conn, ChannelList list)
{
	struct channel *chan;

	TAILQ_FOREACH(chan, list, link) {
		/* Initialize log file. */
		if (chan->logfile != NULL) {
			if (chan->logfp != NULL)
				fclose(chan->logfp);

			chan->logfp = fopen(chan->logfile, "a");
			if (chan->logfp == NULL) {
				log_warn("Unable to open '%s' for appending",
				    chan->logfile);
			}
		}
		/* Join the channel if not there. */
		if (!chan->joined)
			send_join(conn, chan->name, chan->key);
	}
}

/*
 * user_lookup --
 *	Check if an user is on a channel.  Return NULL if the user is not
 *	on the channel or a pointer to the user if he/she's on the channel.
 */
static struct user *
user_lookup(struct channel *chan, const char *nick)
{
	struct user *usr;

	LIST_FOREACH(usr, &chan->users, link)
		if (strcasecmp(nick, usr->nickname) == 0)
			return (usr);
	return (NULL);
}

/*
 * user_create --
 *	Add a user to a channel.
 */
void
user_create(Channel chan, const char *nick)
{
	struct user *usr;

	usr = xmalloc(sizeof(struct user));
	usr->nickname = xstrdup(nick);
	LIST_INSERT_HEAD(&chan->users, usr, link);
}

/*
 * user_destroy --
 *	Free all allocated space for an user.
 */
static void
user_destroy(struct user *usr)
{
	xfree(usr->nickname);
	xfree(usr);
}

/*
 * user_on_channel --
 *	Check if an user is on a channel.
 */
int
user_on_channel(Channel chan, const char *nick)
{
	return (user_lookup(chan, nick) != NULL);
}

/*
 * user_change_nick --
 *	Change the nickname of an user in all user lists.
 */
void
user_change_nick(ChannelList list, const char *old, const char *new)
{
	struct channel	*chan;
	struct user	*usr;

	TAILQ_FOREACH(chan, list, link) {
		usr = user_lookup(chan, old);
		if (usr != NULL) {
			xstrdup2(&usr->nickname, new);
		}
	}
}

/*
 * user_remove --
 *	Remove one user in all channels.
 */
void
user_remove(ChannelList list, const char *nick)
{
	struct channel	*chan;
	struct user	*usr;

	TAILQ_FOREACH(chan, list, link) {
		usr = user_lookup(chan, nick);
		if (usr != NULL) {
			LIST_REMOVE(usr, link);
			user_destroy(usr);
		}
	}
}

/*
 * user_remove_channel --
 *	Remove one user in a channel.
 */
void
user_remove_channel(Channel chan, const char *nick)
{
	struct user *usr;

	usr = user_lookup(chan, nick);
	if (usr == NULL)
		return;

	LIST_REMOVE(usr, link);
	user_destroy(usr);
}

/*
 * user_remove_all --
 *	Remove all users for all channels in a ChannelList.
 */
void
user_remove_all(ChannelList list)
{
	struct channel	*chan;
	struct user	*usr;

	TAILQ_FOREACH(chan, list, link) {
		while ((usr = LIST_FIRST(&chan->users)) != NULL) {
			LIST_REMOVE(usr, link);
			user_destroy(usr);
		}
	}
}

/*
 * channel_log_write --
 *	Write the log the the file.
 */
static void
channel_log_write(FILE *fp, const char *str)
{
	time_t	   t = time(NULL);
	struct tm *tm = localtime(&t);

	fprintf(fp, "[%02d:%02d] %s\n", tm->tm_hour, tm->tm_min, str);
	fflush(fp);
}

/*
 * channel_do_log --
 *	Figure out where to send the logs to and write the log.
 */
static void
channel_do_log(ChannelList list, Channel chan, const char *sender,
    const char *buf)
{
	if (chan == NULL && list != NULL) {
		/* Global, append to multiple logs. */
		TAILQ_FOREACH(chan, list, link) {
			/* Is logging enabled? */
			if (chan->logfp == NULL)
				continue;
			/* Is the sender on the channel? */
			if (!user_on_channel(chan, sender))
				continue;
			channel_log_write(chan->logfp, buf);
		}
	} else {
		/* One channel log. */
		channel_log_write(chan->logfp, buf);
	}
}

/* Static buffer for the log messages. */
static char log_buf[BUFSIZ];

/*
 * channel_log_join --
 *	Log a JOIN message.
 */
void
channel_log_join(Message msg, Channel chan)
{
	if (chan->logfp == NULL)
		return;

	snprintf(log_buf, sizeof(log_buf), "%s (%s) joined %s.",
	    message_sender(msg), message_userhost(msg), chan->name);

	channel_do_log(NULL, chan, message_sender(msg), log_buf);
} 

/*
 * channel_log_kick --
 *	Log a KICK message.
 */
void
channel_log_kick(Message msg, Channel chan)
{
	if (chan->logfp == NULL)
		return;

	snprintf(log_buf, sizeof(log_buf), "%s kicked from %s by %s: %s",
	    message_parameter(msg, 1), message_parameter(msg, 0),
	    message_sender(msg), message_data(msg));

	channel_do_log(NULL, chan, message_sender(msg), log_buf);
}

/*
 * channel_log_nick --
 *	Log a NICK message.
 */
void
channel_log_nick(Message msg, ChannelList list)
{
	snprintf(log_buf, sizeof(log_buf), "Nick change: %s -> %s",
	    message_sender(msg), message_data(msg));

	/* Use the old nickname, otherwise the bot won't find it. */
	channel_do_log(list, NULL, message_sender(msg), log_buf);
}

/*
 * channel_log_notice --
 *	Log a NOTICE message.
 */
void
channel_log_notice(Message msg, Channel chan)
{
	channel_log_privmsg(msg, chan);
}

/*
 * channel_log_mode --
 *	Log a MODE message.
 */
void
channel_log_mode(Message msg, Channel chan)
{
	size_t i;

	if (chan->logfp == NULL)
		return;

	snprintf(log_buf, sizeof(log_buf), "%s: mode change '",
	    message_parameter(msg, 0));

	if (message_parameter_count(msg) > 1)
		strlcat(log_buf, message_parameter(msg, 1), sizeof(log_buf));
	for (i = 2; i < message_parameter_count(msg); i++) {
		strlcat(log_buf, " ", sizeof(log_buf));
		strlcat(log_buf, message_parameter(msg, i), sizeof(log_buf));
	}

	snprintf(log_buf + strlen(log_buf), sizeof(log_buf) - strlen(log_buf),
	    "' by %s!%s", message_sender(msg), message_userhost(msg));

	channel_do_log(NULL, chan, message_sender(msg), log_buf);
}

/*
 * channel_log_part --
 *	Log a PART message.
 */
void
channel_log_part(Message msg, Channel chan)
{
	if (chan->logfp == NULL)
		return;

	snprintf(log_buf, sizeof(log_buf), "%s (%s) left %s.",
	    message_sender(msg), message_userhost(msg),
	    message_parameter(msg, 0));

	channel_do_log(NULL, chan, message_sender(msg), log_buf);
}

/*
 * channel_log_privmsg --
 *	Log a PRIVMSG message.
 */
void
channel_log_privmsg(Message msg, Channel chan)
{
	char *tmp, *p;

	if (chan->logfp == NULL)
		return;

	if (strncmp(message_data(msg), "\001ACTION", 7) == 0) {
		tmp = xstrdup(message_data(msg) + 8);
		if ((p = strrchr(tmp, '\001')) != NULL)
			*p = '\0';

		snprintf(log_buf, sizeof(log_buf), "Action: %s %s",
		    message_sender(msg), tmp);
		xfree(tmp);
	} else {
		snprintf(log_buf, sizeof(log_buf), "<%s> %s",
		    message_sender(msg), message_data(msg));
	}

	channel_do_log(NULL, chan, message_sender(msg), log_buf);
}

/*
 * channel_log_quit --
 *	Log a QUIT message.
 */
void
channel_log_quit(Message msg, ChannelList list)
{
	snprintf(log_buf, sizeof(log_buf), "%s (%s) left irc: %s",
	    message_sender(msg), message_userhost(msg),
	    message_data(msg) ? message_data(msg) : "");

	channel_do_log(list, NULL, message_sender(msg), log_buf);
}

/*
 * channel_log_topic --
 *	Log a TOPIC message.
 */
void
channel_log_topic(Message msg, Channel chan)
{
	if (chan->logfp == NULL)
		return;

	snprintf(log_buf, sizeof(log_buf), "Topic changed on %s by %s!%s: %s",
	    message_parameter(msg, 0), message_sender(msg),
	    message_userhost(msg), message_data(msg) ? message_data(msg) : "");

	channel_do_log(NULL, chan, message_sender(msg), log_buf);
}

/*
 * channel_log_internal_notice --
 *	Log an internal NOTICE message.  NOTICE messages generated by
 *	the bot aren't repeated by the IRC server, like most other messages.
 */
void
channel_log_internal_notice(ChannelList list, const char *dest,
    const char *data, const char *curnick)
{
	channel_log_internal_privmsg(list, dest, data, curnick);
}

/*
 * channel_log_internal_privmsg --
 *	Log an internal PRIVMSG message.  PRIVMSG messages generated by
 *	the bot aren't repeated by the IRC server, like most other messages.
 */
void
channel_log_internal_privmsg(ChannelList list, const char *dest,
    const char *data, const char *curnick)
{
	Channel	 chan;
	char	*tmp, *p;

	/* Skip private messages. */
	if (strcmp(curnick, dest) == 0)
		return;

	/* Look up the channel. */
	chan = channel_lookup(list, dest);
	if (chan == NULL || chan->logfp == NULL)
		return;

	/* Log the action or normal message. */
	if (strncmp(data, "\001ACTION", 7) == 0) {
		tmp = xstrdup(data + 8);
		if ((p = strrchr(tmp, '\001')) != NULL)
			*p = '\0';

		snprintf(log_buf, sizeof(log_buf), "Action: %s %s",
		    curnick, tmp);
		xfree(tmp);
	} else {
		snprintf(log_buf, sizeof(log_buf), "<%s> %s", curnick, data);
	}

	channel_do_log(NULL, chan, curnick, log_buf);
}
