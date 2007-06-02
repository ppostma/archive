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
 * RSS 2.0 specification: http://blogs.law.harvard.edu/tech/rss
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "compat/compat.h"
#include "plugin.h"
#include "rss.h"
#include "timer.h"

/*
 * Local macros.
 */
#define RSS_CONFIG	"rss.conf"

/* A single item in a RSS. */
struct item {
	char			*title;
	char			*url;
	char			*pubdate;
	TAILQ_ENTRY(item)	 link;
};

/* RSS channel. */
struct rss {
	char			*version;
	TAILQ_HEAD(, item)	 items;
};

/* RSS channel configuration. */
struct rss_head {
	struct rss		*old;
	struct rss		*new;
	struct rss_config	*config;
	time_t			 last;
	TAILQ_ENTRY(rss_head)	 link;
};

static TAILQ_HEAD(, rss_head) rss_head = TAILQ_HEAD_INITIALIZER(rss_head);

/* Supported schemes to connect to. */
static const struct {
	const char *name;
	const char *port;
} schemes[] = {
	{ "http", "80" }
};

/* Function prototypes. */
static void	 check_message(Connection, Message);

static void	 rss_command(Connection, Message, const char *);
static void	 rss_update(struct rss_head *);

static void	 rss_head_free(struct rss_head *);

static struct rss *rss_read(struct rss_config *);
static void	 rss_free(struct rss *);

static int	 host_connect(const char *, const char *);
static int	 parse_url(const char *, char **, char **, char **);
static ssize_t	 send_http_request(int, const char *, const char *);
static char	*receive_data(int);
static char	*http_connect(const char *, const char *, const char *);
static int	 parse_http_header(char **, char **);
static int	 parse_http_data(const char *, struct rss_config *,
			struct rss **);

static char	*html_decode(const char *);

static void	 show_item(Connection, struct rss_config *, const char *,
			struct item *);
static void	 show_item_channel(struct rss_head *, struct item *);
static void	 show_new_items(struct rss_head *);

/*
 * plugin_open --
 *	Called when the plugin is loaded/opened.  This function loads the
 *	configuration file and installs the timers for reading the RSS feed.
 */
void
plugin_open(Plugin p)
{
	struct rss_config *rcp;
	struct rss_head	  *rhp;
	struct timeval	   tv;

	rss_config_parse(RSS_CONFIG);

	/* Setup a timer for each RSS and run an initial update. */
	TAILQ_FOREACH(rhp, &rss_head, link) {
		rcp = rhp->config;

		if (rcp->update) {
			TV_SET(&tv, 60 * rcp->refresh, 0);
			timer_schedule(tv, (void *)rss_update, rhp, TIMER_POLL,
			    "rss_update#%lx", (unsigned long)rhp);
			rss_update(rhp);
		} else {
			rhp->last = time(NULL) - (rcp->refresh * 60) - 60;
		}
	}

	/* Install the commands. */
	callback_register(p, "PRIVMSG", MSG_USER|MSG_DATA, 1, check_message);
}

/*
 * plugin_close --
 *	Called when the plugin is unloaded/closed.
 */
void
plugin_close(Plugin p)
{
	struct rss_head *rhp;

	/* Cancel all timers. */
	TAILQ_FOREACH(rhp, &rss_head, link) {
		if (rhp->config->update)
			timer_cancel("rss_update#%lx", (unsigned long)rhp);
	}

	/* Destroy all rss feeds. */
	while ((rhp = TAILQ_FIRST(&rss_head)) != NULL) {
		TAILQ_REMOVE(&rss_head, rhp, link);
		rss_head_free(rhp);
	}

	/*
	 * Free memory allocated by the yacc/lex generated files.
	 * This is needed because we unload the shared object and then load
	 * it again.  Allocated memory isn't freed upon unload and will not
	 * be used ever again so if we don't free it, it will leak.
	 */
	rss_lex_cleanup();
	rss_yacc_cleanup();
}

/*
 * rss_head_create --
 *	Create a new RSS head structure.
 */
