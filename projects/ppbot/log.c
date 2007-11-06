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
 * Logging buffer sizes.
 */
#define LOG_BUFSIZE		512
#define LOG_BUFSIZE_CONN	64
#define LOG_BUFSIZE_PLUGIN	64

/*
 * Global variables --
 *	Log file pointer, log file, log stderr/debug indicators.
 */
static FILE *log_fp = NULL;
static char *log_file = NULL;
static int   log_stderr = FALSE;
static int   log_ndebug = FALSE;

/*
 * set_logdebug --
 *	Enable/disable debugging messages (via log_debug()).
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
	xstrdup2(&log_file, file);
}

/*
 * logfile_open --
 *	Open the main log file if it's set.  Close it first when already open.
 */
void
logfile_open(void)
{
	logfile_close();

	if (log_file != NULL) {
		log_fp = fopen(log_file, "a");
		if (log_fp == NULL)
			log_msg(LOG_WARNING,
			    "Unable to open the log file '%s'", log_file);
	}
}

/*
 * logfile_close --
 *	Close the main log file when opened.
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
logit(FILE *fp, int flags, const char *prefix, const char *buf)
{
	time_t	   t = time(NULL);
	struct tm *tp = localtime(&t);

	fprintf(fp, "[%02d:%02d] ", tp->tm_hour, tp->tm_min);
	if (prefix != NULL) {
		fprintf(fp, "%s ", prefix);
	}
	fprintf(fp, "%s", buf);
	if ((flags & LOG_WARNING)) {
		fprintf(fp, ": %s", strerror(errno));
	}
	fprintf(fp, "%s", "\n");
	fflush(fp);
}

/*
 * vlog_common --
 *	Log to stderr and/or a file.
 */
static void
vlog_common(int flags, const char *prefix, const char *fmt, va_list ap)
{
	char buf[LOG_BUFSIZE];

	if ((flags & LOG_DEBUG) && !log_ndebug)
		return;

	vsnprintf(buf, sizeof(buf), fmt, ap);

	if (log_stderr) {
		logit(stderr, flags, prefix, buf);
	}
	if (log_fp != NULL) {
		logit(log_fp, flags, prefix, buf);
	}
}

/*
 * log_msg --
 *	Log a message.
 */
void
log_msg(int flags, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog_msg(flags, fmt, ap);
	va_end(ap);
}

/*
 * vlog_msg --
 *	Log a message (arguments are already captured).
 */
void
vlog_msg(int flags, const char *fmt, va_list ap)
{
	vlog_common(flags, NULL, fmt, ap);
}

/*
 * log_plugin --
 *	Log a message, prefix the plugin.
 */
void
log_plugin(int flags, Plugin p, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog_plugin(flags, p, fmt, ap);
	va_end(ap);
}

/*
 * vlog_plugin --
 *	Log a message, prefix the plugin (arguments are already captured).
 */
void
vlog_plugin(int flags, Plugin p, const char *fmt, va_list ap)
{
	char buf[LOG_BUFSIZE_PLUGIN];

	snprintf(buf, sizeof(buf), "*%s*", plugin_id(p));

	vlog_common(flags, buf, fmt, ap);
}

/*
 * log_conn --
 *	Log a message, prefix the connection.
 */
void
log_conn(int flags, Connection conn, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog_conn(flags, conn, fmt, ap);
	va_end(ap);
}

/*
 * vlog_conn --
 *	Log a message, prefix the connection (arguments are already captured).
 */
void
vlog_conn(int flags, Connection conn, const char *fmt, va_list ap)
{
	char buf[LOG_BUFSIZE_CONN];

	snprintf(buf, sizeof(buf), "(%s)", connection_id(conn));

	vlog_common(flags, buf, fmt, ap);
}
