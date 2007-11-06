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

/*
 * Google "I'm feeling lucky" plugin for ppbot.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "plugin.h"

/*
 * Name & version of the plugin.
 */
#define PLUGIN_NAME	"google"
#define PLUGIN_VERSION	"20071104"

/*
 * Host/port to connect to.
 */
static const char google_address[] = "www.google.nl";
static const char google_port[] = "80";

/*
 * Error message buffer.
 */
static char	 google_errbuf[BUFSIZ];

/*
 * Local function prototypes.
 */
static void	 google_check_message(Plugin, Connection, Message);
static void	 google_search(Plugin, Connection, Message, const char *);

static int	 google_connect(const char *, const char *);
static ssize_t	 google_send_request(int, const char *, const char *);
static char	*google_receive_data(int);
static char	*google_parse_data(char *);
static void	 google_close(int);

/*
 * plugin_open --
 *	Called when the plugin is loaded/opened.
 */
void
plugin_open(Plugin p)
{
	callback_register(p, "PRIVMSG", MSG_USER|MSG_DATA, 1,
	    google_check_message);

	log_plugin(LOG_INFO, p, "Loaded plugin %s v%s.",
	    PLUGIN_NAME, PLUGIN_VERSION);
}

/*
 * google_check_message --
 *	Check if the PRIVMSG contains a google query.
 */
static void
google_check_message(Plugin p, Connection conn, Message msg)
{
	const char *buf;

	/* Check if the message was for us. */
	if (message_to_me(conn, msg, &buf) == FALSE)
		return;

	if (strncmp(buf, "google", 6) != 0)
		return;

	buf += 6;
	if (*buf != '\0' && !isspace((unsigned char)*buf))
		return;

	while (*buf != '\0' && isspace((unsigned char)*buf))
		buf++;

	google_search(p, conn, msg, buf);
}

/*
 * google_search --
 *	Connect to google and start a search.
 */
static void
google_search(Plugin p, Connection conn, Message msg, const char *args)
{
	const char *channel = message_parameter(msg, 0);
	const char *sender = message_sender(msg);
	char	   *url, *nargs, *q, *buf = NULL;
	int	    sock;

	if (*args == '\0') {
		send_privmsg(conn, channel, "%s: google what?", sender);
		return;
	}

	memset(google_errbuf, '\0', sizeof(google_errbuf));

	nargs = xstrdup(args);
	for (q = nargs; *q != '\0'; q++) {
		if (isspace((unsigned char)*q))
			*q = '+';
	}

	sock = google_connect(google_address, google_port);
	if (sock == -1) {
		log_plugin(LOG_INFO, p, "%s", google_errbuf);
		send_privmsg(conn, channel,
		    "%s: oops! error retrieving data.", sender);
		goto out;
	}
	if (google_send_request(sock, google_address, nargs) == -1) {
		log_plugin(LOG_INFO, p, "%s", google_errbuf);
		send_privmsg(conn, channel,
		    "%s: oops! error retrieving data.", sender);
		goto out;
	}
	buf = google_receive_data(sock);
	if (buf == NULL) {
		log_plugin(LOG_INFO, p, "%s", google_errbuf);
		send_privmsg(conn, channel,
		    "%s: oops! error retrieving data.", sender);
		goto out;
	}

	url = google_parse_data(buf);
	if (url == NULL)
		send_privmsg(conn, channel, "%s: nothing found.", sender);
	else
		send_privmsg(conn, channel, "%s: %s", sender, url);

 out:
	if (sock != -1)
		google_close(sock);

	xfree(buf);
	xfree(nargs);
}

/*
 * google_connect --
 *	Connect to a host at a specified port.  Return a file descriptor
 *	on success or -1 on failure.
 */
