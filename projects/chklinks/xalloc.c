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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat/compat.h"
#include "chklinks.h"

/*
 * xmalloc --
 *	Same as malloc() but exits on failure.
 */
void *
xmalloc(size_t size)
{
	void *p;

	if ((p = malloc(size)) == NULL) {
		fprintf(stderr, "Unable to allocate %lu bytes: %s\n",
		    (unsigned long)size, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return (p);
}

/*
 * xcalloc --
 *	Same as calloc() but exits on failure.
 */
void *
xcalloc(size_t number, size_t size)
{
	void *p;

	if ((p = calloc(number, size)) == NULL) {
		fprintf(stderr, "Unable to allocate %lu bytes: %s\n",
		    (unsigned long)(number * size), strerror(errno));
		exit(EXIT_FAILURE);
	}
	return (p);
}

/*
 * xrealloc --
 *	Same as realloc() but exits on failure.
 */
void *
xrealloc(void *ptr, size_t size)
{
	void *nptr;

	if ((nptr = realloc(ptr, size)) == NULL) {
		fprintf(stderr, "Unable to reallocate %lu bytes: %s\n",
		    (unsigned long)size, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return (nptr);
}

/*
 * xstrdup --
 *	Same as strdup() but exists on failure.
 *	Returns NULL when 'str' is also NULL.
 */
char *
xstrdup(const char *str)
{
	size_t	 size;
	char	*ptr;

	if (str == NULL)
		return (NULL);

	size = strlen(str) + 1;
	ptr = xmalloc(size);

	strlcpy(ptr, str, size);

	return (ptr);
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
 *	Same as asprintf() but exits on failure.
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
 *	Same as vasprintf() but exists on failure.
 */
char *
xvsprintf(const char *fmt, va_list ap)
{
	char	*ptr = NULL;
	int	 rv;

	rv = vasprintf(&ptr, fmt, ap);
	if (rv == -1 || ptr == NULL) {
		fprintf(stderr, "Unable to format string: %s\n",
		    strerror(errno));
		exit(EXIT_FAILURE);
	}
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
