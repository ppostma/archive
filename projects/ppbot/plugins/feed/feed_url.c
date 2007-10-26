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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "feed.h"
#include "ircbot.h"

/* URL format. */
enum url_format {
	URL_UNKNOWN,
	URL_GENERIC
};

/*
 * URL object.
 */
struct url {
	char		*scheme;
	char		*host;
	char		*port;
	char		*path;
	enum url_format	 format;
};

/*
 * Table with recognized schemes/protocols.
 */
static const struct {
	const char	*name;
	const char	*port;
	enum url_format	 format;
} schemes[] = {
	{ "feed",	"80",	URL_GENERIC },
	{ "feed:http",	"80",	URL_GENERIC },
	{ "http",	"80",	URL_GENERIC },
	{ "rss",	"80",	URL_GENERIC }
};

/*
 * url_parse --
 *	Parse an URL and return an Url object if parsing was successful.
 *	Returns NULL on a parsing failure.
 */
Url
url_parse(const char *str, const char *base)
{
	struct url	*url;
	enum url_format	 format;
	char		*scheme, *host, *port, *path;
	char		*p, *temp, *saved_buf, *buf;
	int		 flags, found = FALSE;
	size_t		 i, size;

	/* Check if 'str' is a relative URL. */
	if (base != NULL && strcasecmp(str, base) != 0) {
		url = url_parse(base, NULL);
		if (url == NULL)
			return (NULL);

		flags = 0;
		if ((p = strpbrk(str, ":/")) == NULL ||
		    (p != NULL && *p == '/')) {
			flags |= URL_SCHEME | URL_AUTHORITY;

			if (str[0] != '/')
				flags |= URL_PATHBASE;
		}

		if (flags != 0) {
			/* Create a new input string. */
			temp = url_asprintf(url, flags);

			size = strlen(temp) + strlen(str) + 1;
			buf = xmalloc(size);
			strlcpy(buf, temp, size);
			strlcat(buf, str, size);

			xfree(temp);
		} else {
			/* Make a copy of the input string. */
			buf = xstrdup(str);
		}

		url_free(url);
	} else {
		/* Make a copy of the input string. */
		buf = xstrdup(str);
	}
	saved_buf = buf;

	/* Reset the return values. */
	scheme = NULL;
	host = NULL;
	port = NULL;
	path = NULL;

	/* Check if the scheme if valid. */
	if ((p = strstr(buf, "://")) == NULL)
		goto error;
	*p = '\0';

	/* Change the scheme to lowercase. */
	for (p = buf; *p != '\0'; p++)
		*p = tolower((unsigned char)*p);

	for (i = 0; i < sizeof(schemes) / sizeof(schemes[0]); i++) {
		if (strcmp(buf, schemes[i].name) == 0) {
			found = TRUE;
			break;
		}
	}

	/* Was the scheme recognized? */
	if (!found)
		goto error;

	scheme = xstrdup(schemes[i].name);
	format = schemes[i].format;

	/* Don't bother to parse it if unknown. */
	if (format == URL_UNKNOWN)
		goto finish;

	buf += strlen(schemes[i].name) + 1;
	while (*buf == '/' || isspace((unsigned char)*buf))
		buf++;
	if (*buf == '\0')
		goto error;

	/* Extract the hostname and optionally the port. */
	if ((p = strpbrk(buf, ":/")) != NULL && *p == ':') {
		/* There's a port number specified. */
		*p++ = '\0';

		temp = buf;
		buf = p;
		/* Path present? */
		if ((p = strchr(buf, '/')) != NULL)
			*p++ = '\0';

		if (*buf == '\0')
			goto error;

		port = xstrdup(buf);
		/* Point buf to the hostname */
		buf = temp;
	} else {
		/* Path present? */
		if ((p = strchr(buf, '/')) != NULL)
			*p++ = '\0';

		/* No port number specified, use the one from the scheme. */
		port = xstrdup(schemes[i].port);
	}
	/* Fill the hostname. */
	host = xstrdup(buf);

	buf = p;
	/* Extract the path. */
	if (buf != NULL) {
		/* Remove the anchor when present. */
		if ((p = strchr(buf, '#')) != NULL)
			*p = '\0';

		size = strlen(buf) + 2;  /* "/" + buf + NUL */

		path = xmalloc(size);

		strlcpy(path, "/", size);
		strlcat(path, buf, size);
	} else {
		path = xstrdup("/");
	}

	xfree(saved_buf);

 finish:
	url = xcalloc(1, sizeof(struct url));
	url->scheme = scheme;
	url->host = host;
	url->port = port;
	url->path = path;
	url->format = format;

	return (url);

 error:
	xfree(path);
	xfree(port);
	xfree(host);
	xfree(scheme);

	xfree(saved_buf);

	return (NULL);
}

