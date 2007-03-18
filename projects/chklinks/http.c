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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat/compat.h"
#include "chklinks.h"

/*
 * HTTP data object.
 */
struct httpdata {
	enum http_request	 request;
	int			 redirected;
	char			*data;
	char			*body;
	char			*(*getdata)(Url, enum http_request);
	const char		*(*geterror)(void);
};

/*
 * Local function prototypes.
 */
static char	  *http_getdata(Url, enum http_request);
static const char *http_geterror(void);

static int	   http_parse_header(char **, char **);
static char	 **http_parse_data(Url, const char *, size_t *);

static char	  *url_encode(const char *);
static char	  *html_decode(const char *);

/*
 * Static buffer for error messages.
 */
static char	   http_errbuf[BUFSIZ];

/*
 * http_create --
 *	Create a HTTP data object.
 */
void *
http_create(void)
{
	struct httpdata *http;

	http = xcalloc(1, sizeof(struct httpdata));
	http->redirected = FALSE;
	http->request = REQUEST_HEAD;

	http_set_getdata(http, http_getdata);
	http_set_geterror(http, http_geterror);

	return (http);
}

/*
 * http_destroy --
 *	Destroy a HTTP data object.
 */
void
http_destroy(void *data)
{
	struct httpdata *http = data;

	xfree(http->data);
	xfree(http);
}

/*
 * http_test --
 *	Test a HTTP Url.
 */
int
http_test(UrlCheck check, Url url)
{
	struct httpdata	*http = urlcheck_data(check);
	char		*data, *redirect = NULL;
	int		 code;

	/* Retrieve the data. */
	data = (*http->getdata)(url, http->request);
	if (data == NULL) {
		urlcheck_set_status(check, CONNECT_FAIL);
		urlcheck_set_message(check, (*http->geterror)());
		return (FALSE);
	}
	http->data = data;

	/* Parse the HTTP header. */
	code = http_parse_header(&data, &redirect);

	/* Possibly a non-HTTP server. */
	if (code == -1) {
		urlcheck_set_status(check, INVALID_RESPONSE);
		urlcheck_set_message(check, "Invalid response");
		return (FALSE);
	}

	/* Check for redirect. */
	if (code == 301 || code == 302) {
		urlcheck_set_rawurl(check, redirect);
		urlcheck_set_retry(check);
		xfree(http->data);
		http->data = NULL;
		http->redirected = TRUE;
		return (FALSE);
	}

	/*
	 * If we got 501 (not implemented) and tried a HEAD request, then
	 * try again with a GET request.  Same for 400 (bad request) and
	 * 405 (resource not allowed).
	 */
	if ((code == 400 || code == 405 || code == 501) &&
	    http->request == REQUEST_HEAD) {
		http->request = REQUEST_GET;
		urlcheck_set_retry(check);
		return (FALSE);
	}

	/* Any non-200 code results in an error. */
	if (code != 200) {
		urlcheck_set_status(check, CHECK_ERROR);
		urlcheck_set_message(check, "HTTP error: %d", code);
		return (FALSE);
	}
	http->body = data;
	urlcheck_set_status(check, CHECK_SUCCESS);

	if (http->redirected)
		urlcheck_set_message(check, "OK (redirected)");
	else
		urlcheck_set_message(check, "OK");

	return (TRUE);
}

/*
 * http_parse --
 *	Parse a HTTP Url.
 */
char **
http_parse(UrlCheck check, Url url, size_t *count)
{
	struct httpdata *http = urlcheck_data(check);

	http->request = REQUEST_GET;

	if (http_test(check, url) == FALSE)
		return (NULL);

	return (http_parse_data(url, http->body, count));
}

/*
 * http_request --
 *	Returns the HTTP request to be send as a string.
 */
char *
http_request(Url url, enum http_request request)
{
	char *buf, *encoded;

	encoded = url_encode(url_path(url));
	buf = xsprintf("%s %s HTTP/1.0\r\nHost: %s\r\n"
	    "User-Agent: Mozilla/5.0 (compatible; %s v%s)\r\n\r\n",
	    (request == REQUEST_HEAD) ? "HEAD" : "GET",
	    encoded, url_host(url), CHKLINKS_NAME, CHKLINKS_VERSION);

	xfree(encoded);

	return (buf);
}

