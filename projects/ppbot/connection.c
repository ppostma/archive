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
#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compat/compat.h"
#include "ircbot.h"
#include "queue.h"
#include "timer.h"

/*
 * Local macros.
 */
#define TIMER_PING	(1 * 60)
#define TIMER_RECONNECT	(3 * 60)

/*
 * Connection object.
 */
struct connection {
	char			*id;
	char			*address;
	char			*port;
	char			*password;
	char			*ident;
	char			*realname;
	char			*nick;
	char			*altnick;
	char			*curnick;
	char			*server;
	int			 fd;
	int			 active;
	char			 buf[MAX_BUFFER + 1];
	int			 trash;
	int			 ping_wait;
	ChannelList		 channels;
	MqueueList		 queue;
	TAILQ_ENTRY(connection)	 link;
};

/*
 * List with connections.
 */
static TAILQ_HEAD(, connection) connections =
    TAILQ_HEAD_INITIALIZER(connections);

/*
 * Poll file descriptors.  Allocated when connections_pollfds() is called.
 */
static struct pollfd	*pfd = NULL;
static unsigned int	 numfd = 0;
static int		 repoll = TRUE;

/*
 * Local function prototypes.
 */
static void	connections_ping_timer(void);
static void	connections_ping_timer_setup(void);
static void	connections_ping_timer_destroy(void);

static void	reconnect_timer(struct connection *);
static void	reconnect_timer_setup(struct connection *);
static void	reconnect_timer_destroy(struct connection *);

/*
 * connection_create --
 *	Create an instance of Connection.
 */
Connection
connection_create(const char *id)
{
	struct connection *conn;

	conn = xcalloc(1, sizeof(struct connection));
	conn->id = xstrdup(id);
	conn->fd = -1;
	conn->active = FALSE;
	conn->channels = channel_list_create();
	TAILQ_INSERT_TAIL(&connections, conn, link);

	return (conn);
}

/*
 * connection_destroy --
 *	Destroy a Connection instance.
 */
void
connection_destroy(Connection conn)
{
	/* Destroy the reconnect timer if it exists. */
	reconnect_timer_destroy(conn);

	/* Free space for all channels. */
	channel_list_destroy(conn->channels);

	/* Free space used for the connection. */
	xfree(conn->id);
	xfree(conn->address);
	xfree(conn->port);
	xfree(conn->password);
	xfree(conn->ident);
	xfree(conn->realname);
	xfree(conn->nick);
	xfree(conn->altnick);
	xfree(conn->curnick);
	xfree(conn->server);
	xfree(conn);
}

/*
 * connection_start --
 *	Try to make a connection to the IRC server using the information
 *	in the supplied connection.  Return TRUE if the connection was
 *	successful or FALSE if there was an error.
 */
int
connection_start(Connection conn)
{
	struct addrinfo	*res, *ai, hints;
	struct pollfd	 fds[1];
	int		 error, sock, flags, rv, val, saved_errno;
	socklen_t	 optlen = sizeof(optlen);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	error = getaddrinfo(conn->address, conn->port, &hints, &res);
	if (error != 0) {
		log_warnx("[%s] Unable to resolve %s[%s]: %s", conn->id,
		    conn->address, conn->port, gai_strerror(error));
		return (FALSE);
	}
	errno = EADDRNOTAVAIL;
	for (sock = -1, ai = res; ai != NULL; ai = ai->ai_next) {
		sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock == -1)
			continue;
		flags = fcntl(sock, F_GETFL, 0);
		if (flags == -1) {
			connection_safeclose(sock);
			sock = -1;
			continue;
		}
		if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
			connection_safeclose(sock);
			sock = -1;
			continue;
		}
		do {
			rv = connect(sock, ai->ai_addr, ai->ai_addrlen);
		} while (rv == -1 && errno == EINTR);

		if (rv == -1 && errno != EINPROGRESS)
			goto failed;

		do {
			fds[0].fd = sock;
			fds[0].events = POLLOUT;
			rv = poll(fds, 1, 20 * 1000);
		} while (rv == -1 && errno == EINTR);

		if (rv == -1) {
			goto failed;
		} else if (rv == 0) {
			errno = ETIMEDOUT;
			goto failed;
		} else if (getsockopt(sock, SOL_SOCKET, SO_ERROR,
		    (void *)&val, &optlen) == -1 || val != 0) {
			errno = ECONNREFUSED;
			goto failed;
		}
		if (fcntl(sock, F_SETFL, flags) == -1)
			goto failed;
		/* Got a working socket. */
		break;
 failed:
		saved_errno = errno;
		connection_safeclose(sock);
		errno = saved_errno;
		sock = -1;
	}
	freeaddrinfo(res);

	if (sock == -1) {
		log_warn("[%s] Unable to connect to %s[%s]", conn->id,
		    conn->address, conn->port);
		return (FALSE);
	}

	/* Attach the message queue. */
	conn->queue = mq_attach(conn);
	if (conn->queue == NULL) {
		log_warnx("[%s] Unable to attach message queue.", conn->id);
		connection_safeclose(sock);
		return (FALSE);
	}

	/* Set the current nickname to the default one. */
	connection_set_current_nick(conn, conn->nick);

	conn->fd = sock;
	conn->active = TRUE;

	/* Re-initialize the poll fds. */
	numfd++;
	repoll = TRUE;

	return (TRUE);
}

