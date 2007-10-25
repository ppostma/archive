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

#ifndef _IRCBOT_H_
#define _IRCBOT_H_

#include <stdarg.h>	/* for va_list type */
#include <stdio.h>	/* for FILE type */

/*
 * Name, version & default configuration of the IRC bot.
 */
#define IRCBOT_NAME		"ppbot"
#define IRCBOT_VERSION		"20071007"
#define IRCBOT_CONFIG		"ppbot.conf"

/*
 * Maximum read buffer, 512 is specified by the RFC as maximum length
 * of an IRC message (including the trailing CR-LF).
 */
#define MAX_BUFFER		512

/*
 * Maximum number of parameters in a message, as per RFC.
 */
#define MAX_PARAMS		15

/*
 * Boolean symbolic constants.
 */
#define FALSE			0
#define TRUE			1

/* Message flags. */
#define MSG_SENDER		0x01  /* The message requires a sender. */
#define MSG_USER		0x02  /* The message requires a valid user. */
#define MSG_DATA		0x04  /* The message requires data. */

/* Channel object. */
typedef struct channel *Channel;

/* Channel list object. */
typedef struct channel_head *ChannelList;

/* Connection object. */
typedef struct connection *Connection;

/* Message object. */
typedef struct message *Message;

/* Message queue list object. */
typedef struct mqueue_head *MqueueList;

/* Plugin object. */
typedef struct plugin *Plugin;

/*
 * Function prototypes.
 */

/* channel.c */
ChannelList	 channel_list_create(void);
void		 channel_list_destroy(ChannelList);

Channel		 channel_create(ChannelList, const char *, int);
void		 channel_destroy(Channel);
void		 channel_destroy_all(ChannelList);

Channel		 channel_lookup(ChannelList, const char *);

const char 	*channel_name(Channel);
const char	*channel_key(Channel);
const char	*channel_topic(Channel);

void		 channel_set_joined(Channel);
void		 channel_set_key(Channel, const char *);
void		 channel_set_logfile(Channel, const char *);
void		 channel_set_parted(ChannelList, Channel);
void		 channel_set_parted_all(ChannelList);
void		 channel_set_topic(Channel, const char *);

void		 channel_join_all(Connection, ChannelList);

void		 user_create(Channel, const char *);
int		 user_on_channel(Channel, const char *);
void		 user_change_nick(ChannelList, const char *, const char *);
void		 user_remove(ChannelList, const char *);
void		 user_remove_channel(Channel, const char *);
void		 user_remove_all(ChannelList);

void		 channel_log_join(Message, Channel);
void		 channel_log_kick(Message, Channel);
void		 channel_log_nick(Message, ChannelList);
void		 channel_log_notice(Message, Channel);
void		 channel_log_mode(Message, Channel);
void		 channel_log_part(Message, Channel);
void		 channel_log_privmsg(Message, Channel);
void		 channel_log_quit(Message, ChannelList);
void		 channel_log_topic(Message, Channel);

void		 channel_log_internal_notice(ChannelList, const char *,
			const char *, const char *);
void		 channel_log_internal_privmsg(ChannelList, const char *,
			const char *, const char *);

/* config_scan.l */
void		 config_scan_initialize(FILE *);

/* config_parse.y */
int		 config_parse(const char *);
int		 config_verify(Connection);

/* connection.c */
Connection	 connection_create(const char *);
void		 connection_destroy(Connection);

int		 connection_start(Connection);
int		 connection_read(Connection);
int		 connection_write(Connection, const char *);
int		 connection_writef(Connection, const char *, ...);
Connection	 connection_lookup(int);
Connection	 connection_find(const char *);
void		 connection_close(Connection, int);
void		 connection_safeclose(int);

int		 connection_active(Connection);
const char	*connection_alternate_nick(Connection);
const char	*connection_address(Connection);
ChannelList	 connection_channels(Connection);
const char	*connection_current_nick(Connection);
const char	*connection_id(Connection);
const char	*connection_ident(Connection);
const char	*connection_nick(Connection);
const char	*connection_password(Connection);
const char	*connection_port(Connection);
MqueueList	 connection_queue(Connection);
const char	*connection_realname(Connection);

void		 connection_set_address(Connection, const char *);
void		 connection_set_alternate_nick(Connection, const char *);
void		 connection_set_current_nick(Connection, const char *);
void		 connection_set_ident(Connection, const char *);
void		 connection_set_nick(Connection, const char *);
void		 connection_set_password(Connection, const char *);
void		 connection_set_port(Connection, const char *);
void		 connection_set_realname(Connection, const char *);
void		 connection_set_server(Connection, const char *);

void		 connection_got_pong(Connection);

void		 connection_join_timer_setup(Connection);
void		 connection_join_timer_destroy(Connection);

void		 connections_verify(void);
struct pollfd	*connections_pollfds(unsigned int *);
void		 connections_initialize(void);
void		 connections_reinitialize(void);
void		 connections_join_channels(void);
void		 connections_close(void);
void		 connections_destroy(void);
void		 connections_destroy_dead(void);

/* log.c */
void		 set_logdebug(int);
void		 set_logstderr(int);

void		 logfile_set(const char *);
void		 logfile_open(void);
void		 logfile_close(void);

void		 log_debug(const char *, ...);
void		 log_warnx(const char *, ...);
void		 log_warn(const char *, ...);

/* message.c */
void		 message_parse(Connection, char *);
int		 message_check_parameters(Connection, Message, int, size_t);
int		 message_is_private(Connection, Message);
int		 message_is_self(Connection, Message);
int		 message_to_me(Connection, Message, const char **);

const char	*message_command(Message);
const char	*message_sender(Message);
const char	*message_userhost(Message);
const char	*message_data(Message);
const char	*message_parameter(Message, size_t);
const char	*message_raw(Message);
size_t		 message_parameter_count(Message);

/* mqueue.c */
MqueueList	 mq_attach(Connection);
void		 mq_detach(Connection, MqueueList);

void		 send_ctcpreply(Connection, const char *, const char *,
			const char *, ...);
void		 send_join(Connection, const char *, const char *);
void		 send_login(Connection);
void		 send_nick(Connection, const char *);
void		 send_notice(Connection, const char *, const char *, ...);
void		 send_ping(Connection, const char *);
void		 send_pong(Connection, const char *);
void		 send_privmsg(Connection, const char *, const char *, ...);
void		 send_quit(Connection, const char *);
void		 send_raw(Connection, const char *, ...);

/* plugin.c */
int		 plugin_add(const char *);

void		 plugins_initialize(void);
void		 plugins_finalize(void);
void		 plugins_destroy(void);
void		 plugins_execute(Connection, Message);

int		 callback_register(Plugin, const char *, int, size_t,
			void (*)(Connection, Message));
int		 callback_deregister(Plugin, const char *, int, size_t,
			void (*)(Connection, Message));

/* xalloc.c */
void		*xmalloc(size_t);
void		*xcalloc(size_t, size_t);
void		*xrealloc(void *, size_t);
char		*xstrdup(const char *);
void		 xstrdup2(char **, const char *);
char		*xsprintf(const char *, ...);
char		*xvsprintf(const char *, va_list);
void		 xfree(void *);

#endif /* !_IRCBOT_H_ */