void
rss_head_create(struct rss_config *rcp)
{
	struct rss_head *rhp;

	rhp = xcalloc(1, sizeof(struct rss_head));
	rhp->config = rcp;
	TAILQ_INSERT_TAIL(&rss_head, rhp, link);
}

/*
 * rss_head_free --
 *      Free all allocated space for an RSS head structure.
 */
static void
rss_head_free(struct rss_head *rhp)
{
	rss_config_free(rhp->config);

	/* Free the old and new rss. */
	if (rhp->old != NULL && rhp->new != rhp->old)
		rss_free(rhp->old);
	if (rhp->new != NULL)
		rss_free(rhp->new);

	xfree(rhp);
}

/*
 * check_message --
 *	Handles the PRIVMSG message.
 */
static void
check_message(Connection conn, Message msg)
{
	const char *buf;

	/* Check if the message was for us. */
	if (message_to_me(conn, msg, &buf) == FALSE)
		return;

	if (strncmp(buf, "rss", 3) != 0)
		return;

	buf += 3;
	if (*buf != '\0' && !isspace((unsigned char)*buf))
		return;

	while (*buf != '\0' && isspace((unsigned char)*buf))
		buf++;

	rss_command(conn, msg, buf);
}

/*
 * rss_command --
 *	Handles the rss command, the command to display headlines from
 *	an RSS feed.
 */
static void
rss_command(Connection conn, Message msg, const char *args)
{
	const char	*channel = message_parameter(msg, 0);
	const char	*sender = message_sender(msg);
	struct rss_head	*rhp, *n;
	struct item	*ip;
	char		 buf[BUFSIZ];
	size_t		 count;
	time_t		 elapsed;

	/* Show usage if there are no arguments, otherwise show headlines. */
	if (*args == '\0') {
		snprintf(buf, sizeof(buf), "%s: available feeds: ",
		    sender);

		rhp = TAILQ_FIRST(&rss_head);
		if (rhp != NULL) {
			strlcat(buf, rhp->config->id, sizeof(buf));

			n = TAILQ_NEXT(rhp, link);
			for (rhp = n; rhp; rhp = TAILQ_NEXT(rhp, link)) {
				strlcat(buf, ", ", sizeof(buf));
				strlcat(buf, rhp->config->id, sizeof(buf));
			}
		}

		send_privmsg(conn, channel, buf);
	} else {
		TAILQ_FOREACH(rhp, &rss_head, link) {
			if (strcasecmp(args, rhp->config->id) == 0)
				break;
		}
		if (rhp == NULL) {
			send_privmsg(conn, channel, "%s: no such feed.",
			    sender);
			return;
		}
		if (!rhp->config->update) {
			elapsed = time(NULL) - rhp->last;

			if ((elapsed / 60) >= rhp->config->refresh) {
				if (rhp->new != NULL)
					rss_free(rhp->new);

				rhp->new = rss_read(rhp->config);
				rhp->last = time(NULL);
			} else {
				log_debug("No refresh needed for %s",
				    rhp->config->id);
			}
		}
		if (rhp->new == NULL) {
			send_privmsg(conn, channel,
			    "%s: feed appears to have no data.", sender);
			return;
		}
		count = rhp->config->showcount;
		TAILQ_FOREACH(ip, &rhp->new->items, link) {
			show_item(conn, rhp->config, channel, ip);
			if (--count == 0)
				break;
		}
	}
}

/*
 * rss_update --
 *	Update the RSS feed and show the new item(s) in the configured
 *	channel(s).  This function is called when opening the plugin and
 *	each time a timer finishes.
 */
static void
rss_update(struct rss_head *rhp)
{
	log_debug("Processing feed %s", rhp->config->id);

	rhp->new = rss_read(rhp->config);
	if (rhp->new != NULL && rhp->old != NULL)
		show_new_items(rhp);

	if (rhp->new != NULL) {
		if (rhp->old != NULL)
			rss_free(rhp->old);
		rhp->old = rhp->new;
	}
}

/*
 * rss_read --
 *	Connection to 'location' and read the RSS found there.
 *	If anything went wrong, this function will return NULL instead
 *	of the RSS structure.
 */