/*
 * http_set_getdata --
 *	Set the data retrieve function.
 */
void
http_set_getdata(void *data, char *(*getdata)(Url, enum http_request))
{
	struct httpdata *http = data;

	http->getdata = getdata;
}

/*
 * http_set_geterror --
 *	Set the error retrieve function.
 */
void
http_set_geterror(void *data, const char *(*geterror)(void))
{
	struct httpdata *http = data;

	http->geterror = geterror;
}

/*
 * http_getdata --
 *	Connect to the Url, send the HTTP request and retrieve all data.
 *	Returns the received data on success or NULL on failure.
 */
static char *
http_getdata(Url url, enum http_request req)
{
	char	*request = NULL;
	char	*buf = NULL;
	int	 fd = -1;
	ssize_t	 rv;

	memset(http_errbuf, '\0', sizeof(http_errbuf));

	/* Connect to the host. */
	fd = connection_create(url_host(url), url_port(url));
	if (fd == -1) {
		strlcpy(http_errbuf, connection_error(), sizeof(http_errbuf));
		goto cleanup;
	}

	/* Send the HTTP request. */
	request = http_request(url, req);
	rv = connection_send(fd, request, strlen(request));
	if (rv == -1) {
		strlcpy(http_errbuf, connection_error(), sizeof(http_errbuf));
		goto cleanup;
	}

	/* Receive all data. */
	buf = connection_recv_data(fd);
	if (buf == NULL)
		strlcpy(http_errbuf, connection_error(), sizeof(http_errbuf));

 cleanup:
	/* Free used space. */
	xfree(request);

	/* Close the connection. */
	if (fd != -1)
		connection_close(fd);

	return (buf);
}

/*
 * http_geterror --
 *	Return the contents of the error buffer.
 */
static const char *
http_geterror(void)
{
	return (http_errbuf);
}

/*
 * http_parse_header --
 *	Parse data from the HTTP header.  Returns the status code found on
 *	the first line of the header.  The data is passed as a pointer and
 *	will be changed to point to the begin of the http body.
 */
static int
http_parse_header(char **data, char **redirect)
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
	if (size > BUFSIZ)
		return (-1);
	strlcpy(line, temp, size);

	/* Read the HTTP code. */
	if (sscanf(line, "HTTP/%*[0-1].%*[0-9] %d", &code) != 1)
		return (-1);

	/* Read all lines until we've got an empty line. */
	for (;;) {
		if ((p = strchr(temp, '\n')) == NULL)
			break;
		size = p - temp + 1;
		if (size > BUFSIZ)
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
			if (redirect != NULL) {
				*redirect = xstrdup(buf);
			}
		}
	}
	/* Set data pointer to the start of the http body. */
	*data = temp;

	return (code);
}

/*
 * http_parse_data --
 *	Parses the HTTP data and returns a list of all links found,
 *	including duplicates.
 */
