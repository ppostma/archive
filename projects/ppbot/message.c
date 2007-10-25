/*
 * Copyright (c) 2004-2007 Peter Postma <peter@pointless.nl>
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

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "compat/compat.h"
#include "ircbot.h"

/*
 * Message object.
 */
struct message {
	char	*command;
	char	*sender;
	char	*userhost;
	char	*data;
	char	*params[MAX_PARAMS];
	char	 raw[MAX_BUFFER + 1];
	size_t	 paramcount;
};

/*
 * Function prototypes for built-in event handlers.
 */
static void	event_command_error(Connection, Message);
static void	event_command_join(Connection, Message);
static void	event_command_kick(Connection, Message);
static void	event_command_kill(Connection, Message);
static void	event_command_mode(Connection, Message);
static void	event_command_nick(Connection, Message);
static void	event_command_notice(Connection, Message);
static void	event_command_part(Connection, Message);
static void	event_command_ping(Connection, Message);
static void	event_command_pong(Connection, Message);
static void	event_command_privmsg(Connection, Message);
static void	event_command_quit(Connection, Message);
static void	event_command_topic(Connection, Message);

static void	event_numeric_names(Connection, Message);
static void	event_numeric_nickname(Connection, Message);
static void	event_numeric_topic(Connection, Message);
static void	event_numeric_unknown(Connection, Message);
static void	event_numeric_welcome(Connection, Message);

/* 
 * Table with built-in event handlers.
 *
 * This table is ordered by events we expect the most to events we expect
 * the least.  This way we reduce the search time for common events.
 */
static const struct {
	const char	*command;
	void		(*callback)(Connection, Message);
	int		 flags;
	size_t		 paramcount;
} messages[] = {
	{ "PRIVMSG", event_command_privmsg,	MSG_USER|MSG_DATA,	1 },
	{    "PONG", event_command_pong,	MSG_SENDER|MSG_DATA,	0 },
	{    "PING", event_command_ping,	MSG_DATA,		0 },
	{    "JOIN", event_command_join,	MSG_USER,		0 },
	{    "PART", event_command_part,	MSG_USER,		1 },
	{    "QUIT", event_command_quit,	MSG_USER,		0 },
	{    "MODE", event_command_mode,	MSG_USER,		1 },
	{   "TOPIC", event_command_topic,	MSG_USER,		1 },
	{  "NOTICE", event_command_notice,	MSG_DATA,		1 },
	{    "NICK", event_command_nick,	MSG_USER|MSG_DATA,	0 },
	{    "KICK", event_command_kick,	MSG_USER,		2 },
	{   "ERROR", event_command_error,	MSG_DATA,		0 },
	{    "KILL", event_command_kill,	MSG_USER|MSG_DATA,	1 },
	{     "433", event_numeric_nickname,	MSG_SENDER|MSG_DATA,	2 },
	{     "432", event_numeric_nickname,	MSG_SENDER|MSG_DATA,	2 },
	{     "421", event_numeric_unknown,	MSG_SENDER|MSG_DATA,	2 },
	{     "353", event_numeric_names,	MSG_SENDER,		3 },
	{     "332", event_numeric_topic,	MSG_SENDER,		2 },
	{     "331", event_numeric_topic,	MSG_SENDER,		2 },
	{     "001", event_numeric_welcome,	MSG_SENDER,		0 },
};

/*
 * message_parse --
 *	Parse an IRC message, handle the built-in functions and
 *	execute the plugins.
 */
