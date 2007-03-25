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
#include <sys/time.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat/compat.h"
#include "queue.h"
#include "timer.h"

/*
 * Flags for timer lookups.
 */
#define TIMER_ACTIVE		0x01
#define TIMER_SUSPENDED		0x02

/*
 * Timer object.
 */
struct timer {
	struct timeval		 timeout;
	struct timeval		 expires;
	char			*name;
	enum timer_type		 type;
	void			(*callback)(void *);
	void			*arg;
	TAILQ_ENTRY(timer)	 link;
};

/*
 * List with active timers.
 */
static TAILQ_HEAD(, timer) timers = TAILQ_HEAD_INITIALIZER(timers);

/*
 * List with suspended timers.
 */
static TAILQ_HEAD(, timer) suspended_timers =
    TAILQ_HEAD_INITIALIZER(suspended_timers);

/*
 * timer_lookup --
 *	Look up a timer by name in the active and/or suspended list.
 */
static struct timer *
timer_lookup(const char *name, int flags)
{
	struct timer *t;

	if (flags & TIMER_ACTIVE) {
		TAILQ_FOREACH(t, &timers, link)
			if (strcmp(t->name, name) == 0)
				return (t);
	}

	if (flags & TIMER_SUSPENDED) {
		TAILQ_FOREACH(t, &suspended_timers, link)
			if (strcmp(t->name, name) == 0)
				return (t);
	}

	return (NULL);
}

/*
 * timer_add --
 *	Add a timer to the timers list.
 */
static void
timer_add(struct timer *t)
{
	struct timer *n;

	/*
	 * Keep the list ordered so that the first element is always
	 * the first timer that expires.
	 */
	for (n = TAILQ_FIRST(&timers);
	     n != NULL && timercmp(&t->expires, &n->expires, >=);
	     n = TAILQ_NEXT(n, link))
		continue;
	if (n == NULL)
		TAILQ_INSERT_TAIL(&timers, t, link);
	else
		TAILQ_INSERT_BEFORE(n, t, link);
}

/*
 * timer_remove --
 *	Remove a timer from the timers list.
 */
static void
timer_remove(struct timer *t)
{
	TAILQ_REMOVE(&timers, t, link);
}

/*
 * timer_add_suspended --
 *	Add a timer to the suspended timers list.
 */
static void
timer_add_suspended(struct timer *t)
{
	TAILQ_INSERT_HEAD(&suspended_timers, t, link);
}

/*
 * timer_remove_suspended --
 *	Remove a timer from the suspended timers list.
 */
static void
timer_remove_suspended(struct timer *t)
{
	TAILQ_REMOVE(&suspended_timers, t, link);
}

/*
 * timer_destroy --
 *	Destroy a timer object.
 */
static void
timer_destroy(struct timer *t)
{
	free(t->name);
	free(t);
}

/*
 * timer_tvadd --
 *	Add 'tv1' and 'tv2' and put the result in 'tv'.
 */
static void
timer_tvadd(struct timeval *tv1, struct timeval *tv2, struct timeval *tv)
{
	tv->tv_sec = tv1->tv_sec + tv2->tv_sec;
	tv->tv_usec = tv1->tv_usec + tv2->tv_usec;

	if (tv->tv_usec >= 1000000) {
		tv->tv_sec++;
		tv->tv_usec -= 1000000;
	}
}

/*
 * timer_tvsub --
 *	Subtract 'tv1' and 'tv2' and put the result in 'tv'.
 */
static void
timer_tvsub(struct timeval *tv1, struct timeval *tv2, struct timeval *tv)
{
	tv->tv_sec = tv1->tv_sec - tv2->tv_sec;
	tv->tv_usec = tv1->tv_usec - tv2->tv_usec;

	if (tv->tv_usec < 0) {
		tv->tv_sec--;
		tv->tv_usec += 1000000;
	}
}

/*
 * timer_set_expire --
 *	Set the expiration time of the timer.
 */
static void
timer_set_expire(struct timer *t)
{
	struct timeval now;

	gettimeofday(&now, NULL);
	timer_tvadd(&now, &t->timeout, &t->expires);
}

/*
 * timer_schedule --
 *	Schedule a new timer.
 */
int
timer_schedule(struct timeval timeout, void (*callback)(void *),
    void *arg, enum timer_type type, const char *fmt, ...)
{
	struct timer	*t;
	va_list		 ap;
	char		*name;
	int		 rv;

	va_start(ap, fmt);
	rv = vasprintf(&name, fmt, ap);
	va_end(ap);

	if (rv == -1 || name == NULL)
		return (ENOMEM);

	if (timer_lookup(name, TIMER_ACTIVE|TIMER_SUSPENDED) != NULL) {
		free(name);
		return (EEXIST);
	}

	t = calloc(1, sizeof(struct timer));
	if (t == NULL) {
		free(name);
		return (ENOMEM);
	}

	t->callback = callback;
	t->timeout = timeout;
	t->name = name;
	t->arg = arg;
	t->type = type;

	timer_set_expire(t);
	timer_add(t);

	return (0);
}

