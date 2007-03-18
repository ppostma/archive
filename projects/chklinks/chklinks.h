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

#ifndef _CHKLINKS_H_
#define _CHKLINKS_H_

#include <stdarg.h>	/* for va_list type */

/*
 * Name & version of the program.
 */
#define CHKLINKS_NAME		"chklinks"
#define CHKLINKS_VERSION	"20070318"

/*
 * Boolean symbolic constants.
 */
#define FALSE		0
#define TRUE		1

/* URL flags. */
#define URL_SCHEME	0x01
#define URL_HOST	0x02
#define URL_PORT	0x04
#define URL_AUTHORITY	(URL_HOST | URL_PORT)
#define URL_PATH	0x08
#define URL_COMPLETE	(URL_SCHEME | URL_AUTHORITY | URL_PATH)
#define URL_PATHBASE	0x10

/* URL pointer. */
typedef struct url *Url;

/* URL check pointer. */
typedef struct urlcheck *UrlCheck;

/* URL check status. */
enum check_status {
	NOT_CHECKED,
	INVALID_URL,
	CONNECT_FAIL,
	INVALID_RESPONSE,
	CHECK_ERROR,
	CHECK_SUCCESS,
	CHECK_SKIPPED
};

/* HTTP request types. */
enum http_request {
	REQUEST_GET,
	REQUEST_HEAD
};

/* URL format. */
enum url_format {
	URL_UNKNOWN,
	URL_GENERIC
};

/*
 * Function prototypes.
 */

/* connection.c */
int		  connection_create(const char *, const char *);
void		  connection_close(int);
char		 *connection_recv_data(int);
ssize_t		  connection_recv(int, char *, size_t);
ssize_t		  connection_send(int, const char *, size_t);
const char	 *connection_error(void);

/* ftp.c */
int		  ftp_test(UrlCheck, Url);

/* http.c */
void		 *http_create(void);
void		  http_destroy(void *);
int		  http_test(UrlCheck, Url);
char		**http_parse(UrlCheck, Url, size_t *);

char		 *http_request(Url, enum http_request);

void		  http_set_getdata(void *, char *(*)(Url, enum http_request));
void		  http_set_geterror(void *, const char *(*)(void));

/* https.c */
void		 *https_create(void);
void		  https_destroy(void *);
int		  https_test(UrlCheck, Url);
char		**https_parse(UrlCheck, Url, size_t *);

/* url.c */
Url		  url_parse(const char *, const char *);
char		 *url_asprintf(Url, int);
void		  url_free(Url);

const char	 *url_scheme(Url);
const char	 *url_host(Url);
const char	 *url_port(Url);
const char	 *url_path(Url);
const char	 *url_username(Url);
const char	 *url_password(Url);

/* urlcheck.c */
UrlCheck	  urlcheck_create(const char *);
void		  urlcheck_destroy(UrlCheck);

int		  urlcheck_test(UrlCheck);
char		**urlcheck_parse(UrlCheck, size_t *);

enum check_status urlcheck_status(UrlCheck);
const char	 *urlcheck_message(UrlCheck);
const char	 *urlcheck_url(UrlCheck);
void		 *urlcheck_data(UrlCheck);

void		  urlcheck_set_status(UrlCheck, enum check_status);
void		  urlcheck_set_message(UrlCheck, const char *, ...);
void		  urlcheck_set_rawurl(UrlCheck, const char *);
void		  urlcheck_set_retry(UrlCheck);

/* xalloc.c */
void		 *xmalloc(size_t);
void		 *xcalloc(size_t, size_t);
void		 *xrealloc(void *, size_t);
char		 *xstrdup(const char *);
void		  xstrdup2(char **, const char *);
char		 *xsprintf(const char *, ...);
char		 *xvsprintf(const char *, va_list);
void		  xfree(void *);

#endif /* _CHKLINKS_H_ */