/*
 * connection_read --
 *	Read data from an IRC server and parse the lines.
 */
int
connection_read(Connection conn)
{
	char	*p, *q, *line;
	char	 buf[MAX_BUFFER + 1];
	size_t	 len;
	ssize_t	 n;

	if (*conn->buf != '\0') {
		strlcpy(buf, conn->buf, sizeof(buf));
		len = strlen(buf);
	} else
		len = 0;

	do {
		n = recv(conn->fd, buf + len, sizeof(buf) - len - 1, 0);
	} while (n == -1 && errno == EINTR);

	if (n == -1) {
		log_warn("[%s] recv error", conn->id);
		return (FALSE);
	} else if (n == 0)
		return (FALSE);
	buf[n + len] = '\0';

	line = buf;
	for (p = buf; *p != '\0'; line = p) {
		if ((p = strchr(line, '\n')) == NULL) {
			if (strlen(line) >= MAX_BUFFER) {
				/*
				 * This would be pretty bad and should never
				 * happen on normal (correct) IRC servers.
				 */
				log_debug("[%s] Read buffer is full! "
				    "(message ignored).", conn->id);
				conn->buf[0] = '\0';
				conn->trash = TRUE; /* trash remaining data */
			} else {
				strlcpy(conn->buf, line, sizeof(conn->buf));
			}
			return (TRUE);
		}
		*p++ = '\0';

		if (conn->trash) {
			conn->trash = FALSE;
			continue;
		}

		if ((q = strpbrk(line, "\r\n")) != NULL)
			*q = '\0';

		message_parse(conn, line);
	}
	if (len != 0)
		conn->buf[0] = '\0';

	return (TRUE);
}

/*
 * connection_write --
 *	Write data to an IRC server.
 */
int
connection_write(Connection conn, const char *str)
{
	size_t	total, left;
	ssize_t	rv;

	if (conn->fd == -1) {
		log_debug("[%s] Invalid socket descriptor", conn->id);
		return (FALSE);
	}

	total = 0;
	left = strlen(str);
	do {
		rv = send(conn->fd, str + total, left, 0);
		if (rv == -1) {
			log_warn("[%s] send error", conn->id);
			return (FALSE);
		}
		total += rv;
		left -= rv;
	} while (left > 1);

	return (TRUE);
}

/*
 * connection_writef --
 *	Write formatted data to an IRC server.
 */
int
connection_writef(Connection conn, const char *fmt, ...)
{
	va_list	 ap;
	char	*str;
	int	 rv;

	va_start(ap, fmt);
	str = xvsprintf(fmt, ap);
	va_end(ap);

	rv = connection_write(conn, str);

	xfree(str);

	return (rv);
}

/*
 * connection_lookup --
 *	Look up for a connection by searching for the file descriptor.
 */
Connection
connection_lookup(int fd)
{
	struct connection *conn;

	TAILQ_FOREACH(conn, &connections, link)
		if (conn->fd == fd)
			return (conn);
	return (NULL);
}

/*
 * connection_find --
 *	Find a connection by searching for the ID.
 */
Connection
connection_find(const char *id)
{
	struct connection *conn;

	TAILQ_FOREACH(conn, &connections, link)
		if (strcmp(conn->id, id) == 0)
			return (conn);
	return (NULL);
}

/*
 * connection_close --
 *	Terminate a connection.
 */