/*
 * timer_cancel --
 *	Cancel an active or suspended timer.
 */
int
timer_cancel(const char *fmt, ...)
{
	struct timer	*t;
	char		*name = NULL;
	va_list		 ap;
	int		 rv, error = 0;

	va_start(ap, fmt);
	rv = vasprintf(&name, fmt, ap);
	va_end(ap);

	if (rv == -1 || name == NULL)
		return (ENOMEM);

	t = timer_lookup(name, TIMER_ACTIVE);
	if (t != NULL) {
		timer_remove(t);
	} else {
		t = timer_lookup(name, TIMER_SUSPENDED);
		if (t != NULL)
			timer_remove_suspended(t);
	}
	if (t != NULL) {
		timer_destroy(t);
	} else {
		error = ENOENT;
	}

	free(name);

	return (error);
}

/*
 * timer_suspend --
 *	Suspend a timer.
 */
int
timer_suspend(const char *fmt, ...)
{
	struct timer	*t;
	char		*name = NULL;
	va_list		 ap;
	int		 rv, error = 0;

	va_start(ap, fmt);
	rv = vasprintf(&name, fmt, ap);
	va_end(ap);

	if (rv == -1 || name == NULL)
		return (ENOMEM);

	t = timer_lookup(name, TIMER_ACTIVE);
	if (t != NULL) {
		timer_remove(t);
		timer_add_suspended(t);
	} else {
		error = ENOENT;
	}

	free(name);

	return (error);
}

/*
 * timer_resume --
 *	Resume a suspended timer.
 */
int
timer_resume(const char *fmt, ...)
{
	struct timer	*t;
	char		*name = NULL;
	va_list		 ap;
	int		 rv, error = 0;

	va_start(ap, fmt);
	rv = vasprintf(&name, fmt, ap);
	va_end(ap);

	if (rv == -1 || name == NULL)
		return (ENOMEM);

	t = timer_lookup(name, TIMER_SUSPENDED);
	if (t != NULL) {
		timer_remove_suspended(t);
		timer_set_expire(t);
		timer_add(t);
	} else {
		error = ENOENT;
	}

	free(name);

	return (error);
}

/*
 * timer_next --
 *	Return # milliseconds when the next timer expires.
 *	This function will return 0 when there are no timers active.
 */
int
timer_next(void)
{
	struct timer	*t;
	struct timeval	 timeout, now;
	int		 rv;

	t = TAILQ_FIRST(&timers);
	if (t == NULL)
		return (0);

	gettimeofday(&now, NULL);
	timer_tvsub(&t->expires, &now, &timeout);

	/* Transform to milliseconds. */
	rv = (timeout.tv_sec * 1000) + (timeout.tv_usec / 1000);

	/*
	 * If rv is equal to or smaller than zero, then the timer is
	 * already expired.  Return 1 to indicate that the timer should
	 * be executed as soon as possible.
	 */
	return (rv > 0 ? rv : 1);
}

/*
 * timer_run_expired --
 *	Run the callback functions from expired timers.
 */
void
timer_run_expired(void)
{
	struct timer	*t, *n;
	struct timeval	 now;

	for (t = TAILQ_FIRST(&timers); t != NULL; t = n) {
		n = TAILQ_NEXT(t, link);
		gettimeofday(&now, NULL);
		if (timercmp(&t->expires, &now, >))
			break;
		(*t->callback)(t->arg);

		switch (t->type) {
		case TIMER_ONCE:
			timer_remove(t);
			timer_destroy(t);
			break;
		case TIMER_POLL:
			if (timer_lookup(t->name, TIMER_ACTIVE) != NULL) {
				timer_remove(t);
				timer_set_expire(t);
				timer_add(t);
			}
			break;
		}
	}
}

/*
 * timer_correct --
 *	Correct the expiration time of all timers.  This should be done
 *	when we've detected that the time is running backwards.
 */
void
timer_correct(struct timeval *base, struct timeval *now)
{
	struct timeval	 off;
	struct timer	*t;

	timer_tvsub(base, now, &off);

	TAILQ_FOREACH(t, &timers, link) {
		timer_tvsub(&t->expires, &off, &t->expires);
	}
}

/*
 * timer_destroy_all --
 *	Destroy all timers.
 */
void
timer_destroy_all(void)
{
	struct timer *t;

	while ((t = TAILQ_FIRST(&timers)) != NULL) {
		timer_remove(t);
		timer_destroy(t);
	}
}