static struct rss *
rss_read(struct rss_config *rcp)
{
	struct rss	*rp = NULL;
	char		*host = NULL, *port = NULL, *uri = NULL;
	char		*data = NULL, *saved_data = NULL, *buf = NULL;
	int		 code, status = FALSE;

	if (parse_url(rcp->location, &host, &port, &uri) == FALSE) {
		log_warnx("Unable to parse URL %s.", buf);
		return (NULL);
	}
 again:
	data = http_connect(host, port, uri);
	if (data == NULL) {
		log_warnx("Unable to get data from %s.", rcp->location);
		goto out;
	}
	saved_data = data;

	code = parse_http_header(&data, &buf);
	if ((code == 301 || code == 302) && buf != NULL) {
		/* Redirect, but first free the url. */
		xfree(uri);
		xfree(port);
		xfree(host);
		/* Parse the URL and then redirect to the new location. */
		if (parse_url(buf, &host, &port, &uri) == FALSE) {
			log_warnx("Unable to parse URL %s.", buf);
			xfree(buf);
			goto out;
		}
		xfree(buf);
		xfree(saved_data);
		goto again;
	} else if (code != 200) {
		log_warnx("Got an unexpected %d HTTP response (feed %s).",
		    code, rcp->id);
		goto out;
	}

	rp = xcalloc(1, sizeof(struct rss));
	TAILQ_INIT(&rp->items);

	status = parse_http_data(data, rcp, &rp);
	if (status == FALSE) {
		if (rp->version == NULL) {
			log_warnx("Unable to determine RSS version.");
		}
		log_warnx("Unable to parse data from feed %s.", rcp->id);
	}

 out:
	xfree(saved_data);
	xfree(uri);
	xfree(port);
	xfree(host);

	if (!status && rp != NULL) {
		/* Only return the RSS if parsing completed succesfully. */
		rss_free(rp);
		return (NULL);
	}
	return (rp);
}

/*
 * rss_free --
 *	Free all allocated space in an RSS structure.
 */
static void
rss_free(struct rss *rp)
{
	struct item *ip;

	/* Clean up all items. */
	while ((ip = TAILQ_FIRST(&rp->items)) != NULL) {
		TAILQ_REMOVE(&rp->items, ip, link);
		xfree(ip->title);
		xfree(ip->url);
		xfree(ip->pubdate);
		xfree(ip);
	}

	/* Clean up rss members. */
	xfree(rp->version);
	xfree(rp);
}

/*
 * host_connect --
 *	Connect to a host at a specified port.  Return a file descriptor
 *	on success or -1 on failure.
 */