static char **
http_parse_data(Url url, const char *data, size_t *lcount)
{
	char		**links;
	const char	*buf;
	char		*p, *q, *n, *linkp, *temp, ch;
	char		 chunk[BUFSIZ], href[BUFSIZ];
	size_t		 size, links_size, count;
	int		 flags;

	/* Initialize the array of links. */
	count = 0;
	links_size = 32;
	links = xcalloc(links_size, sizeof(char *));

	/* Extract all URLs from the data. */
	for (buf = data; ; buf = n) {
		/* Get a chunk that matches "<a ...>". */
		if ((p = strcasestr(buf, "<a ")) == NULL)
			break;
		if ((n = strchr(p, '>')) == NULL)
			break;
		size = n - p + 2;
		if (size > BUFSIZ)
			break;
		strlcpy(chunk, p, size);

		/* Extract the URL from the chunk. */
		if ((p = strcasestr(chunk, "href=")) == NULL)
			continue;
		ch = *(p + 5);
		/* Skip "href=". */
		if (ch != '"' && ch != '\'') {
			ch = '\0';
			p += 5;
		} else {
			p += 6;
		}
		/* Skip leading spaces. */
		while (*p != '\0' && isspace((unsigned char)*p))
			p++;
		if (*p == '\0')
			continue;
		if (ch == '\0') {
			if ((q = strpbrk(p, " >")) == NULL)
				continue;
		} else {
			if ((q = strchr(p, ch)) == NULL)
				continue;
		}
		q = q - 1;
		/* Strip trailing spaces. */
		while (*q != '\0' && isspace((unsigned char)*q))
			*q-- = '\0';
		size = q - p + 2;
		if (size > BUFSIZ)
			break;
		strlcpy(href, p, size);

		/* Check if this is a relative or absolute link. */
		if ((p = strpbrk(href, ":/")) == NULL ||
		    (p != NULL && *p == '/')) {
			flags = URL_SCHEME | URL_AUTHORITY;

			/* Is the path relative or absolute?  */
			if (href[0] != '/')
				flags |= URL_PATHBASE;

			temp = url_asprintf(url, flags);

			size = strlen(temp) + strlen(href) + 1;
			linkp = xmalloc(size);
			strlcpy(linkp, temp, size);
			strlcat(linkp, href, size);

			xfree(temp);
		} else {
			linkp = xstrdup(href);
		}

		if (count >= links_size) {
			links_size *= 2;
			links = xrealloc(links, links_size * sizeof(char *));
		}
		links[count] = html_decode(linkp);
		count++;

		xfree(linkp);
	}
	*lcount = count;

	return (links);
}

/*
 * url_encode --
 *	Encode unsafe characters in the URL as per RFC 1738.
 */
static char *
url_encode(const char *str)
{
	char	*encoded;
	char	 num[4];
	size_t	 size, idx;

	size = strlen(str) + 1;
	encoded = xmalloc(size);
	idx = 0;

#define SPECIAL_CHAR(x) \
    (((x) == '$') || ((x) == '-') || ((x) == '_') || ((x) == '.') || \
     ((x) == '+') || ((x) == '!') || ((x) == '*') || ((x) == '\'') || \
     ((x) == '(') || ((x) == ')') || ((x) == '/') || ((x) == '?') || \
     ((x) == ';') || ((x) == ':') || ((x) == '@') || ((x) == '=') || \
     ((x) == '&'))

	for (; *str != '\0'; str++) {
		if (!isalnum((unsigned char)*str) && !SPECIAL_CHAR(*str)) {
			snprintf(num, sizeof(num), "%%%02x", *str);

			/* Resize the encoded string and append the char. */
			size += sizeof(num);
			encoded = xrealloc(encoded, size);

			strlcat(encoded, num, size);

			idx += sizeof(num) - 1;
		} else {
			encoded[idx++] = *str;
		}
		encoded[idx] = '\0';
	}

	return (encoded);
}

/*
 * Table with some Latin-1 and special HTML entities.
 */
static const struct {
	const char *entity;
	const char ch;
} entities[] = {
	{ "&amp;",	'&' },
	{ "&quot;",	'"' },
	{ "&lt;",	'<' },
	{ "&gt;",	'>' }
};

/*
 * html_decode --
 *	Translate HTML entities back to characters.
 *	NOTE: This is not complete, many entities are missing.
 */
static char *
html_decode(const char *str)
{
	char	*decoded = xmalloc(strlen(str) + 1);
	char	*current = decoded;
	int	 intable;
	size_t	 i;

	while (*str != '\0') {
		intable = FALSE;
		for (i = 0; i < sizeof(entities) / sizeof(entities[0]); i++) {
			if (strncmp(str, entities[i].entity,
			    strlen(entities[i].entity)) == 0) {
				intable = TRUE;
				break;
			}
		}
		if (intable) {
			*current++ = entities[i].ch;
			str += strlen(entities[i].entity);
		} else if (strncmp(str, "&#", 2) == 0) {
			char num[6], *p;
			int count = 0, val;

			str += 2;	/* skip &# */
			while (*str != '\0' && *str != ';' && count < 5)
				num[count++] = *str++;
			num[count] = '\0';

			str++;		/* skip ; */

			/* Convert to integer. */
			val = strtol(num, &p, 10);
			if (*num == '\0' || *p != '\0')
				continue;

			*current++ = val;
		} else {
			*current++ = *str++;
		}
	}
	*current = '\0';

	return (decoded);
}
