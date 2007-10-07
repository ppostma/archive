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

#include "feed.h"
#include "ircbot.h"
#include "queue.h"

/*
 * A HTTP header.
 */
struct http_header {
	char			*keyword;
	char			*value;
	LIST_ENTRY(http_header)	 link;
};

/*
 * Result object of parsing the HTTP header.
 */
struct http_result {
	int			 code;
	char			*data;
	char			*body; /* points to data */
	int			 status;
	char			 errbuf[BUFSIZ];
	LIST_HEAD(, http_header) headers;
};

/*
 * Local function prototypes.
 */
static char			*http_getdata(Url, const char *, const char *);
static int			 http_parse_header(struct http_result *);

static struct http_result	*http_create(void);

static struct http_header	*http_header_create(const char *, const char *);
static void			 http_header_destroy(struct http_header *);

/*
 * http_receive --
 *	Receive the document on 'origurl'.
 */
HttpResult
http_receive(const char *origurl, const char *modified, const char *etag)
{
	struct http_result	*result;
	char			*rawurl = xstrdup(origurl);
	char			*data = NULL;
	Url			 url;

 retry:
	result = http_create();

	/* Parse the URL. */
	url = url_parse(rawurl, origurl); 
	if (url == NULL) {
		strlcpy(result->errbuf, "failed to parse URL",
		    sizeof(result->errbuf));
		goto cleanup;
	}

	/* Retrieve the data. */
	data = http_getdata(url, modified, etag);

	url_free(url);

	if (data == NULL) {
		snprintf(result->errbuf, sizeof(result->errbuf),
		    "failed to receive HTTP data (%s)",
		    feed_connection_error());
		goto cleanup;
	}
	result->data = data;

	/* Parse the HTTP header. */
	if (http_parse_header(result) == FALSE) {
		strlcpy(result->errbuf,
		    "failed to parse HTTP header (invalid response)",
		    sizeof(result->errbuf));
		goto cleanup;
	}

	/* Check for redirect. */
	if (result->code == 301 || result->code == 302) {
		xstrdup2(&rawurl, http_header_value(result, "Location"));
		xstrdup2(&data, NULL);
		http_destroy(result);
		goto retry;
	}

	/* Any non-(200|304) code results in an error. */
	if (result->code != 200 && result->code != 304) {
		snprintf(result->errbuf, sizeof(result->errbuf),
		    "HTTP error %d", result->code);
		goto cleanup;
	}
	result->status = TRUE;

 cleanup:
	xfree(rawurl);

	return (result);
}

/*
 * http_getdata --
 *	Connect to the Url, send the HTTP request and retrieve all data.
 *	Returns the received data on success or NULL on failure.
 */
static char *
http_getdata(Url url, const char *modified, const char *etag)
{
	char	*encoded;
	char	*buf = NULL;
	char	 request[BUFSIZ];
	int	 fd;
	ssize_t	 rv;

	/* Connect to the host. */
	fd = feed_connection_create(url_host(url), url_port(url));
	if (fd == -1)
		goto cleanup;

	/* Build the HTTP request query. */
	encoded = url_encode(url, URL_PATH);

	snprintf(request, sizeof(request),
	    "GET %s HTTP/1.0\r\nHost: %s\r\n"
	    "User-Agent: Mozilla/5.0 (compatible; %s v%s)\r\n",
	    encoded, url_host(url), PLUGIN_NAME, PLUGIN_VERSION);

	xfree(encoded);

	if (modified != NULL) {
		snprintf(request + strlen(request),
		    sizeof(request) - strlen(request),
		    "If-Modified-Since: %s\r\n", modified);

		if (etag != NULL) {
			snprintf(request + strlen(request),
			    sizeof(request) - strlen(request),
			    "ETag: %s\r\n", etag);
		}
	}
	strlcat(request, "\r\n", sizeof(request));

	/* Send the HTTP request. */
	rv = feed_connection_send(fd, request, strlen(request));
	if (rv == -1)
		goto cleanup;

	/* Receive all data. */
	buf = feed_connection_recv_data(fd);

 cleanup:
	/* Close the connection. */
	if (fd != -1)
		feed_connection_close(fd);

	return (buf);
}

/*
 * http_parse_header --
 *	Parse data from the HTTP header.
 */
static int
http_parse_header(struct http_result *result)
{
	struct http_header	*header;
	char			*temp = result->data;
	char			*keyword, *value, *p;
	char			 line[BUFSIZ];
	char			 buf[BUFSIZ];
	size_t			 size;
	int			 code;

	memset(buf, '\0', sizeof(buf));

	/* Extract the first line, this should contain the HTTP status code. */
	if ((p = strchr(temp, '\n')) == NULL)
		return (FALSE);

	size = p - temp + 1;
	if (size > BUFSIZ)
		return (FALSE);
	strlcpy(line, temp, size);

	/* Read the HTTP code. */
	if (sscanf(line, "HTTP/%*[0-1].%*[0-9] %d", &code) != 1)
		return (FALSE);
	result->code = code;

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

		/* Parse the header. */
		if ((p = strchr(line, ':')) == NULL)
			continue;
		*p = '\0';

		keyword = line;
		value = p + 1;

		while (*value != '\0' && isspace((unsigned char)*value))
			value++;

		header = http_header_create(keyword, value);
		LIST_INSERT_HEAD(&result->headers, header, link);
	}
	/* Set data pointer to the start of the http body. */
	result->body = temp;

	return (TRUE);
}

/*
 * http_create --
 *	Create a http_result object.
 */
static struct http_result *
http_create(void)
{
	struct http_result *result;

	result = xcalloc(1, sizeof(struct http_result));
	result->code = -1;
	result->status = FALSE;
	LIST_INIT(&result->headers);

	return (result);
}

/*
 * http_destroy --
 *	Destroy a http_result object.
 */
void
http_destroy(HttpResult result)
{
	struct http_header *header;

	while ((header = LIST_FIRST(&result->headers)) != NULL) {
		LIST_REMOVE(header, link);
		http_header_destroy(header);
	}

	xfree(result->data);
	xfree(result);
}

/*
 * http_result_code --
 *	Accessor function for the 'code' member.
 */
int
http_result_code(HttpResult result)
{
	return (result->code);
}

/*
 * http_result_status --
 *	Accessor function for the 'status' member.
 */
int
http_result_status(HttpResult result)
{
	return (result->status);
}

/*
 * http_result_body --
 *	Accessor function for the 'body' member.
 */
const char *
http_result_body(HttpResult result)
{
	return (result->body);
}

/*
 * http_result_error --
 *	Accessor function for the 'errbuf' member.
 */
const char *
http_result_error(HttpResult result)
{
	return (result->errbuf);
}

/*
 * http_header_create --
 *	Create a http_header object.
 */
static struct http_header *
http_header_create(const char *keyword, const char *value)
{
	struct http_header *header;

	header = xmalloc(sizeof(struct http_header));
	header->keyword = xstrdup(keyword);
	header->value = xstrdup(value);

	return (header);
}

/*
 * http_header_destroy --
 *	Destroy a http_header object.
 */
static void
http_header_destroy(struct http_header *header)
{
	xfree(header->keyword);
	xfree(header->value);
	xfree(header);
}

/*
 * http_header_value --
 *	Get a HTTP keyword value from a HTTP result.
 */
const char *
http_header_value(HttpResult result, const char *keyword)
{
	struct http_header *header;

	LIST_FOREACH(header, &result->headers, link) {
		if (strcasecmp(keyword, header->keyword) == 0)
			return (header->value);
	}

	return (NULL);
}
