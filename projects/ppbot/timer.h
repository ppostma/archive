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

#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>

/* Initialize a timeval structure. */
#define TV_SET(tv, sec, usec)			\
	do {					\
		(tv)->tv_sec = sec;		\
		(tv)->tv_usec = usec;		\
	} while (/*CONSTCOND*/0)

/* Timer types. */
enum timer_type {
	TIMER_ONCE,
	TIMER_POLL
};

/* Prototypes. */
int	timer_schedule(struct timeval, void (*)(void *), void *,
		enum timer_type, const char *, ...);
int	timer_cancel(const char *, ...);
int	timer_suspend(const char *, ...);
int	timer_resume(const char *, ...);
int	timer_next(void);
void	timer_run_expired(void);
void	timer_correct(struct timeval *, struct timeval *);
void	timer_destroy_all(void);

#endif /* !_TIMER_H_ */
