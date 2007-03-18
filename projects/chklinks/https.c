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

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "compat/compat.h"
#include "chklinks.h"

/*
 * Local function prototypes.
 */
static char	  *https_getdata(Url, enum http_request);
static const char *https_geterror(void);

/*
 * Static buffer for error messages.
 */
static char	   https_errbuf[BUFSIZ];

/*
 * https_create --
 *	Create a HTTP data object.
 */
void *
https_create(void)
{
	void		*http;
	static int	 initialized = FALSE;

	if (!initialized) {
		SSL_library_init();
		SSL_load_error_strings();
		SSLeay_add_ssl_algorithms();

		initialized = TRUE;
	}

	http = http_create();
	/* Override the getdata/geterror methods. */
	http_set_getdata(http, https_getdata);
	http_set_geterror(http, https_geterror);

	return (http);
}

/*
 * https_destroy --
 *	Destroy a HTTP data object.
 */
void
https_destroy(void *data)
{
	http_destroy(data);
}

/*
 * https_test --
 *	Test a HTTPS Url.
 */
int
https_test(UrlCheck check, Url url)
{
	return (http_test(check, url));
}

/*
 * https_parse --
 *	Parse a HTTPS Url.
 */
char **
https_parse(UrlCheck check, Url url, size_t *count)
{
	return (http_parse(check, url, count));
}

/*
 * https_getdata --
 *	Connect to the Url, send the HTTP request and retrieve all data.
 *	Returns the received data on success or NULL on failure.
 */
static char *
https_getdata(Url url, enum http_request req)
{
	SSL_CTX	*ssl_ctx = NULL;
	SSL	*ssl = NULL;
	char	*request = NULL;
	char	*buf = NULL;
	size_t	 buflen = BUFSIZ;
	size_t	 len = 0;
	int	 fd = -1;
	int	 rv;

	memset(https_errbuf, '\0', sizeof(https_errbuf));

	/* Connect to the host. */
	fd = connection_create(url_host(url), url_port(url));
	if (fd == -1) {
		strlcpy(https_errbuf, connection_error(), sizeof(https_errbuf));
		goto cleanup;
	}

	/* Create SSL structures and initiate TLS/SSL handshake. */
	ssl_ctx = SSL_CTX_new(SSLv23_client_method());
	ssl = SSL_new(ssl_ctx);
	if (ssl == NULL || ssl_ctx == NULL) {
		snprintf(https_errbuf, sizeof(https_errbuf), "SSL init: %s\n",
		    ERR_reason_error_string(ERR_get_error()));
		goto cleanup;
	}
	if (SSL_set_fd(ssl, fd) == 0) {
		snprintf(https_errbuf, sizeof(https_errbuf), "SSL set fd: %s\n",
		    ERR_reason_error_string(ERR_get_error()));
		goto cleanup;
	}
	rv = SSL_connect(ssl);
	switch (SSL_get_error(ssl, rv)) {
	case SSL_ERROR_NONE:
		break;
	case SSL_ERROR_SYSCALL:
		if (errno != 0) {
			strlcpy(https_errbuf, strerror(errno),
			    sizeof(https_errbuf));
			goto cleanup;
		}
		/* FALLTHROUGH */
	default:
		snprintf(https_errbuf, sizeof(https_errbuf), "SSL connect: %s",
		    ERR_reason_error_string(ERR_get_error()));
		goto cleanup;
	}	

	/* Send the HTTP request. */
	request = http_request(url, req);
	rv = SSL_write(ssl, request, strlen(request));
	switch (SSL_get_error(ssl, rv)) {
	case SSL_ERROR_NONE:
		break;
	case SSL_ERROR_SYSCALL:
		if (errno != 0) {
			strlcpy(https_errbuf, strerror(errno),
			    sizeof(https_errbuf));
			goto cleanup;
		}
		/* FALLTHROUGH */
	default:
		snprintf(https_errbuf, sizeof(https_errbuf), "SSL write: %s",
		    ERR_reason_error_string(ERR_get_error()));
		goto cleanup;
	}

	/* Receive all data. */
	buf = xmalloc(buflen);
	for (;;) {
		rv = SSL_read(ssl, buf + len, buflen - len - 1);
		switch (SSL_get_error(ssl, rv)) {
		case SSL_ERROR_NONE:
			break;
		case SSL_ERROR_SYSCALL:
			if (errno != 0) {
				strlcpy(https_errbuf, strerror(errno),
				    sizeof(https_errbuf));
				goto cleanup;
			}
			/* FALLTHROUGH */
		case SSL_ERROR_ZERO_RETURN:
			if (rv == 0)		/* EOF */
				goto cleanup;
			/* FALLTHROUGH */
		default:
			snprintf(https_errbuf, sizeof(https_errbuf),
			    "SSL read: %s",
			    ERR_reason_error_string(ERR_get_error()));
			xfree(buf);
			buf = NULL;
			goto cleanup;
		}
		buf[rv + len] = '\0';
		len += rv;

		if (len >= buflen - 1) {
			if (len > (BUFSIZ * BUFSIZ)) {
				strlcpy(https_errbuf, strerror(EMSGSIZE),
				    sizeof(https_errbuf));
				goto cleanup;
			}
			while (len >= buflen - 1)
				buflen *= 2;
			buf = xrealloc(buf, buflen);
		}
	}

 cleanup:
	/* Free used memory. */
	xfree(request);

	/* Cleanup SSL structures. */
	if (ssl != NULL) {
		SSL_shutdown(ssl);
		SSL_free(ssl);
	}
	if (ssl_ctx != NULL)
		SSL_CTX_free(ssl_ctx);
	
	/* Close the connection. */
	if (fd != -1)
		connection_close(fd);

	return (buf);
}

/*
 * https_geterror --
 *	Return the contents of the error buffer.
 */
static const char *
https_geterror(void)
{
	return (https_errbuf);
}
