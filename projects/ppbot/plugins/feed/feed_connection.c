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

#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "feed.h"
#include "ircbot.h"

#define TIMEOUT	20

/*
 * Static buffer for error messages.
 */
static char feed_connection_errbuf[BUFSIZ];

/*
 * Some OS have sucky messages from gai_strerror, so we provide our own.
 */
static const struct {
	int		 ecode;
	const char	*msg;
} gai_error_messages[] = {
#ifdef EAI_ADDRFAMILY
	{ EAI_ADDRFAMILY, "Address family for hostname not supported"  },
#endif
	{ EAI_AGAIN,	  "Temporary failure in name resolution"       },
	{ EAI_BADFLAGS,   "The address flags has an invalid value"     },
	{ EAI_FAIL,       "Non-recoverable failure in name resolution" },
	{ EAI_FAMILY,     "Address family not supported"               },
	{ EAI_MEMORY,     "Memory allocation failure"                  },
#ifdef EAI_NODATA
	{ EAI_NODATA,     "No address associated with hostname"        },
#endif
	{ EAI_NONAME,     "Hostname nor service provided, or unknown"  },
#ifdef EAI_OVERFLOW
	{ EAI_OVERFLOW,   "Argument buffer overflow"                   },
#endif
	{ EAI_SERVICE,    "Unknown service name"                       },
	{ EAI_SOCKTYPE,   "Unsupported socket type",                   },
	{ EAI_SYSTEM,     "A system error occurred"                    }
};

/*
 * getaddrinfo_error --
 *	Return user-friendly error message for getaddrinfo() failure.
 */
static const char *
getaddrinfo_error(int ecode)
{
	size_t n = sizeof(gai_error_messages) / sizeof(gai_error_messages[0]);
	size_t i;

	for (i = 0; i < n; i++) {
		if (ecode == gai_error_messages[i].ecode)
			return (gai_error_messages[i].msg);
	}

	return (gai_strerror(ecode));
}

/*
 * feed_connection_create --
 *	Connect to a host at a specified port.  Returns a file descriptor
 *	on success or -1 on failure.
 */
int
feed_connection_create(const char *host, const char *port)
{
	struct addrinfo	*res, *ai, hints;
	struct pollfd	 fds[1];
	int		 error, sock, flags, rv, val, saved_errno;
	socklen_t	 optlen = sizeof(optlen);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((error = getaddrinfo(host, port, &hints, &res)) != 0) {
		strlcpy(feed_connection_errbuf, getaddrinfo_error(error),
		    sizeof(feed_connection_errbuf));
		return (-1);
	}
	errno = EADDRNOTAVAIL;
	for (sock = -1, ai = res; ai != NULL; ai = ai->ai_next) {
		sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock == -1)
			continue;
		flags = fcntl(sock, F_GETFL, 0);
		if (flags == -1) {
			feed_connection_close(sock);
			sock = -1;
			continue;
		}
		if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
			feed_connection_close(sock);
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
			rv = poll(fds, 1, TIMEOUT * 1000);
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
		feed_connection_close(sock);
		errno = saved_errno;
		sock = -1;
	}
	freeaddrinfo(res);

	if (sock == -1)
		strlcpy(feed_connection_errbuf, strerror(errno),
		    sizeof(feed_connection_errbuf));

	return (sock);
}

/*
 * feed_connection_close --
 *	Safe version of close().  Makes sure that the descriptor is closed
 *	even when close() gets interrupted by a signal.
 */
void
feed_connection_close(int fd)
{
	int rv;

	do {
		rv = close(fd);
	} while (rv == -1 && errno == EINTR);
}

/*
 * feed_connection_recv_data --
 *	Receive all available data on the file descriptor.  Memory will
 *	be allocated when needed and a pointer to the data will be
 *	returned on success or NULL on failure.
 */
char *
feed_connection_recv_data(int fd)
{
	ssize_t	 nbytes;
	size_t	 len = 0;
	size_t	 buflen = BUFSIZ;
	char	*buf;

	buf = xmalloc(buflen);
	for (;;) {
		nbytes = feed_connection_recv(fd, buf + len, buflen - len - 1);
		if (nbytes == -1) {
			xfree(buf);
			return (NULL);
		} else if (nbytes == 0)
			break;
		buf[nbytes + len] = '\0';
		len += nbytes;

		if (len >= buflen - 1) {
			if (len > (BUFSIZ * BUFSIZ)) {
				strlcpy(feed_connection_errbuf,
				    strerror(EMSGSIZE),
				    sizeof(feed_connection_errbuf));
				break;
			}
			while (len >= buflen - 1)
				buflen *= 2;
			buf = xrealloc(buf, buflen);
		}
	}

	return (buf);
}

/*
 * feed_connection_recv --
 *	Receive data from a file descriptor.
 */
ssize_t
feed_connection_recv(int fd, char *buf, size_t len)
{
	struct pollfd	fds[1];
	ssize_t		nbytes;
	int		rv;

	do {
		fds[0].fd = fd;
		fds[0].events = POLLIN;
		rv = poll(fds, 1, TIMEOUT * 1000);
	} while (rv == -1 && errno == EINTR);

	if (rv == -1) {
		strlcpy(feed_connection_errbuf, strerror(errno),
		    sizeof(feed_connection_errbuf));
		return (-1);
	} else if (rv == 0) {
		strlcpy(feed_connection_errbuf, "Timeout waiting for data",
		    sizeof(feed_connection_errbuf));
		return (-1);
	}

	do {
		nbytes = recv(fd, buf, len, 0);
	} while (nbytes == -1 && errno == EINTR);

	if (nbytes == -1) {
		strlcpy(feed_connection_errbuf, strerror(errno),
		    sizeof(feed_connection_errbuf));
		return (-1);
	}

	return (nbytes);
}

/*
 * feed_connection_send --
 *	Send data to a file descriptor.
 */
ssize_t
feed_connection_send(int fd, const char *buf, size_t len)
{
	size_t	total, left;
	ssize_t	rv;

	total = 0;
	left = len;
	do {
		rv = send(fd, buf + total, left, 0);
		if (rv == -1) {
			strlcpy(feed_connection_errbuf, strerror(errno),
			     sizeof(feed_connection_errbuf));
			return (-1);
		}
		total += rv;
		left -= rv;
	} while (left > 1);

	return (total);
}

/*
 * feed_connection_error --
 *	Returns the last error from a connection function.
 */
const char *
feed_connection_error(void)
{
	return (feed_connection_errbuf);
}
