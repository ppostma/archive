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
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compat/compat.h"
#include "ircbot.h"
#include "timer.h"

/*
 * Local function prototypes.
 */
static void usage(const char *);
static void signal_handler(int);
static void reconfigure(const char *);
static void main_loop(const char *);

/*
 * Signal indicators.  quit - INT/TERM, reconfig - HUP.
 */
static volatile sig_atomic_t quit = FALSE;
static volatile sig_atomic_t reconfig = FALSE;

/*
 * main --
 *	Main entry point.
 */
int
main(int argc, char *argv[])
{
	const char *cfg = IRCBOT_CONFIG;
	char	   *progname;
	int	    ch, debug = FALSE;

	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	/* Parse command line arguments. */
	while ((ch = getopt(argc, argv, "df:")) != -1) {
		switch (ch) {
		case 'd':
			debug = TRUE;
			break;
		case 'f':
			cfg = optarg;
			break;
		default:
			usage(progname);
		}
	}

	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	set_logdebug(debug);
	set_logstderr(TRUE);

	if (config_parse(cfg) == FALSE)
		exit(EXIT_FAILURE);

	connections_verify();

	logfile_open();
	plugins_initialize();

	connections_initialize();

	if (!debug) {
		if (daemon(1, 0) == -1) {
			log_warn("Unable to daemonize");
			exit(EXIT_FAILURE);
		}
		set_logstderr(FALSE);
	}

	main_loop(cfg);

	connections_close();
	connections_destroy();

	timer_destroy_all();
	plugins_finalize();
	plugins_destroy();
	logfile_close();

	return (EXIT_SUCCESS);
}

/*
 * usage --
 *	Print a message how to use the program and exit.
 */
static void
usage(const char *progname)
{
	fprintf(stderr, "Usage: %s [-d] [-f configfile]\n", progname);
	exit(EXIT_FAILURE);
}

/*
 * signal_handler --
 *	Catch signals and set the signal indicators.
 */
static void
signal_handler(int sig)
{
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		quit = TRUE;
		break;
	case SIGHUP:
		reconfig = TRUE;
		break;
	}
}

/*
 * reconfigure --
 *	Re-read the configuration file.  Can load new plugins, start new
 *	connections, join new channels, etc...
 *	This function is executed when receiving the HUP signal.
 */
static void
reconfigure(const char *cfg)
{
	plugins_finalize();
	plugins_destroy();
	logfile_close();

	connections_destroy_dead();

	config_parse(cfg);

	connections_verify();

	logfile_open();
	plugins_initialize();

	connections_reinitialize();
	connections_join_channels();
}

/*
 * main_loop --
 *	The program main loop: wait with poll() for data from the IRC
 *	server(s), read/parse the data and handle the timers.
 */
static void
main_loop(const char *cfg)
{
	struct connection *conn;
	struct pollfd	  *pfd;
	struct timeval	   now, base;
	int		   rv, timeout;
	unsigned int	   numfd, i;

	gettimeofday(&base, NULL);

	while (quit == FALSE) {
		/* Run expired timers. */
		timer_run_expired();

		/* Reload the configuration if we have been asked to. */
		if (reconfig) {
			reconfig = FALSE;
			reconfigure(cfg);
			log_warnx("The configuration has been reloaded.");
		}

		/* Correct the timers when running backwards. */
		gettimeofday(&now, NULL);
		if (timercmp(&now, &base, <)) {
			log_debug("Time is running backwards, corrected.");
			timer_correct(&base, &now);
		}
		base = now;

		/* Wait with poll for events, or a timer timeout. */
		pfd = connections_pollfds(&numfd);
		timeout = timer_next();
		rv = poll(pfd, numfd, timeout ? timeout : -1);
		if (rv == -1) {
			if (errno == EINTR)
				continue;
			log_warn("poll error");
			break;
		} else if (rv == 0)
			continue;

		/* Process any events. */
		for (i = 0; i < numfd; i++) {
			if (pfd[i].revents & (POLLIN|POLLERR)) {
				conn = connection_lookup(pfd[i].fd);
				if (conn == NULL)
					continue;
				if (connection_read(conn) == FALSE) {
					log_warnx("[%s] Connection lost to %s.",
					    connection_id(conn),
					    connection_address(conn));
					connection_close(conn, TRUE);
				}
			}
		}
	}
}
