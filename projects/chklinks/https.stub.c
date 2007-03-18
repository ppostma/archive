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

#include "compat/compat.h"
#include "chklinks.h"

/*
 * https_create --
 *	Stub for 'https_create'.
 */
void *
https_create(void)
{
	return (NULL);
}

/*
 * https_destroy --
 *	Stub for 'https_destroy'.
 */
void
https_destroy(void *data)
{
}

/*
 * https_test --
 *	Stub for 'https_test'.
 */
int
https_test(UrlCheck check, Url url)
{
	urlcheck_set_status(check, CHECK_SKIPPED);
	urlcheck_set_message(check, "Scheme not supported");
	return (FALSE);
}

/*
 * https_parse --
 *	Stub for 'https_parse'.
 */
char **
https_parse(UrlCheck check, Url url, size_t *count)
{
	urlcheck_set_status(check, CHECK_SKIPPED);
	urlcheck_set_message(check, "Scheme not supported");
	return (NULL);
}