static int
host_connect(const char *host, const char *port)
{
	struct addrinfo	*res, *ai, hints;
	struct pollfd	 fds[1];
	int		 error, sock, flags, rv, val, saved_errno;
	socklen_t	 optlen = sizeof(optlen);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((error = getaddrinfo(host, port, &hints, &res)) != 0) {
		log_warnx("Unable to resolve %s[%s]: %s", host, port,
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
		log_warn("Unable to connect to %s[%s]", host, port);

	return (sock);
}

/*
 * parse_url --
 *	Parse an URL (input) and return the parts via pointers.
 *	Return TRUE on success or FALSE on failure.
 */
static int
parse_url(const char *input, char **host, char **port, char **uri)
{
	char	*p, *buf, *saved_buf, *temp = NULL;
	int	 found = FALSE;
	size_t	 i, size;

	/* Make a copy of the input. */
	buf = xstrdup(input);
	saved_buf = buf;

	/* Reset the return values. */
	*host = NULL;
	*port = NULL;
	*uri = NULL;

	/* Check if the scheme if valid. */
	if ((p = strchr(buf, ':')) == NULL)
		goto error;
	*p = '\0';

	for (i = 0; i < sizeof(schemes) / sizeof(schemes[0]); i++) {
		if (strncasecmp(input, schemes[i].name,
		    strlen(schemes[i].name)) == 0) {
			found = TRUE;
			break;
		}
	}

	/* Was the scheme recognized? */
	if (!found)
		goto error;

	buf += strlen(schemes[i].name) + 1;
	while (*buf == '/')
		buf++;
	if (*buf == '\0')
		goto error;

	/* Extract the hostname. */
	if ((p = strchr(buf, ':')) != NULL) {
		/* There's a port number specified. */
		*p++ = '\0';

		temp = buf;
		buf = p;
		/* URI present? */
		if ((p = strchr(buf, '/')) == NULL)
			goto error;
		*p++ = '\0';

		if (*buf == '\0')
			goto error;

		*port = xstrdup(buf);
		/* Point buf to the hostname. */
		buf = temp;
	} else {
		/* URI present? */
		if ((p = strchr(buf, '/')) == NULL)
			goto error;
		*p++ = '\0';

		/* No port number specified, use the one from the scheme. */
		*port = xstrdup(schemes[i].port);
	}
	/* Fill the hostname. */
	*host = xstrdup(buf);

	/* Remove the anchor when present. */
	buf = p;
	if ((p = strchr(buf, '#')) != NULL)
		*p = '\0';
	/* Extract the URI. */
	if (*buf != '\0') {
		size = strlen(buf) + 2;  /* '/' + buf + NUL */

		*uri = xmalloc(size);

		strlcpy(*uri, "/", size);
		strlcat(*uri, buf, size);
	} else {
		*uri = xstrdup("/");
	}

	xfree(saved_buf);

	return (TRUE);

 error:
	xfree(*uri);
	xfree(*port);
	xfree(*host);
	xfree(saved_buf);

	return (FALSE);
}

/*
 * send_http_request --
 *	Send a HTTP GET request to a host.  Return the amount
 *	of bytes sent or -1 on failure.
 */
static ssize_t
send_http_request(int fd, const char *host, const char *uri)
{
	char	*buf;
	ssize_t	 nbytes;

	buf = xsprintf("GET %s HTTP/1.0\r\nHost: %s\r\n"
	    "User-Agent: Mozilla/5.0 (compatible; )\r\n\r\n", uri, host);

	nbytes = send(fd, buf, strlen(buf), 0);
	xfree(buf);

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
 * http_connect --
 *	Short for doing: connect, sending http request, receiving all data.
 *	Return the received data on success or NULL on failure.
 */
static char *
http_connect(const char *host, const char *port, const char *uri)
{
	char *data;
	int sock;

	sock = host_connect(host, port);
	if (sock == -1)
		return (NULL);

	if (send_http_request(sock, host, uri) == -1) {
		connection_safeclose(sock);
		return (NULL);
	}

	data = receive_data(sock);

	connection_safeclose(sock);

	return (data);
}

/*
 * parse_http_header --
 *	Parse data from the HTTP header.  Return the status code found on
 *	the first line of the header.  The data is passed as a pointer and
 *	will be changed to point to the begin of the http body.
 */
static int
parse_http_header(char **data, char **str)
{
	char	*p, *temp = *data;
	char	 line[BUFSIZ], buf[BUFSIZ];
	size_t	 size;
	int	 code = 0;

	memset(buf, '\0', sizeof(buf));

	/* Extract the first line, this should contain the HTTP status code. */
	if ((p = strchr(temp, '\n')) == NULL)
		return (-1);
	size = p - temp + 1;
	if (size > sizeof(line))
		return (-1);
	strlcpy(line, temp, size);

	/* Read the HTTP code. */
	if (sscanf(line, "HTTP/%*[0-1].%*[0-9] %d", &code) != 1)
		return (-1);

	/* Read all lines until we get an empty line. */
	for (;;) {
		if ((p = strchr(temp, '\n')) == NULL)
			break;
		size = p - temp + 1;
		if (size > sizeof(line))
			break;
		strlcpy(line, temp, size);
		/* Set pointer to the next line. */
		temp = p + 1;
		/* Strip \r and \n from the current line. */
		if ((p = strpbrk(line, "\r\n")) != NULL)
			*p = '\0';
		if (strlen(line) == 0)
			break;

		/* Check for a redirect. */
		if (sscanf(line, "Location: %s\n", buf) == 1) {
			*str = xstrdup(buf);
		}
        }
	/* Set data pointer to the start of the http body. */
	*data = temp;

	return (code);
}

/*
 * parse_http_data --
 *	Parse the data received via HTTP.
 */
static int
parse_http_data(const char *buf, struct rss_config *rcp, struct rss **rssp)
{
	struct rss	*rp = *rssp;
	struct item	*ip = NULL;
	const char	*data = buf;
	char		*next, *value, *str, *p = NULL;
	char		*title = NULL, *url = NULL, *pubdate = NULL;
	int		 start = FALSE, cdata = FALSE, item = FALSE;
	int		 status = TRUE;
	size_t		 len, size = BUFSIZ;

	/* Temporary buffer for data. */
	str = xcalloc(1, size);

	while (*data != '\0') {
		/* Skip spaces. */
		while (*data != '\0' && isspace((unsigned char)*data))
			data++;
		if (*data == '\0')
			break;
		/* Check for element begin. */
		if (*data == '<' && !cdata) {
			/* Check for CDATA begin. */
			if (strncmp(data, "<![CDATA[", 9) == 0) {
				data = data + 9;
				cdata = TRUE;
				continue;
			}
			/* Skip spaces. */
			while (*data != '\0' && isspace((unsigned char)*data))
				data++;
			/* Check if this is an ending element. */
			if (*++data == '/') {
				start = FALSE;
				++data;
			} else {
				start = TRUE;
			}
			/* Find end of element. */
			if ((p = strchr(data, '>')) == NULL) {
				log_debug("end of element not found");
				status = FALSE;
				break;
			}
			*p++ = '\0';
			next = p;

			if (start && strncmp(data, "rss", 3) == 0) {
				/* Find the version string. */
				if ((p = strcasestr(data, "version")) == NULL) {
					log_debug("version string not found");
					status = FALSE;
					break;
				}
				if ((p = strchr(p, '=')) == NULL) {
					log_debug("invalid version format");
					status = FALSE;
					break;
				}
				if (*++p == '"')
					p++;
				value = p;
				if ((p = strchr(p, '"')) == NULL) {
					log_debug("string terminator missing");
					status = FALSE;
					break;
				}
				*p = '\0';

				/* Save the RSS version. */
				if (rp->version != NULL) {
					xfree(rp->version);
					rp->version = NULL;
				}
				rp->version = html_decode(value);
			} else if (start && strcmp(data, "item") == 0) {
				item = TRUE;
			} else if (!start && strcmp(data, "item") == 0) {
				/* Add the item. */
				if (rcp->ignore != NULL && title != NULL &&
				    strncasecmp(rcp->ignore, title,
				    strlen(rcp->ignore)) == 0) {
					xfree(title);
					xfree(url);
					xfree(pubdate);
				} else {
					ip = xcalloc(1, sizeof(struct item));
					ip->title = title;
					ip->url = url;
					ip->pubdate = pubdate;
					TAILQ_INSERT_TAIL(&rp->items, ip, link);
				}

				/* Reset the value keepers. */
				title = NULL;
				url = NULL;
				pubdate = NULL;

				item = FALSE;
			} else if (!start && strcmp(data, "title") == 0) {
				if (title != NULL) {
					xfree(title);
					title = NULL;
				}
				if (item) {
					title = html_decode(str);
				}
			} else if (!start &&
			    (strcmp(data, "feedburner:origLink") == 0 ||
			     strcmp(data, "link") == 0)) {
				if (url != NULL) {
					xfree(url);
					url = NULL;
				}
				if (item) {
					url = html_decode(str);
				}
			} else if (!start && strcmp(data, "pubDate") == 0) {
				if (pubdate != NULL) {
					xfree(pubdate);
					pubdate = NULL;
				}
				if (item) {
					pubdate = html_decode(str);
				}
			}
			memset(str, '\0', size);
		} else {
			/* Find the ending element. */
			if (!cdata && (p = strchr(data, '<')) == NULL) {
				log_debug("no element end found");
				status = FALSE;
				break;
			}
			next = p;

			/* Find end of CDATA. */
			if (cdata && (p = strstr(data, "]]>")) != NULL) {
				cdata = FALSE;
				next = p + 3;
			}

			/* Copy the data to a temporary buffer. */
			len = p - data + 1;
			if (len > size) {
				if (len > (BUFSIZ * BUFSIZ)) {
					errno = ENOMEM;
					log_warn("Unable to allocate memory");
					status = FALSE;
					break;
				}
				while (len > size)
					size *= 2;
				str = xrealloc(str, size);
			}
			strlcpy(str, data, len);
		}
		data = next;
	}

	/* Check for elements that should be freed. */
	xfree(title);
	xfree(url);
	xfree(pubdate);
	xfree(str);

	return (status);
}

/*
 * html_decode --
 *	Translate HTML entities back to characters.
 *	NOTE: This is not complete, many entities are missing.
 */
static char *
html_decode(const char *str)
{
	char *decoded = xmalloc(strlen(str) + 1);
	char *current = decoded;

	while (*str != '\0') {
		if (strncmp(str, "&amp;", 5) == 0) {
			*current++ = '&';
			str += 5;
		} else if (strncmp(str, "&quot;", 6) == 0) {
			*current++ = '"';
			str += 6;
		} else if (strncmp(str, "&lt;", 4) == 0) {
			*current++ = '<';
			str += 4;
		} else if (strncmp(str, "&gt;", 4) == 0) {
			*current++ = '>';
			str += 4;
		} else if (strncmp(str, "&eacute;", 8) == 0) {
			*current++ = 'é';
			str += 8;
		} else if (strncmp(str, "&iuml;", 6) == 0) {
			*current++ = 'à';
			str += 6;
		} else if (strncmp(str, "&#", 2) == 0) {
			char num[6], *p;
			int i = 0, val;

			str += 2;	/* skip &# */
			while (*str != '\0' && *str != ';' && i < 5)
				num[i++] = *str++;
			num[i] = '\0';

			str++;		/* skip ; */

			/* Convert to integer. */
			val = strtol(num, &p, 10);
			if (*num == '\0' || *p != '\0') {
				log_debug("%s: Invalid number to convert", num);
				continue;
			}

			*current++ = val;
		} else {
			*current++ = *str++;
		}
	}
	*current = '\0';

	return (decoded);
}

/*
 * show_item --
 *	Display a single RSS item at the specified destination.
 */
static void
show_item(Connection conn, struct rss_config *rcp, const char *dest,
    struct item *ip)
{
	send_privmsg(conn, dest, "%s) %s - %s", rcp->name, ip->title, ip->url);
}

/*
 * show_item_channel --
 *	Display a single RSS item in the configured channels in
 *	the RSS head.
 */
static void
show_item_channel(struct rss_head *head, struct item *ip)
{
	struct rss_dest	*chan;
	Connection	 conn;

	LIST_FOREACH(chan, &head->config->channels, link) {
		conn = connection_find(chan->id);
		if (conn == NULL) {
			log_debug("Connection '%s' not found.", chan->id);
			continue;
		}
		show_item(conn, head->config, chan->channel, ip);
	}
}

/*
 * show_new_items --
 *	Display new items in the configured channels.  This works by
 *	comparing the new RSS to the old RSS and dislaying the items
 *	that aren't found in the old RSS.
 */
static void
show_new_items(struct rss_head *head)
{
	struct item	*np, *op;
	size_t		 count = 0;
	int		 exists;

	TAILQ_FOREACH(np, &head->new->items, link) {
		exists = FALSE;
		TAILQ_FOREACH(op, &head->old->items, link) {
			if (op->url != NULL && np->url != NULL) {
				if (strcmp(op->url, np->url) == 0) {
					exists = TRUE;
					break;
				}
                        } else if (op->pubdate != NULL && np->pubdate != NULL) {
                                if (strcmp(op->pubdate, np->pubdate) == 0) {
                                        exists = TRUE;
                                        break;
                                }
			} else if (op->title != NULL && np->title != NULL) {
				if (strcmp(op->title, np->title) == 0) {
					exists = TRUE;
					break;
				}
			}
		}
		if (!exists) {
			show_item_channel(head, np);

			if (++count >= head->config->showcount)
				break;
		}
	}
}
