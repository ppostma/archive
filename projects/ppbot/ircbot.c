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
#include <fcntl.h>
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
static void ircbot_print_usage(const char *);
static void ircbot_print_welcome(void);
static void ircbot_print_daemon(pid_t);
static void ircbot_print_bye(void);

static void ircbot_signal_handler(int);
static void ircbot_reconfigure(const char *);
static void ircbot_main_loop(const char *);

/*
 * Signal indicators.
 */
static volatile sig_atomic_t sig_int = FALSE;
static volatile sig_atomic_t sig_term = FALSE;
static volatile sig_atomic_t sig_hup = FALSE;

/*
 * main --
 *	Main entry point.
 */
int
main(int argc, char *argv[])
{
	const char *cfg = IRCBOT_CONFIG;
	char	   *progname;
	int	    ch, fd, debug = FALSE;
	pid_t	    child;

	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	/* Enable logging to stderr if started from a terminal. */
	if (isatty(STDERR_FILENO))
		set_logstderr(TRUE);

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
			ircbot_print_usage(progname);
		}
	}

	signal(SIGHUP, ircbot_signal_handler);
	signal(SIGINT, ircbot_signal_handler);
	signal(SIGTERM, ircbot_signal_handler);

	set_logdebug(debug);

	ircbot_print_welcome();

	if (config_parse(cfg) == FALSE) {
		ircbot_print_bye();
		exit(EXIT_FAILURE);
	}

	connections_verify();

	logfile_open();
	plugins_initialize();

	connections_initialize();

	/* Become a daemon when flag -d is not present. */
	if (!debug) {
		switch (child = fork()) {
		case -1:
			log_warnx("Unable to fork process");
			exit(EXIT_FAILURE);
		case 0:
			break;
		default:
			ircbot_print_daemon(child);
			_exit(0);
		}

		if (setsid() == -1) {
			log_warn("Unable to create new session");
			exit(EXIT_FAILURE);
		}

		fd = open("/dev/null", O_RDWR);
		if (fd != -1) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);

			if (fd > STDERR_FILENO)
				close(fd);
		}

		set_logstderr(FALSE);
	}

	ircbot_main_loop(cfg);

	ircbot_print_bye();

	connections_close();
	connections_destroy();

	timer_destroy_all();
	plugins_finalize();
	plugins_destroy();
	logfile_close();

	return (EXIT_SUCCESS);
}

/*
 * ircbot_print_usage --
 *	Print a message how to use the program and exit.
 */
static void
ircbot_print_usage(const char *progname)
{
	fprintf(stderr, "Usage: %s [-d] [-f configfile]\n", progname);
	exit(EXIT_FAILURE);
}

/*
 * ircbot_print_welcome --
 *	Print a welcome message.
 */
static void
ircbot_print_welcome(void)
{
	time_t	 t = time(NULL);
	char	*date;

	date = ctime(&t);
	date[strlen(date) - 1] = '\0';

	log_warnx("%s version %s started at %s", IRCBOT_NAME,
	    IRCBOT_VERSION, date);
}

/*
 * ircbot_print_daemon --
 *	Print a running as daemon message.
 */
static void
ircbot_print_daemon(pid_t child)
{
	time_t	 t = time(NULL);
	char	*date;

	date = ctime(&t);
	date[strlen(date) - 1] = '\0';

	log_warnx("%s version %s running in background at %s, pid %u",
	    IRCBOT_NAME, IRCBOT_VERSION, date, (unsigned int)child);
}

/*
 * ircbot_print_bye --
 *	Print a farewell message.
 */
static void
ircbot_print_bye(void)
{
	time_t	 t = time(NULL);
	char	*date;

	date = ctime(&t);
	date[strlen(date) - 1] = '\0';

	log_warnx("%s version %s shutting down at %s", IRCBOT_NAME,
	    IRCBOT_VERSION, date);
}

/*
 * ircbot_signal_handler --
 *	Catch signals and set the signal indicators.
 */
static void
ircbot_signal_handler(int sig)
{
	switch (sig) {
	case SIGTERM:
		sig_term = TRUE;
		break;
	case SIGINT:
		sig_int = TRUE;
		break;
	case SIGHUP:
		sig_hup = TRUE;
		break;
	}
}

/*
 * ircbot_reconfigure --
 *	Used to reconfigure the ircbot when already active.
 *
 *	Reads the configuration file, reloads plugins, starts new connections
 *	and joins new channels.
 */
static void
ircbot_reconfigure(const char *cfg)
{
	int rv;

	plugins_finalize();
	plugins_destroy();

	connections_destroy_dead();

	logfile_set(NULL);

	rv = config_parse(cfg);

	if (rv == TRUE) {
		logfile_open();

		connections_verify();

		plugins_initialize();

		connections_reinitialize();
		connections_join_channels();
	}
}

/*
 * ircbot_main_loop --
 *	The program main loop: wait with poll() for data from the IRC
 *	server(s), read/parse the data and handle the timers.
 */
static void
ircbot_main_loop(const char *cfg)
{
	struct connection *conn;
	struct pollfd	  *pfd;
	struct timeval	   now, base;
	int		   rv, timeout;
	unsigned int	   numfd, i;

	gettimeofday(&base, NULL);

	for (;;) {
		/* Run expired timers. */
		timer_run_expired();

		/* Reload the configuration if we have been asked to. */
		if (sig_hup) {
			sig_hup = FALSE;
			ircbot_reconfigure(cfg);
			log_warnx("The configuration has been reloaded.");
		}

		/* Shut down when receiving INT/TERM signal. */
		if (sig_int) {
			log_warnx("Received SIGINT, shutting down.");
			break;
		}
		if (sig_term) {
			log_warnx("Received SIGTERM, shutting down.");
			break;
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
