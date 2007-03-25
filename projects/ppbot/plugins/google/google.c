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

static const char address[] = "www.google.nl";
static const char port[] = "80";

static void	 check_message(Connection, Message);
static void	 google_search(Connection, Message, const char *);

static int	 host_connect(const char *, const char *);
static ssize_t	 send_http_request(int, const char *, const char *);
static char	*receive_data(int);
static char	*parse_http_data(char *);

void
plugin_open(Plugin p)
{
	callback_register(p, "PRIVMSG", MSG_USER|MSG_DATA, 1, check_message);
}

static void
check_message(Connection conn, Message msg)
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

	google_search(conn, msg, buf);
}

static void
google_search(Connection conn, Message msg, const char *args)
{
	const char *channel = message_parameter(msg, 0);
	const char *sender = message_sender(msg);
	char	   *url, *nargs, *p, *buf = NULL;
	int	    sock;

	if (*args == '\0') {
		send_privmsg(conn, channel, "%s: google what?", sender);
		return;
	}

	nargs = xstrdup(args);
	for (p = nargs; *p != '\0'; p++) {
		if (isspace((unsigned char)*p))
			*p = '+';
	}

	sock = host_connect(address, port);
	if (sock == -1)
		goto out;
	if (send_http_request(sock, address, nargs) == -1)
		goto out;
	buf = receive_data(sock);
	if (buf == NULL)
		goto out;

	url = parse_http_data(buf);
	if (url == NULL)
		send_privmsg(conn, channel, "%s: nothing found.", sender);
	else
		send_privmsg(conn, channel, "%s: %s", sender, url);

 out:
	if (sock != -1)
		connection_safeclose(sock);

	xfree(buf);
	xfree(nargs);
}

/*
 * host_connect --
 *	Connect to a host at a specified port.  Return a file descriptor
 *	on success or -1 on failure.
 */
static int
host_connect(const char *host, const char *nport)
{
	struct addrinfo	*res, *ai, hints;
	struct pollfd	 fds[1];
	int		 error, sock, flags, rv, val, saved_errno;
	socklen_t	 optlen = sizeof(optlen);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((error = getaddrinfo(host, nport, &hints, &res)) != 0) {
		log_warnx("Unable to resolve %s[%s]: %s", host, nport,
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
		connection_safeclose(sock);
		errno = saved_errno;
		sock = -1;
	}
	freeaddrinfo(res);

	if (sock == -1)
		log_warn("Unable to connect to %s[%s]", host, nport);

	return (sock);
}

/*
 * send_http_request --
 *	Send a HTTP GET request to a host.  Return the amount
 *	of bytes sent or -1 on failure.
 */
static ssize_t
send_http_request(int fd, const char *host, const char *search)
{
	char	*query = NULL;
	ssize_t	 nbytes;

	query = xsprintf(
	    "GET /search?q=%s&btnI=Ik%%20doe%%20een%%20gok HTTP/1.0\r\n"
	    "Host: %s\r\n"
	    "User-Agent: Mozilla/5.0 (compatible; )\r\n\r\n", search, host);

	nbytes = send(fd, query, strlen(query), 0);
	xfree(query);

	if (nbytes == -1)
		log_warn("send error");

	return (nbytes);
}

/*
 * receive_data --
 *	Receive all available data on the file descriptor.  Memory will
 *	be allocated when needed and a pointer to the data will be
 *	returned on success or NULL on failure.
 */
static char *
receive_data(int fd)
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
			if (rv == -1)
				log_warn("poll error");
			else
				log_warnx("Timeout waiting for data.");
			xfree(buf);
			return (NULL);
		}
		nbytes = recv(fd, buf + len, buflen - len - 1, 0);
		if (nbytes == -1) {
			if (errno == EINTR)
				continue;
			log_warn("recv error");
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
 * parse_http_data --
 *	Parse the received data.  Returns the URL that was found or NULL
 *	if nothing was found.
 */
char *
parse_http_data(char *buf)
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