void
message_parse(Connection conn, char *buf)
{
	struct message	 msg;
	char		*p, *q;
	size_t		 i;

	/* Make a copy of the raw message. */
	strlcpy(msg.raw, buf, sizeof(msg.raw));

	/* Reset sender, user host, command and data. */
	msg.sender   = NULL;
	msg.userhost = NULL;
	msg.command  = NULL;
	msg.data     = NULL;

	if (*buf == ':') {
		/* Extract the sender. */
		p = ++buf;
		while (*p != '\0' && isspace((unsigned char)*p))
			p++;
		buf = p;
		while (*p != '\0' && !isspace((unsigned char)*p))
			p++;
		if (*p != '\0')
			*p++ = '\0';
		if ((q = strchr(buf, '!')) != NULL)
			*q++ = '\0';
		if (*buf != '\0')
			msg.sender = xstrdup(buf);
		/* Extract the user host. */
		if (q != NULL) {
			buf = q;
			while (*q != '\0' && !isspace((unsigned char)*q))
				q++;
			*q = '\0';
		}
		if (*buf != '\0')
			msg.userhost = xstrdup(buf);
		/* Extract message command. */
		while (*p != '\0' && isspace((unsigned char)*p))
			p++;
		buf = p;
		while (*p != '\0' && !isspace((unsigned char)*p))
			p++;
		if (*p != '\0')
			*p++ = '\0';
		if (*buf != '\0')
			msg.command = xstrdup(buf);
	} else {
		/* Extract message command. */
		p = buf;
		while (*p != '\0' && isspace((unsigned char)*p))
			p++;
		buf = p;
		while (*p != '\0' && !isspace((unsigned char)*p))
			p++;
		if (*p != '\0')
			*p++ = '\0';
		if (*buf != '\0')
			msg.command = xstrdup(buf);
	}
	/* Parse the arguments. */
	while (*p != '\0' && isspace((unsigned char)*p))
		p++;
	for (i = 0; *p != '\0' && *p != ':' && i < MAX_PARAMS; i++) {
		buf = p;
		while (*p != '\0' && !isspace((unsigned char)*p))
			p++;
		while (*p != '\0' && isspace((unsigned char)*p))
			*p++ = '\0';
		msg.params[i] = xstrdup(buf);
	}
	msg.paramcount = i;
	/* Set unused parameters to NULL. */
	for (; i < MAX_PARAMS; i++)
		msg.params[i] = NULL;
	if (msg.paramcount == MAX_PARAMS) {
		/* Ignore any remaining arguments. */
		while (*p != '\0' && *p != ':')
			p++;
	}
	buf = p;
	/* Save the data in the message. */
	if (*buf == ':')
		buf++;
	if (*buf != '\0')
		msg.data = xstrdup(buf);

	if (msg.command != NULL) {
		/* Search and execute a built-in function. */
		for (i = 0; i < sizeof(messages) / sizeof(messages[0]); i++) {
			/* Check for a match. */
			if (strcmp(msg.command, messages[i].command) != 0)
				continue;
			/* Check the parameters. */
			if (!message_check_parameters(conn, &msg,
			    messages[i].flags, messages[i].paramcount))
				continue;
			/* Execute the built-in function. */
			(*messages[i].callback)(conn, &msg);
			/* Stop searching. */
			break;
		}

		/* Handle the plugins. */
		plugins_execute(conn, &msg);
	}

	/* Free memory used for the message. */
	for (i = 0; i < msg.paramcount; i++)
		xfree(msg.params[i]);
	xfree(msg.data);
	xfree(msg.command);
	xfree(msg.userhost);
	xfree(msg.sender);
}

/*
 * message_check_parameters --
 *	Check if all parameters are set for a message.  This is also
 *	used when handling the plugins.
 */
int
message_check_parameters(Connection conn, Message msg, int flags,
    size_t paramcount)
{
	if ((flags & MSG_SENDER) && msg->sender == NULL) {
		log_debug("[%s] Sender required, but not found (%s).",
		    connection_id(conn), msg->command);
		return (FALSE);
	}
	if ((flags & MSG_USER) &&
	    (msg->sender == NULL || msg->userhost == NULL)) {
		log_debug("[%s] User required, but not found (%s).",
		    connection_id(conn), msg->command);
		return (FALSE);
	}
	if ((flags & MSG_DATA) && msg->data == NULL) {
		log_debug("[%s] Data required, but not found (%s).",
		    connection_id(conn), msg->command);
		return (FALSE);
	}
	if (msg->paramcount < paramcount) {
		log_debug("[%s] %lu parameters required, found only %lu (%s).",
		    connection_id(conn), (unsigned long)paramcount,
		    (unsigned long)msg->paramcount, msg->command);
		return (FALSE);
	}

	return (TRUE);
}

/*
 * message_is_private --
 *	Return TRUE if the message is a private message.
 *	This works only for MODE, NOTICE and PRIVMSG messages.
 */
int
message_is_private(Connection conn, Message msg)
{
	const char *nick = connection_current_nick(conn);

	if (strcmp(msg->command, "PRIVMSG") == 0 ||
	    strcmp(msg->command, "NOTICE") == 0 ||
	    strcmp(msg->command, "MODE") == 0) {
		return (strcmp(nick, msg->params[0]) == 0);
	}

	return (FALSE);
}

/*
 * message_is_self --
 *	Return TRUE if the message sender matches ourselves.
 */
int
message_is_self(Connection conn, Message msg)
{
	const char *nick = connection_current_nick(conn);

	return (strcmp(nick, msg->sender) == 0);
}

/*
 * message_to_me --
 *	Checks if the message is directed to the bot.  Optionally sets the
 *	buf pointer to the begin of the text.
 */