void
connection_close(Connection conn, int reconnect)
{
	/* Set all channels to not-joined. */
	channel_set_parted_all(conn->channels);

	/* If the join channels timer is still active, destroy it. */
	connection_join_timer_destroy(conn);

	/* Detach the message queue. */
	mq_detach(conn, conn->queue);

	/* Close the connection. */
	connection_safeclose(conn->fd);

	conn->fd = -1;
	conn->active = FALSE;
	conn->trash = FALSE;
	conn->ping_wait = 0;

	/* Empty the read buffer. */
	memset(&conn->buf, '\0', sizeof(conn->buf));

	/* Re-initialize the poll fds. */
	numfd--;
	repoll = TRUE;

	/* Install reconnect timer if requested. */
	if (reconnect) {
		reconnect_timer_setup(conn);
	}
}

/*
 * connection_safeclose --
 *	Safe version of close().  Makes sure that the descriptor is closed
 *	even when close() gets interrupted by a signal.
 */
void
connection_safeclose(int fd)
{
	int rv;

	do {
		rv = close(fd);
	} while (rv == -1 && errno == EINTR);
}

/*
 * connection_active --
 *	Accessor function for the 'active' member.
 */
int
connection_active(Connection conn)
{
	return (conn->active);
}

/*
 * connection_address --
 *	Accessor function for the 'address' member.
 */
const char *
connection_address(Connection conn)
{
	return (conn->address);
}

/*
 * connection_alternate_nick --
 *	Accessor function for the 'altnick' member.
 */
const char *
connection_alternate_nick(Connection conn)
{
	return (conn->altnick);
}

/*
 * connection_channels --
 *	Accessor function for the 'channels' member.
 */
ChannelList
connection_channels(Connection conn)
{
	return (conn->channels);
}

/*
 * connection_current_nick --
 *	Accessor function for the 'curnick' member.
 */
const char *
connection_current_nick(Connection conn)
{
	return (conn->curnick);
}

/*
 * connection_id --
 *	Accessor function for the 'id' member.
 */
const char *
connection_id(Connection conn)
{
	return (conn->id);
}

/*
 * connection_ident --
 *	Accessor function for the 'ident' member.
 */
const char *
connection_ident(Connection conn)
{
	return (conn->ident);
}

/*
 * connection_nick
 *	Accessor function for the 'nick' member.
 */
const char *
connection_nick(Connection conn)
{
	return (conn->nick);
}

/*
 * connection_password --
 *	Accessor function for the 'password' member.
 */
const char *
connection_password(Connection conn)
{
	return (conn->password);
}

/*
 * connection_port --
 *	Accessor function for the 'port' member.
 */
const char *
connection_port(Connection conn)
{
	return (conn->port);
}

/*
 * connection_queue --
 *	Accessor function for the 'queue' member.
 */
MqueueList
connection_queue(Connection conn)
{
	return (conn->queue);
}

/*
 * connection_realname --
 *	Accessor function for the 'realname' member.
 */
const char *
connection_realname(Connection conn)
{
	return (conn->realname);
}

/*
 * connection_set_address --
 *	Set the address of a connection.
 */
void
connection_set_address(Connection conn, const char *address)
{
	xstrdup2(&conn->address, address);
}

/*
 * connection_set_alternate_nick --
 *	Set the alternate nickname of a connection.
 */
void
connection_set_alternate_nick(Connection conn, const char *altnick)
{
	xstrdup2(&conn->altnick, altnick);
}

/*
 * connection_set_current_nick --
 *	Set the current nickname of a connection.
 */
void
connection_set_current_nick(Connection conn, const char *curnick)
{
	xstrdup2(&conn->curnick, curnick);
}

/*
 * connection_set_ident --
 *	Set the user identification of a connection.
 */
void
connection_set_ident(Connection conn, const char *ident)
{
	xstrdup2(&conn->ident, ident);
}

/*
 * connection_set_nick --
 *	Set the nickname of a connection.
 */
void
connection_set_nick(Connection conn, const char *nick)
{
	xstrdup2(&conn->nick, nick);
}

/*
 * connection_set_password --
 *	Set the password of a connection.
 */
void
connection_set_password(Connection conn, const char *password)
{
	xstrdup2(&conn->password, password);
}

/*
 * connection_set_port --
 *	Set the port of a connection.
 */
