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

#include <stdio.h>

#include "feed.h"
#include "ircbot.h"

static Plugin plugin;

/*
 * feed_logger_initialize --
 *	Initialize the logger.
 */
void
feed_logger_initialize(Plugin p)
{
	plugin = p;
}

/*
 * feed_log_debug --
 *	Log a debug message.
 */
void
feed_log_debug(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog_plugin(LOG_DEBUG, plugin, fmt, ap);
	va_end(ap);
}

/*
 * feed_log_info --
 *	Log an informational message.
 */
void
feed_log_info(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog_plugin(LOG_INFO, plugin, fmt, ap);
	va_end(ap);
}

/*
 * feed_log_warning --
 *	Log a warning message.
 */
void
feed_log_warning(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog_plugin(LOG_WARNING, plugin, fmt, ap);
	va_end(ap);
}
