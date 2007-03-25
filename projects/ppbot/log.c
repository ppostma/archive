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

#include <sys/types.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ircbot.h"

/*
 * Global variables --
 *	Log file pointer, log file, log stderr/debug indicators.
 */
static FILE	*log_fp = NULL;
static char	*log_file = NULL;
static int	 log_stderr = FALSE;
static int	 log_ndebug = FALSE;

/*
 * set_logdebug --
 *	Turn on/off log_debug() functions.
 */
void
set_logdebug(int ndebug)
{
	log_ndebug = ndebug;
}

/*
 * set_logstderr --
 *	Turn on/off logging to stderr.
 */
void
set_logstderr(int nlog)
{
	log_stderr = nlog;
}

/*
 * logfile_set --
 *	Set the main log file.
 */
void
logfile_set(const char *file)
{
	if (log_file != NULL) {
		logfile_close();
	}
	xstrdup2(&log_file, file);
}

/*
 * logfile_open --
 *	Open the main log file if it's set and not already open.
 */
void
logfile_open(void)
{
	if (log_file == NULL || log_fp != NULL)
		return;

	log_fp = fopen(log_file, "a");
	if (log_fp == NULL)
		log_warn("Unable to open the main log file");
}

/*
 * logfile_close --
 *	Close the main log file if it's open.
 */
void
logfile_close(void)
{
	if (log_fp != NULL) {
		fclose(log_fp);
		log_fp = NULL;
	}
}

/*
 * logit --
 *	Log to the specified FILE pointer.
 */
static void
logit(FILE *fp, const char *buf, int error)
{
	time_t	   t = time(NULL);
	struct tm *tp = localtime(&t);

	fprintf(fp, "%02d:%02d %s", tp->tm_hour, tp->tm_min, buf);
	if (error) {
		fprintf(fp, ": %s", strerror(errno));
	}
	fprintf(fp, "%s", "\n");
	fflush(fp);
}

/*
 * vlog --
 *	Log a string to stderr and/or a file.
 */
static void
vlog(const char *fmt, va_list ap, int error)
{
	char buf[BUFSIZ];

	vsnprintf(buf, sizeof(buf), fmt, ap);

	if (log_stderr) {
		logit(stderr, buf, error);
	}
	if (log_fp != NULL) {
		logit(log_fp, buf, error);
	}
}

/*
 * log_debug --
 *	Log only when debug is enabled.
 */
void
log_debug(const char *fmt, ...)
{
	va_list	ap;

	if (log_ndebug) {
		va_start(ap, fmt);
		vlog(fmt, ap, FALSE);
		va_end(ap);
	}
}

/*
 * log_warnx --
 *	Log in warnx() style.
 */
void
log_warnx(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vlog(fmt, ap, FALSE);
	va_end(ap);
}

/*
 * log_warn --
 *	Log in warn() style (outputs errno as string).
 */
void
log_warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog(fmt, ap, TRUE);
	va_end(ap);
}