static int
google_connect(const char *host, const char *nport)
{
	struct addrinfo	*res, *ai, hints;
	struct pollfd	 fds[1];
	int		 error, sock, flags, rv, val, saved_errno;
	socklen_t	 optlen = sizeof(optlen);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((error = getaddrinfo(host, nport, &hints, &res)) != 0) {
		snprintf(google_errbuf, sizeof(google_errbuf),
		    "Unable to resolve %s[%s]: %s", host, nport,
		    gai_strerror(error));
		return (-1);
	}
	errno = EADDRNOTAVAIL;
	for (sock = -1, ai = res; ai != NULL; ai = ai->ai_next) {
		sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock == -1)
			continue;
		flags = fcntl(sock, F_GETFL, 0);
		if (flags == -1) {
			google_close(sock);
			sock = -1;
			continue;
		}
		if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
			google_close(sock);
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
			rv = poll(fds, 1, 10 * 1000);
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
		google_close(sock);
		errno = saved_errno;
		sock = -1;
	}
	freeaddrinfo(res);

	if (sock == -1) {
		snprintf(google_errbuf, sizeof(google_errbuf),
		    "Unable to connect to %s[%s]: %s", host, nport,
		    strerror(errno));
	}

	return (sock);
}

/*
 * google_send_request --
 *	Send a HTTP GET request to a host.  Return the amount
 *	of bytes sent or -1 on failure.
 */
static ssize_t
google_send_request(int fd, const char *host, const char *search)
{
	char	*query;
	ssize_t	 nbytes;

	query = xsprintf(
	    "GET /search?q=%s&btnI=Ik%%20doe%%20een%%20gok HTTP/1.0\r\n"
	    "Host: %s\r\n"
	    "User-Agent: Mozilla/5.0 (compatible; %s %s v%s)\r\n\r\n",
	    search, host, IRCBOT_NAME, PLUGIN_NAME, PLUGIN_VERSION);

	nbytes = send(fd, query, strlen(query), 0);
	xfree(query);

	if (nbytes == -1) {
		snprintf(google_errbuf, sizeof(google_errbuf),
		    "Error sending data: %s", strerror(errno));
	}

	return (nbytes);
}

/*
 * google_receive_data --
 *	Receive all available data on the file descriptor.  Memory will
 *	be allocated when needed and a pointer to the data will be
 *	returned on success or NULL on failure.
 */
static char *
google_receive_data(int fd)
{
	struct pollfd	 fds[1];
	ssize_t		 nbytes;
	size_t		 len = 0;
	size_t		 buflen = BUFSIZ;
	char		*buf;
	int		 rv;

	buf = xmalloc(buflen);
	for (;;) {
		do {
			fds[0].fd = fd;
			fds[0].events = POLLIN;
			rv = poll(fds, 1, 10 * 1000);
		} while (rv == -1 && errno == EINTR);

		if (rv == -1 || rv == 0) {
			if (rv == -1) {
				snprintf(google_errbuf, sizeof(google_errbuf),
				    "Error in poll: %s", strerror(errno));
			} else {
				snprintf(google_errbuf, sizeof(google_errbuf),
				    "Timeout waiting for data.");
			}
			xfree(buf);
			return (NULL);
		}
		nbytes = recv(fd, buf + len, buflen - len - 1, 0);
		if (nbytes == -1) {
			if (errno == EINTR)
				continue;
			snprintf(google_errbuf, sizeof(google_errbuf),
			    "Error receiving data: %s", strerror(errno));
			xfree(buf);
			return (NULL);
		} else if (nbytes == 0)
			break;
		buf[nbytes + len] = '\0';
		len += nbytes;

		if (len >= buflen - 1) {
			if (len > (BUFSIZ * BUFSIZ)) {
				xfree(buf);
				return (NULL);
			}
			while (len >= buflen - 1)
				buflen *= 2;
			buf = xrealloc(buf, buflen);
		}
	}

	return (buf);
}

/*
 * google_parse_data --
 *	Parse the received data.  Returns the URL that was found or NULL
 *	if nothing was found.  Data in 'buf' may get modified.
 */
static char *
google_parse_data(char *buf)
{
	char *line, *p, *url;

	url = NULL;
	for (p = line = buf; *p != '\0'; line = p) {
		while (*p != '\0' && *p != '\n')
			p++;
		if (*p == '\0')
			break;
		*p++ = '\0';

		if (strncmp(line, "Location:", 9) == 0) {
			url = line + 9;
			while (*url != '\0' && isspace((unsigned char)*url))
				url++;
			if ((p = strpbrk(url, "\r\n")) != NULL)
				*p = '\0';
			break;
		}
	}

	return (url);
}

/*
 * google_close --
 *	Close the connection.
 */
static void
google_close(int fd)
{
	int rv;

	do {
		rv = close(fd);
	} while (rv == -1 && errno == EINTR);
}