int
message_to_me(Connection conn, Message msg, const char **buf)
{
	const char *nick = connection_current_nick(conn);
	const char *p;

	/* We only check public messages. */
	if (message_is_private(conn, msg))
		return (FALSE);

	/* Check if the message was for us. */
	if (strncasecmp(msg->data, nick, strlen(nick)) != 0)
		return (FALSE);
	p = msg->data + strlen(nick);

	if (*p != ':' && *p != ',')
		return (FALSE);
	p++;
	while (*p != '\0' && isspace((unsigned char)*p))
		p++;
	/* We expect more data after the name. */
	if (*p == '\0')
		return (FALSE);
	if (buf != NULL)
		*buf = p;

	return (TRUE);
}

/*
 * event_command_error --
 *	Print an error message when we receive one.
 */
static void
event_command_error(Connection conn, Message msg)
{
	log_warnx("[%s] %s", connection_id(conn), msg->data);
}

/*
 * event_command_join --
 *	Handle the JOIN message.
 */
static void
event_command_join(Connection conn, Message msg)
{
	ChannelList	 list = connection_channels(conn);
	Channel		 chan;
	const char	*ch;

	/* Look up the channel, create it if non-existent. */
	ch = (msg->params[0] != NULL) ? msg->params[0] : msg->data;
	if (ch == NULL) {
		log_debug("[%s] No channel in JOIN message.",
		    connection_id(conn));
		return;
	}
	chan = channel_lookup(list, ch);
	if (chan == NULL)
		chan = channel_create(list, ch, FALSE);

	/* Log the message. */
	channel_log_join(msg, chan);

	/* Track joins from ourself and users. */
	if (message_is_self(conn, msg) == TRUE) {
		channel_set_joined(chan);
	} else {
		user_create(chan, msg->sender);
	}
}

/*
 * event_command_kick --
 *	Handle the KICK message.
 */
static void
event_command_kick(Connection conn, Message msg)
{
	ChannelList	list = connection_channels(conn);
	Channel		chan;

	/* Look up the channel. */
	chan = channel_lookup(list, msg->params[0]);
	if (chan == NULL) {
		log_debug("[%s] No such channel '%s' (event_command_kick)",
		    connection_id(conn), msg->params[0]);
		return;
	}

	/* Log the message. */
	channel_log_kick(msg, chan);

	/* Track kicks for ourself and users. */
	if (strcmp(connection_current_nick(conn), msg->params[1]) == 0) {
		channel_set_parted(list, chan);
	} else {
		user_remove_channel(chan, msg->params[1]);
	}
}

/*
 * event_command_kill --
 *	Handle the KILL message.
 */
static void
event_command_kill(Connection conn, Message msg)
{
	ChannelList list = connection_channels(conn);

	/* Destroy the user list for all channels. */
	user_remove_all(list);
}

/*
 * event_command_mode --
 *	Handle the MODE message.
 */
static void
event_command_mode(Connection conn, Message msg)
{
	ChannelList	list = connection_channels(conn);
	Channel		chan;

	/* We don't handle private messages. */
	if (message_is_private(conn, msg))
		return;

	/* Look up the channel. */
	chan = channel_lookup(list, msg->params[0]);
	if (chan == NULL) {
		log_debug("[%s] No such channel '%s' (event_command_mode)",
		    connection_id(conn), msg->params[0]);
		return;
	}

	/* Log the message. */
	channel_log_mode(msg, chan);
}

/*
 * event_command_nick --
 *	Handle the NICK message.
 */
static void
event_command_nick(Connection conn, Message msg)
{
	ChannelList list = connection_channels(conn);

	/* Log the message. */
	channel_log_nick(msg, list);

	/* Track nick changes from ourself and users. */
	if (message_is_self(conn, msg) == TRUE) {
		connection_set_current_nick(conn, msg->data);
	} else {
		user_change_nick(list, msg->sender, msg->data);
	}
}

/*
 * event_command_notice --
 *	Handle the NOTICE message.
 */
static void
event_command_notice(Connection conn, Message msg)
{
	ChannelList	list = connection_channels(conn);
	Channel		chan;

	/* Ignore "NOTICE AUTH" messages. */
	if (msg->sender == NULL || strcmp(msg->params[0], "AUTH") == 0)
		return;

	/* We don't handle private messages. */
	if (message_is_private(conn, msg))
		return;

	/* Look up the channel. */
	chan = channel_lookup(list, msg->params[0]);
	if (chan == NULL) {
		log_debug("[%s] No such channel '%s' (event_command_notice)",
		    connection_id(conn), msg->params[0]);
		return;
	}

	/* Log the message. */
	channel_log_notice(msg, chan);
}

/*
 * event_command_part --
 *	Handle the PART message.
 */