/*
 * url_asprintf --
 *	Return printable form of an Url object.
 */
char *
url_asprintf(Url url, int flags)
{
	char	*str, *p;
	size_t	 i, size = 0;
	int	 port = FALSE;

	if (url->format == URL_UNKNOWN)
		return (NULL);

	if (flags & URL_SCHEME) {
		size += strlen(url->scheme);
		size += 3; /* + "://" */
	}
	if (flags & URL_HOST) {
		size += strlen(url->host);
	}
	if (flags & URL_PORT) {
		for (i = 0; i < sizeof(schemes) / sizeof(schemes[0]); i++) {
			if (strcmp(url->scheme, schemes[i].name) == 0 &&
			    schemes[i].port != NULL &&
			    strcmp(url->port, schemes[i].port) != 0) {
				port = TRUE;
				size += strlen(url->port) + 1; /* + ":" */
				break;
			}
		}
	}
	if (flags & URL_PATH) {
		size += strlen(url->path);
	}
	if (flags & URL_PATHBASE) {
		p = strrchr(url->path, '/');
		if (p == NULL) {
			size += strlen(url->path);
		} else {
			size += p - url->path + 1;
		}
	}
	size += 1; /* + NUL */

	str = xcalloc(1, size);

	if (flags & URL_SCHEME) {
		strlcpy(str, url->scheme, size);
		strlcat(str, "://", size);
	}
	if (flags & URL_HOST) {
		strlcat(str, url->host, size);
	}
	if (flags & URL_PORT) {
		if (port) {
			strlcat(str, ":", size);
			strlcat(str, url->port, size);
		}
	}
	if (flags & URL_PATH) {
		strlcat(str, url->path, size);
	}
	if (flags & URL_PATHBASE) {
		strlcat(str, url->path, size);
	}

	return (str);
}

/*
 * url_encode --
 *	Encode unsafe characters in the URL as per RFC 1738.
 */
char *
url_encode(Url url, int flags)
{
	char	*decoded;
	char	*encoded;
	char	*str;
	char	 num[4];
	size_t	 size, idx;

	decoded = url_asprintf(url, flags);
	if (decoded == NULL)
		return (NULL);

	str = decoded;
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

	xfree(decoded);

	return (encoded);
}

/*
 * url_free --
 *	Free space used by an Url object.
 */
void
url_free(Url url)
{
	xfree(url->scheme);
	xfree(url->host);
	xfree(url->port);
	xfree(url->path);
	xfree(url);
}

/*
 * url_scheme --
 *	Accessor function for the 'scheme' member.
 */
const char *
url_scheme(Url url)
{
	return (url->scheme);
}

/*
 * url_host --
 *	Accessor function for the 'host' member.
 */
const char *
url_host(Url url)
{
	return (url->host);
}

/*
 * url_port --
 *	Accessor function for the 'port' member.
 */
const char *
url_port(Url url)
{
	return (url->port);
}

/*
 * url_path --
 *	Accessor function for the 'path' member.
 */
const char *
url_path(Url url)
{
	return (url->path);
}
