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

/* Very simple replacement for vasprintf(). */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"

#ifdef WANT_VASPRINTF
int
vasprintf(char **str, const char *fmt, va_list ap)
{
	const char	*p;
	char		*buf, *strval;
	int		 ret;
	va_list		 ap2;
	size_t		 size;

	memcpy(&ap2, &ap, sizeof(va_list));

	size = strlen(fmt) + 1;
	p = fmt;
	while (*p != '\0') {
		if (*p++ == '%') {
			if (*p == 'l')
				p++;
			switch (*p) {
			case 'd':
			case 'u':
			case 'x':
				va_arg(ap2, int);
				size += 30;
				break;
			case 's':
				strval = va_arg(ap2, char *);
				size += strlen(strval);
				break;
			}
			p++;
		}
	}

	buf = malloc(size);
	if (buf == NULL) {
		*str = NULL;
		errno = ENOMEM;
		return (-1);
	}
	ret = vsnprintf(buf, size, fmt, ap);

	assert(ret >= 0 && (size_t)ret < size);

	*str = buf;
	return (ret);
}
#endif /* WANT_VASPRINTF */

#ifdef WANT_ASPRINTF
int
asprintf(char **str, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vasprintf(str, fmt, ap);
	va_end(ap);

	return (ret);
}
#endif /* WANT_ASPRINTF */