static void
event_command_part(Connection conn, Message msg)
{
	ChannelList	list = connection_channels(conn);
	Channel		chan;

	/* Look up the channel. */
	chan = channel_lookup(list, msg->params[0]);
	if (chan == NULL) {
		log_debug("[%s] No such channel '%s' (event_command_part)",
		    connection_id(conn), msg->params[0]);
		return;
	}

	/* Log the message. */
	channel_log_part(msg, chan);

	/* Track parts from ourself and users. */
	if (message_is_self(conn, msg) == TRUE) {
		channel_set_parted(list, chan);
	} else {
		user_remove_channel(chan, msg->sender);
	}
}

/*
 * event_command_ping --
 *	Reply with pong when we receive a PING message.
 */
static void
event_command_ping(Connection conn, Message msg)
{
	send_pong(conn, msg->data);
}

/*
 * event_command_pong --
 *	Reset the ping wait indicator when we got a PONG message.
 */
static void
event_command_pong(Connection conn, Message msg)
{
	connection_got_pong(conn);
}

/*
 * event_command_privmsg --
 *	Handle the PRIVMSG message.
 */
static void
event_command_privmsg(Connection conn, Message msg)
{
	ChannelList	 list = connection_channels(conn);
	Channel		 chan;
	char		*data = msg->data;
	char		*buf, *p;
	time_t		 t;
	int		 is_action = FALSE;
	int		 is_ctcp = FALSE;

	/* Check what sort message this is (PRIVMSG or CTCP). */
	if (*data == '\001') {
		is_ctcp = TRUE;
		if (strncmp(++data, "ACTION", 6) == 0) {
			is_action = TRUE;
		}
	}

	/* Check to log when this is not a private message. */
	if (!message_is_private(conn, msg)) {
		/* Look up the channel. */
		chan = channel_lookup(list, msg->params[0]);
		if (chan == NULL) {
			log_debug("[%s] No such channel '%s' "
			    "(event_command_privmsg)", connection_id(conn),
			    msg->params[0]);
			return;
		}

		/* Log normal messages and actions. */
		if (!is_ctcp || is_action) {
			channel_log_privmsg(msg, chan);
		}
	}

	/* Handle CTCP actions if this is a CTCP message. */
	if (is_ctcp) {
		if (strncmp(data, "CLIENTINFO", 10) == 0) {
			/* Client info request. */
			send_ctcpreply(conn, msg->sender, "CLIENTINFO",
			    "CLIENTINFO PING TIME VERSION");
		} else if (strncmp(data, "PING", 4) == 0) {
			/* Ping request. */
			data += 4;
			while (*data != '\0' && isspace((unsigned char)*data))
				data++;
			if (*data == '\0' || *data == '\001')
				return;
			buf = xstrdup(data);
			if ((p = strrchr(buf, '\001')) != NULL)
				*p = '\0';
			send_ctcpreply(conn, msg->sender, "PING", "%s", buf);
			xfree(buf);
		} else if (strncmp(data, "TIME", 4) == 0) {
			/* Time request. */
			t = time(NULL);
			buf = ctime(&t);
			buf[strlen(buf) - 1] = '\0';
			send_ctcpreply(conn, msg->sender, "TIME", "%s", buf);
		} else if (strncmp(data, "VERSION", 7) == 0) {
			/* Version request. */
			send_ctcpreply(conn, msg->sender, "VERSION",
			    "%s v%s", IRCBOT_NAME, IRCBOT_VERSION);
		}
	}
}

/*
 * event_command_quit --
 *	Handle the QUIT message.
 */
static void
event_command_quit(Connection conn, Message msg)
{
	ChannelList list = connection_channels(conn);

	/* Log the message. */
	channel_log_quit(msg, list);

	/* Track quits from ourself and users. */
	if (message_is_self(conn, msg) == TRUE) {
		channel_destroy_all(list);
	} else {
		user_remove(list, msg->sender);
	}
}

/*
 * messages_topic --
 *	Track topic changes for a channel.
 */
static void
event_command_topic(Connection conn, Message msg)
{
	ChannelList	list = connection_channels(conn);
	Channel		chan;

	/* Look up the channel. */
	chan = channel_lookup(list, msg->params[0]);
	if (chan == NULL) {
		log_debug("[%s] No such channel '%s' (event_command_topic)",
		    connection_id(conn), msg->params[0]);
		return;
	}

	/* Log the message. */
	channel_log_topic(msg, chan);

	/* Record the topic change. */
	channel_set_topic(chan, msg->data);
}

/*
 * event_numeric_names --
 *	Handle the name reply numeric message (353).
 */
