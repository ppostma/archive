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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compat/compat.h"
#include "ircbot.h"

/*
 * xmalloc --
 *	malloc() that never returns NULL but keeps trying on error.
 */
void *
xmalloc(size_t size)
{
	void	*p;
	int	 warned = FALSE;

	do {
		p = malloc(size);
		if (p == NULL && !warned++)
			log_msg(LOG_WARNING, "Unable to allocate %lu bytes",
			    (unsigned long)size);
		if (p == NULL)
			sleep(1);
	} while (p == NULL);

	return (p);
}

/*
 * xcalloc --
 *	calloc() that never returns NULL but keeps trying on error.
 */
void *
xcalloc(size_t number, size_t size)
{
	void	*p;
	int	 warned = FALSE;

	do {
		p = calloc(number, size);
		if (p == NULL && !warned++)
			log_msg(LOG_WARNING, "Unable to allocate %lu bytes",
			    (unsigned long)(number * size));
		if (p == NULL)
			sleep(1);
	} while (p == NULL);

	return (p);
}

/*
 * xrealloc --
 *	realloc() that never returns NULL but keeps trying on error.
 */
void *
xrealloc(void *ptr, size_t size)
{
	void	*p;
	int	 warned = FALSE;

	if (ptr == NULL)
		return (xmalloc(size));

	do {
		p = realloc(ptr, size);
		if (p == NULL && !warned++)
			log_msg(LOG_WARNING, "Unable to reallocate %lu bytes",
			    (unsigned long)size);
		if (p == NULL)
			sleep(1);
	} while (p == NULL);

	return (p);
}

/*
 * xstrdup --
 *	strdup() that keeps trying on error.
 *	Returns NULL when 'str' is also NULL.
 */
char *
xstrdup(const char *str)
{
	size_t	 size;
	char	*nptr;

	if (str == NULL)
		return (NULL);

	size = strlen(str) + 1;
	nptr = xmalloc(size);

	strlcpy(nptr, str, size);

	return (nptr);
}

/*
 * xstrdup2 --
 *	Like strdup() but can resize the destination pointer.
 *	Returns NULL when 'str' is also NULL.
 */
void
xstrdup2(char **dst, const char *str)
{
	size_t	 size;
	char	*nptr = NULL;

	if (str == NULL) {
		if (*dst != NULL)
			free(*dst);
	} else {
		size = strlen(str) + 1;
		nptr = xrealloc(*dst, size);

		strlcpy(nptr, str, size);
	}

	*dst = nptr;
}

/*
 * xsprintf --
 *	asprintf() that never returns NULL but keeps trying on error.
 */
char *
xsprintf(const char *fmt, ...)
{
	va_list	 ap;
	char	*ptr;

	va_start(ap, fmt);
	ptr = xvsprintf(fmt, ap);
	va_end(ap);

	return (ptr);
}

/*
 * xvsprintf --
 *	vasprintf() that never returns NULL but keeps trying on error.
 */
char *
xvsprintf(const char *fmt, va_list ap)
{
	char	*ptr = NULL;
	int	 rv, warned = FALSE;

	do {
		rv = vasprintf(&ptr, fmt, ap);
		if (rv == -1 && !warned++)
			log_msg(LOG_WARNING, "Unable to format string");
		if (rv == -1)
			sleep(1);
	} while (rv == -1);

	return (ptr);
}

/*
 * xfree --
 *	free() that checks if the pointer is non-NULL before freeing.
 */
void
xfree(void *ptr)
{
	if (ptr != NULL)
		free(ptr);
}
