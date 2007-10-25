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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ircbot.h"
#include "queue.h"
#include "timer.h"

/*
 * Message queue object.
 */
struct mqueue {
	char			*msg;
	SIMPLEQ_ENTRY(mqueue)	 link;
};

/*
 * Message queue list head.
 */
SIMPLEQ_HEAD(mqueue_head, mqueue);

/*
 * Local function prototypes.
 */
static struct mqueue	*mq_create(const char *, ...);
static void		 mq_destroy(struct mqueue *);
static void		 mq_suspend_queue(Connection);
static void		 mq_resume_queue(Connection);
static void		 mq_enqueue(Connection, struct mqueue *);
static struct mqueue	*mq_dequeue(Connection);
static void		 mq_handler(Connection);

/*
 * mq_attach --
 *	Attach a message queue.
 */
MqueueList
mq_attach(Connection conn)
{
	struct mqueue_head *list;
	struct timeval	    tv;
	int		    rv;

	/* Poll every 500 milliseconds. */
	TV_SET(&tv, 0, 500 * 1000);

	rv = timer_schedule(tv, (void *)mq_handler, conn, TIMER_POLL,
	    "mq_handler#%lx", (unsigned long)conn);
	if (rv != 0) {
		log_warnx("[%s] Unable to schedule message queue timer: %s",
		    connection_id(conn), strerror(rv));
		return (NULL);
	}

	/* Suspend the timer immediately. */
	rv = timer_suspend("mq_handler#%lx", (unsigned long)conn);
	if (rv != 0) {
		log_warnx("[%s] Unable to suspend message queue timer: %s",
		    connection_id(conn), strerror(rv));
		return (NULL);
	}

	/* Initialize the message queue list. */
	list = xmalloc(sizeof(struct mqueue_head));
	SIMPLEQ_INIT(list);

	return (list);
}

/*
 * mq_detach --
 *	Detach a message queue.
 */
void
mq_detach(Connection conn, MqueueList list)
{
	struct mqueue	*mq;
	int		 rv;

	/* Nuke all messages. */
	while ((mq = SIMPLEQ_FIRST(list)) != NULL) {
		SIMPLEQ_REMOVE_HEAD(list, link);
		mq_destroy(mq);
	}
	xfree(list);

	/* Remove the timer. */
	rv = timer_cancel("mq_handler#%lx", (unsigned long)conn);
	if (rv != 0) {
		log_warnx("[%s] Unable to cancel message queue timer: %s",
		    connection_id(conn), strerror(rv));
	}
}

/*
 * mq_create --
 *	Create a message queue object.
 */
static struct mqueue *
mq_create(const char *fmt, ...)
{
	struct mqueue	*mq;
	char		*msg;
	va_list		 ap;

	va_start(ap, fmt);
	msg = xvsprintf(fmt, ap);
	va_end(ap);

	mq = xmalloc(sizeof(struct mqueue));
	mq->msg = msg;

	return (mq);
}

/*
 * mq_destroy --
 *	Destroy a message queue object.
 */
static void
mq_destroy(struct mqueue *mq)
{
	xfree(mq->msg);
	xfree(mq);
}

/*
 * mq_suspend_queue --
 *	Suspend the message queue.
 */
static void
mq_suspend_queue(Connection conn)
{
	int rv;

	rv = timer_suspend("mq_handler#%lx", (unsigned long)conn);
	if (rv != 0) {
		log_warnx("[%s] Unable to suspend message queue timer: %s",
		    connection_id(conn), strerror(rv));
	}
}

/*
 * mq_resume_queue --
 *	Resume the message queue.
 */
static void
mq_resume_queue(Connection conn)
{
	int rv;

	rv = timer_resume("mq_handler#%lx", (unsigned long)conn);
	if (rv != 0) {
		log_warnx("[%s] Unable to resume message queue timer: %s",
		    connection_id(conn), strerror(rv));
	}
}

/*
 * mq_enqueue --
 *	Enqueue a message.
 */
static void
mq_enqueue(Connection conn, struct mqueue *mq)
{
	struct mqueue_head *list = connection_queue(conn);

	/* Is the list attached? */
	if (list == NULL)
		return;

	/* If the queue is empty, resume the queue timer. */
	if (SIMPLEQ_EMPTY(list))
		mq_resume_queue(conn);

	SIMPLEQ_INSERT_TAIL(list, mq, link);
}

/*
 * mq_dequeue --
 *	Dequeue a message.
 */
static struct mqueue *
mq_dequeue(Connection conn)
{
	struct mqueue_head *list = connection_queue(conn);
	struct mqueue	   *mq;

	/* Is the list attached? */
	if (list == NULL)
		return (NULL);

	mq = SIMPLEQ_FIRST(list);
	if (mq == NULL)
		return (NULL);

	SIMPLEQ_REMOVE_HEAD(list, link);

	/* If the queue is empty, suspend the queue timer. */
	if (SIMPLEQ_EMPTY(list))
		mq_suspend_queue(conn);

	return (mq);
}