void
connection_set_port(Connection conn, const char *port)
{
	xstrdup2(&conn->port, port);
}

/*
 * connection_set_realname --
 *	Set the real name of a connection.
 */
void
connection_set_realname(Connection conn, const char *realname)
{
	xstrdup2(&conn->realname, realname);
}

/*
 * connection_set_server --
 *	Set the server of a connection.
 */
void
connection_set_server(Connection conn, const char *server)
{
	xstrdup2(&conn->server, server);
}

/*
 * connection_got_pong --
 *	Reset the ping wait indicator.
 */
void
connection_got_pong(Connection conn)
{
	conn->ping_wait = 0;
}

/*
 * connection_join_channels --
 *	Join all channels in a connection.
 */
static void
connection_join_channels(Connection conn)
{
	channel_join_all(conn, conn->channels);
}

/*
 * connection_join_timer_setup --
 *	Setup a timer to join all channels in 5 seconds.
 */
void
connection_join_timer_setup(Connection conn)
{
	struct timeval	tv;
	int		rv;

	TV_SET(&tv, 5, 0);

	rv = timer_schedule(tv, (void *)connection_join_channels, conn,
	    TIMER_ONCE, "join_channels#%lx", (unsigned long)conn);
	if (rv != 0) {
		log_debug("[%s] Unable to schedule join channels timer: %s",
		    conn->id, strerror(rv));
	}
}

/*
 * connection_join_timer_destroy --
 *	Remove the join channels timer, if active.
 */
void
connection_join_timer_destroy(Connection conn)
{
	int rv;

	rv = timer_cancel("join_channels#%lx", (unsigned long)conn);
	if (rv != 0 && rv != ENOENT) {
		log_debug("[%s] Unable to cancel join channels timer: %s",
		    conn->id, strerror(rv));
	}
}

/*
 * connections_verify --
 *	Check if non-active connections have a valid configuration.
 *	Invalid connections will be removed from the connections list.
 */
void
connections_verify(void)
{
	struct connection *conn, *n;

	for (conn = TAILQ_FIRST(&connections); conn != NULL; conn = n) {
		n = TAILQ_NEXT(conn, link);
		if (!conn->active && !config_verify(conn)) {
			TAILQ_REMOVE(&connections, conn, link);
			connection_destroy(conn);
		}
	}
}

/*
 * connections_pollfds --
 *	Return an array of pollfd structures that contain all
 *	active connections.
 */
struct pollfd *
connections_pollfds(unsigned int *nfd)
{
	struct connection *conn;
	unsigned int	   n, i = 0;

	if (repoll) {
		repoll = FALSE;

		if ((n = numfd) < 1)
			n = 1;	/* Allocate at least one. */

		pfd = xrealloc(pfd, n * sizeof(struct pollfd));

		TAILQ_FOREACH(conn, &connections, link) {
			if (!conn->active)
				continue;
			pfd[i].fd = conn->fd;
			pfd[i++].events = POLLIN;
		}

		log_debug("Poll init, using %d descriptors.", numfd);
	}

	if (nfd != NULL)
		*nfd = numfd;

	return (pfd);
}

/*
 * connections_init --
 *	Initialize all connections and install the ping timer.
 */
void
connections_init(void)
{
	struct connection *conn;

	/* Install the connection ping timer. */
	connections_ping_timer_setup();

	/* Initialize all connections. */
	TAILQ_FOREACH(conn, &connections, link) {
		if (!connection_start(conn)) {
			reconnect_timer_setup(conn);
			continue;
		}
		send_login(conn);
	}
}

/*
 * connections_reinit --
 *	Initialize all non-active connections.
 */
void
connections_reinit(void)
{
	struct connection *conn;

	TAILQ_FOREACH(conn, &connections, link) {
		if (conn->active)
			continue;
		if (!connection_start(conn)) {
			reconnect_timer_setup(conn);
			continue;
		}
		send_login(conn);
	}
}

/*
 * connections_join_channels --
 *	Join all channels for all active connections.
 */
void
connections_join_channels(void)
{
	struct connection *conn;

	TAILQ_FOREACH(conn, &connections, link) {
		if (!conn->active)
			continue;
		channel_join_all(conn, conn->channels);
	}
}

/*
 * connections_close --
 *	Close all active connections.
 */
void
connections_close(void)
{
	struct connection *conn;

	TAILQ_FOREACH(conn, &connections, link) {
		if (!conn->active)
			continue;
		send_quit(conn, NULL);
		connection_close(conn, FALSE);
	}
}

