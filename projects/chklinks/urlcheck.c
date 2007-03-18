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

#include <stddef.h>
#include <string.h>

#include "compat/compat.h"
#include "chklinks.h"

/*
 * URL check object.
 */
struct urlcheck {
	Url			 url;
	char			*rawurl;
	char			*origurl;
	int			 retry;
	size_t			 tries;
	size_t			 previous;
	enum check_status	 status;
	char			*message;
	void			*data;
};

/*
 * Table with schemes/protocols where we have checks for.
 */
static const struct {
	const char	   *name;
	void		 *(*create)(void);
	void		  (*destroy)(void *);
	int		  (*test)(UrlCheck, Url);
	char		**(*parse)(UrlCheck, Url, size_t *);
} checks[] = {
	{ "http",  http_create,  http_destroy,  http_test,  http_parse  },
	{ "https", https_create, https_destroy, https_test, https_parse },
	{ "ftp",   NULL,         NULL,          ftp_test,   NULL        }
};

/*
 * urlcheck_create --
 *	Create an UrlCheck instance.
 */
UrlCheck
urlcheck_create(const char *url)
{
	UrlCheck check;

	check = xcalloc(1, sizeof(struct urlcheck));
	check->rawurl = xstrdup(url);
	check->origurl = xstrdup(url);
	check->status = NOT_CHECKED;
	check->retry = FALSE;
	check->previous = ~0;

	return (check);
}

/*
 * urlcheck_destroy --
 *	Destroy an instance of UrlCheck.
 */
void
urlcheck_destroy(UrlCheck check)
{
	if (check->url != NULL)
		url_free(check->url);

	xfree(check->message);
	xfree(check->origurl);
	xfree(check->rawurl);
	xfree(check);
}

/*
 * urlcheck_test --
 *	Test the given UrlCheck object.  Returns TRUE if the test was
 *	successful and FALSE otherwise.
 */
int
urlcheck_test(UrlCheck check)
{
	size_t	i;
	int	rv, found;

 again:
	/* If we tried it 5 times then give up. */
	if (check->tries >= 5) {
		check->status = CHECK_ERROR;
		check->message = xstrdup("Too many redirects");
		return (FALSE);
	}

	/* Parse the URL. */
	check->url = url_parse(check->rawurl, check->origurl);
	if (check->url == NULL) {
		check->status = INVALID_URL;
		check->message = xstrdup("Invalid URL");
		return (FALSE);
	}

	/* Search for the test function. */
	found = FALSE;
	for (i = 0; i < sizeof(checks) / sizeof(checks[0]); i++) {
		if (strcmp(url_scheme(check->url), checks[i].name) == 0) {
			found = TRUE;
			break;
		}
	}

	if (!found || checks[i].test == NULL) {
		check->status = CHECK_SKIPPED;
		check->message = xstrdup("Scheme not supported");
		return (FALSE);
	}

	/* Cleanup data if redirected to other scheme type. */
	if (check->previous != i && check->data != NULL) {
		(*checks[check->previous].destroy)(check->data);
		check->data = NULL;
	}

	/* Create data object if not already created. */
	if (check->data == NULL && (*checks[i].create) != NULL)
		check->data = (*checks[i].create)();

	/* Call the test function. */
	rv = (*checks[i].test)(check, check->url);
	if (rv == FALSE && check->retry == TRUE) {
		check->tries++;
		check->retry = FALSE;
		check->previous = i;
		url_free(check->url);
		goto again;
	}

	/* Destroy any data. */
	if (check->data != NULL && (*checks[i].destroy) != NULL)
		(*checks[i].destroy)(check->data);

	return (rv);
}

/*
 * urlcheck_parse --
 *	Parse the links for the given UrlCheck object.
 */
char **
urlcheck_parse(UrlCheck check, size_t *count)
{
	size_t	  i;
	int	  found;
	char	**result;

 again:
	/* If we tried it 5 times then give up. */
	if (check->tries >= 5) {
		check->status = CHECK_ERROR;
		check->message = xstrdup("Too many redirects");
		return (FALSE);
	}

	/* Parse the URL. */
	check->url = url_parse(check->rawurl, check->origurl);
	if (check->url == NULL) {
		check->status = INVALID_URL;
		check->message = xstrdup("Invalid URL");
		return (FALSE);
	}

	/* Search for the parse function. */
	found = FALSE;
	for (i = 0; i < sizeof(checks) / sizeof(checks[0]); i++) {
		if (strcmp(url_scheme(check->url), checks[i].name) == 0) {
			found = TRUE;
			break;
		}
	}

	if (!found || checks[i].parse == NULL) {
		check->status = CHECK_SKIPPED;
		check->message = xstrdup("Scheme not supported");
		return (NULL);
	}

	/* Cleanup data if redirected to other scheme type. */
	if (check->previous != i && check->data != NULL) {
		(*checks[check->previous].destroy)(check->data);
		check->data = NULL;
	}

	/* Create data object if not already created. */
	if (check->data == NULL && (*checks[i].create) != NULL)
		check->data = (*checks[i].create)();

	/* Call the parse function. */
	result = (*checks[i].parse)(check, check->url, count);
	if (result == NULL) {
		if (check->retry == TRUE) {
			check->tries++;
			check->retry = FALSE;
			check->previous = i;
			url_free(check->url);
			goto again;
		}
		*count = 0;
	}

	/* Destroy any data. */
	if (check->data != NULL && (*checks[i].destroy) != NULL)
		(*checks[i].destroy)(check->data);

	return (result);
}

/*
 * urlcheck_status --
 *	Accessor function for the 'status' member.
 */
enum check_status
urlcheck_status(UrlCheck check)
{
	return (check->status);
}

/*
 * urlcheck_message --
 *	Accessor function for the 'message' member.
 */
const char *
urlcheck_message(UrlCheck check)
{
	return (check->message);
}

/*
 * urlcheck_url --
 *	Accessor function for the 'origurl' member.
 */
const char *
urlcheck_url(UrlCheck check)
{
	return (check->origurl);
}

/*
 * urlcheck_data --
 *	Accessor function for the 'data' member.
 */
void *
urlcheck_data(UrlCheck check)
{
	return (check->data);
}

/*
 * urlcheck_set_status --
 *	Setter for the 'status' member.
 */
void
urlcheck_set_status(UrlCheck check, enum check_status status)
{
	check->status = status;
}

/*
 * urlcheck_set_message --
 *	Setter for the 'message' member.
 */
void
urlcheck_set_message(UrlCheck check, const char *fmt, ...)
{
	va_list	 ap;
	char	*message;

	va_start(ap, fmt);
	message = xvsprintf(fmt, ap);
	va_end(ap);

	check->message = message;
}

/*
 * urlcheck_set_rawurl --
 *	Setter for the 'rawurl' member.
 */
void
urlcheck_set_rawurl(UrlCheck check, const char *rawurl)
{
	check->rawurl = xstrdup(rawurl);
}

/*
 * urlcheck_set_retry --
 *	Setter for the 'retry' member.
 */
void
urlcheck_set_retry(UrlCheck check)
{
	check->retry = TRUE;
}