/*
 * mq_handler --
 *	Take a message from the queue and send it to a server.
 */
static void
mq_handler(Connection conn)
{
	struct mqueue *mq = mq_dequeue(conn);

	if (mq == NULL)
		return;

	/* Send the data. */
	connection_write(conn, mq->msg);

	/* Free the queued message. */
	mq_destroy(mq);
}

/*
 * send_ctcp --
 *	Send a CTCP message to an IRC server.
 */
void
send_ctcp(Connection conn, const char *dest, const char *cmd,
    const char *fmt, ...)
{
	struct mqueue	*mq;
	va_list		 ap;
	char		*str;

	va_start(ap, fmt);
	str = xvsprintf(fmt, ap);
	va_end(ap);

	mq = mq_create("PRIVMSG %s :\001%s %s\001\r\n", dest, cmd, str);
	mq_enqueue(conn, mq);

	xfree(str);
}

/*
 * send_ctcpreply --
 *	Send a CTCP reply to an IRC server (skips the queue).
 */
void
send_ctcpreply(Connection conn, const char *dest, const char *cmd,
    const char *fmt, ...)
{
	va_list	 ap;
	char	*str;

	va_start(ap, fmt);
	str = xvsprintf(fmt, ap);
	va_end(ap);

	connection_writef(conn, "NOTICE %s :\001%s %s\001\r\n",
	    dest, cmd, str);

	xfree(str);
}

/*
 * send_join --
 *	Send a JOIN message to an IRC server.
 */
void
send_join(Connection conn, const char *channel, const char *key)
{
	struct mqueue *mq;

	if (key == NULL)
		mq = mq_create("JOIN %s\r\n", channel);
	else
		mq = mq_create("JOIN %s %s\r\n", channel, key);

	mq_enqueue(conn, mq);
}

/*
 * send_login --
 *	Send the login messages (PASS/NICK/USER) to an IRC server
 *	(skips the queue).
 */
void
send_login(Connection conn)
{
	if (connection_password(conn) != NULL) {
		connection_writef(conn, "PASS %s\r\n",
		    connection_password(conn));
	}

	connection_writef(conn, "NICK %s\r\n", connection_nick(conn));
	connection_writef(conn, "USER %s 0 * :%s\r\n",
	    connection_ident(conn), connection_realname(conn));
}

/*
 * send_nick --
 *	Send a NICK message to an IRC server.
 */
void
send_nick(Connection conn, const char *nick)
{
	struct mqueue *mq;

	mq = mq_create("NICK %s\r\n", nick);
	mq_enqueue(conn, mq);
}

/*
 * send_notice --
 *	Send a NOTICE message to an IRC server.
 */
void
send_notice(Connection conn, const char *dest, const char *fmt, ...)
{
	struct mqueue	*mq;
	va_list		 ap;
	char		*str;

	va_start(ap, fmt);
	str = xvsprintf(fmt, ap);
	va_end(ap);

	/* Log the message. */
	channel_log_internal_notice(connection_channels(conn),
	    dest, str, connection_current_nick(conn));

	mq = mq_create("NOTICE %s :%s\r\n", dest, str);
	mq_enqueue(conn, mq);

	xfree(str);
}

/*
 * send_ping --
 *	Send a PING message to an IRC server.
 */
void
send_ping(Connection conn, const char *data)
{
	struct mqueue *mq;

	mq = mq_create("PING %s\r\n", data);
	mq_enqueue(conn, mq);
}

/*
 * send_pong --
 *	Send a PONG message to an IRC server (skips the queue).
 */
void
send_pong(Connection conn, const char *data)
{
	connection_writef(conn, "PONG %s\r\n", data);
}

/*
 * send_privmsg --
 *	Send a PRIVMSG message to an IRC server.
 */
void
send_privmsg(Connection conn, const char *dest, const char *fmt, ...)
{
	struct mqueue   *mq;
	va_list		 ap;
	char		*str;

	va_start(ap, fmt);
	str = xvsprintf(fmt, ap);
	va_end(ap);

	/* Log the message. */
	channel_log_internal_privmsg(connection_channels(conn),
	    dest, str, connection_current_nick(conn));

	mq = mq_create("PRIVMSG %s :%s\r\n", dest, str);
	mq_enqueue(conn, mq);

	xfree(str);
}

/*
 * send_quit --
 *	Send a QUIT message to an IRC server (skips the queue).
 */
void
send_quit(Connection conn, const char *msg)
{
	if (msg == NULL)
		connection_write(conn, "QUIT\r\n");
	else
		connection_writef(conn, "QUIT %s\r\n", msg);
}

/*
 * send_raw --
 *	Send raw data to an IRC server.
 */
void
send_raw(Connection conn, const char *fmt, ...)
{
	struct mqueue	*mq;
	va_list		 ap;
	char		*str;

	va_start(ap, fmt);
	str = xvsprintf(fmt, ap);
	va_end(ap);

	mq = mq_create("%s\r\n", str);
	mq_enqueue(conn, mq);

	xfree(str);
}