/*
 * connections_destroy --
 *	Remove all active and non-active connections from the list
 *	and destroy the ping timer.
 */
void
connections_destroy(void)
{
	struct connection *conn;

	/* Destroy all connections. */
	while ((conn = TAILQ_FIRST(&connections)) != NULL) {
		TAILQ_REMOVE(&connections, conn, link);
		connection_destroy(conn);
	}

	/* Destroy the connections ping timer. */
	connections_ping_timer_destroy();
}

/*
 * connections_destroy_dead --
 *	Remove all non-active connections from the list.
 */
void
connections_destroy_dead(void)
{
	struct connection *conn, *n;

	for (conn = TAILQ_FIRST(&connections); conn != NULL; conn = n) {
		n = TAILQ_NEXT(conn, link);
		if (conn->active)
			continue;
		TAILQ_REMOVE(&connections, conn, link);
		connection_destroy(conn);
	}
}

/*
 * connections_ping_timer -
 *	Called when the ping timer finishes.  This function checks if we got
 *	a PONG reply in a reasonable time and sends out PING requests.
 */
static void
connections_ping_timer(void)
{
	struct connection *conn;

	TAILQ_FOREACH(conn, &connections, link) {
		if (!conn->active)
			continue;
		if (conn->ping_wait > 0 &&
		    (conn->ping_wait * TIMER_PING) >= 300) {
			/* If the last PONG was too long ago, disconnect. */
			log_warnx("[%s] No PONG received from %s in 300 seconds"
			    ", disconnecting.", conn->id, conn->address);
			connection_close(conn, TRUE);
		} else {
			if (conn->ping_wait++ > 0) {
				log_debug("[%s] Waiting for ping reply... (%d)",
				    conn->id, conn->ping_wait);
			}
			send_ping(conn, conn->server);
		}
	}
}

/*
 * connections_ping_timer_setup --
 *	Setup a timer for periodically sending PING requests.
 */
static void
connections_ping_timer_setup(void)
{
	struct timeval	tv;
	int		rv;

	TV_SET(&tv, TIMER_PING, 0);

	rv = timer_schedule(tv, (void *)connections_ping_timer, NULL,
	    TIMER_POLL, "connections_ping");
	if (rv != 0) {
		log_debug("Unable to schedule connections ping timer: %s",
		    strerror(rv));
	}
}

/*
 * connections_ping_timer_destroy --
 *	Destroy the timer for periodically sending PING requests.
 */
static void
connections_ping_timer_destroy(void)
{
	/* This is only called when shutting down, so ignore errors. */
	timer_cancel("connections_ping");
}

/*
 * reconnect_timer --
 *	Function that will be executed when the reconnect timer finishes.
 */
static void
reconnect_timer(struct connection *conn)
{
	/* Can happen when someone has reloaded the bot. */
	if (conn->active) {
		reconnect_timer_destroy(conn);
		return;
	}

	/* Default to the first nick when connecting. */
	xstrdup2(&conn->curnick, conn->nick);

	/* Try to connect to the server. */
	if (!connection_start(conn)) {
		/* Try again. */
		return;
	}

	/* Stop the reconnect timer. */
	reconnect_timer_destroy(conn);

	send_login(conn);
}

/*
 * reconnect_timer_setup --
 *	Setup a timer to reconnect.
 */
static void
reconnect_timer_setup(struct connection *conn)
{
	struct timeval	tv;
	int		rv;

	TV_SET(&tv, TIMER_RECONNECT, 0);

	rv = timer_schedule(tv, (void *)reconnect_timer, conn, TIMER_POLL,
	    "reconnect_timer#%lx", (unsigned long)conn);
	if (rv != 0 && rv != EEXIST) {
		log_debug("[%s] Unable to schedule reconnect timer: %s",
		    conn->id, strerror(rv));
	}
}

/*
 * reconnect_timer_destroy --
 *	Remove the reconnect timer, if active.
 */
static void
reconnect_timer_destroy(struct connection *conn)
{
	int rv;

	rv = timer_cancel("reconnect_timer#%lx", (unsigned long)conn);
	if (rv != 0 && rv != ENOENT) {
		log_debug("[%s] Unable to cancel reconnect timer: %s",
		    conn->id, strerror(rv));
	}
}