static void
event_numeric_names(Connection conn, Message msg)
{
	ChannelList	 list = connection_channels(conn);
	Channel		 chan;
	char		*buf, *save, *p;

	/* Look up the channel. */
	chan = channel_lookup(list, msg->params[2]);
	if (chan == NULL) {
		log_debug("[%s] No such channel '%s' (event_numeric_names)",
		    connection_id(conn), msg->params[2]);
		return;
	}

	buf = xstrdup(msg->data);
	save = buf;

	/* Parse the names. */
	for (p = buf; *p != '\0'; buf = p) {
		while (*p != '\0' && !isspace((unsigned char)*p))
			p++;
		while (*p != '\0' && isspace((unsigned char)*p))
			*p++ = '\0';
		if (*buf == '@' || *buf == '+' || *buf == '%')
			buf++;

		user_create(chan, buf);
	}

	xfree(save);
}

/*
 * event_numeric_nickname --
 *	Handle the numeric nickname (432 and 433) messages.
 */
static void
event_numeric_nickname(Connection conn, Message msg)
{
	if (strcmp(msg->command, "433") == 0) {
		log_warnx("[%s] Nickname '%s' is in use.",
		    connection_id(conn), connection_current_nick(conn));
	} else {
		log_warnx("[%s] Erroneous nickname '%s'.",
		    connection_id(conn), connection_current_nick(conn));
	}

	/* Try alternate nick if we've one. */
	if (strcmp(connection_current_nick(conn), msg->params[1]) == 0 &&
	    connection_alternate_nick(conn) != NULL &&
	    strcmp(connection_alternate_nick(conn), msg->params[1]) != 0) {
		log_warnx("[%s] Trying nickname '%s'.", connection_id(conn),
		    connection_alternate_nick(conn));
		send_nick(conn, connection_alternate_nick(conn));

		/* Save the new nickname. */
		connection_set_current_nick(conn,
		    connection_alternate_nick(conn));
	} else {
		log_warnx("[%s] No nicknames available to use.",
		    connection_id(conn));
		send_quit(conn, NULL);
	}
}

/*
 * event_numeric_topic --
 *	Watch topic events for 331 and 332 messages.
 */
static void
event_numeric_topic(Connection conn, Message msg)
{
	ChannelList	 list = connection_channels(conn);
	Channel		 chan;
	char		*topic;

	/* Topic was set or unset? */
	if (strcmp(msg->command, "331") == 0) {
		topic = NULL;
	} else {
		topic = msg->data;
	}

	/* Look up the channel. */
	chan = channel_lookup(list, msg->params[1]);
	if (chan == NULL) {
		log_debug("[%s] No such channel '%s' (event_numeric_topic)",
		    connection_id(conn), msg->params[1]);
		return;
	}

	/* Record the topic change. */
	channel_set_topic(chan, topic);
}

/*
 * event_numeric_unknown --
 *	Print a warning about the unknown command.
 */
static void
event_numeric_unknown(Connection conn, Message msg)
{
	log_warnx("[%s] %s: %s", connection_id(conn), msg->params[1],
	    msg->data);
}

/*
 * event_numeric_welcome --
 *	Welcome message from the IRC server, if we get this then we should be
 *	online.
 */
static void
event_numeric_welcome(Connection conn, Message msg)
{
	/* Schedule a timer to join the channels. */
	connection_join_timer_setup(conn);

	/* Record the real name of the server. */
	connection_set_server(conn, msg->sender);
}

/*
 * message_command --
 *	Accessor function for the 'command' member.
 */
const char *
message_command(Message msg)
{
	return (msg->command);
}

/*
 * message_sender --
 *	Accessor function for the 'sender' member.
 */
const char *
message_sender(Message msg)
{
	return (msg->sender);
}

/*
 * message_userhost --
 *	Accessor function for the 'userhost' member.
 */
const char *
message_userhost(Message msg)
{
	return (msg->userhost);
}

/*
 * message_data --
 *	Accessor function for the 'data' member.
 */
const char *
message_data(Message msg)
{
	return (msg->data);
}

/*
 * message_parameter --
 *	Accessor function for the 'params' member.
 */
const char *
message_parameter(Message msg, size_t idx)
{
	if (idx >= MAX_PARAMS)
		return (NULL);

	return (msg->params[idx]);
}

/*
 * message_raw --
 *	Accessor function for the 'raw' member.
 */
const char *
message_raw(Message msg)
{
	return (msg->raw);
}

/*
 * message_parameter_count --
 *	Accessor function for the 'paramcount' member.
 */
size_t
message_parameter_count(Message msg)
{
	return (msg->paramcount);
}
