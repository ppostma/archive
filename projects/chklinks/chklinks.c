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

#include <sys/types.h>
#include <sys/time.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compat/compat.h"
#include "chklinks.h"
#include "queue.h"
#include "tree.h"

#define FLAG_VERBOSE	0x01
#define FLAG_QUIET	0x02

/*
 * Url input object.
 */
struct UrlLinkInput {
	UrlCheck			input;
	TAILQ_ENTRY(UrlLinkInput)	link;
};

/*
 * Url check object.
 */
struct UrlLinkCheck {
	UrlCheck			link;
	TAILQ_HEAD(, UrlLinkFrom)	fromhead;
	RB_ENTRY(UrlLinkCheck)		check;
	TAILQ_ENTRY(UrlLinkCheck)	broken;
};

/*
 * Url from object.
 */
struct UrlLinkFrom {
	UrlCheck			input;
	TAILQ_ENTRY(UrlLinkFrom)	from;
};

/*
 * Url check stats object.
 */
struct UrlCheckStats {
	size_t		input;
	size_t		total;
	size_t		broken;
	size_t		skipped;
};

/*
 * Compare function, used by the red-black tree.
 */
static int
UrlLinkCheckCompare(const struct UrlLinkCheck *a, const struct UrlLinkCheck *b)
{
	return (strcasecmp(urlcheck_url(a->link), urlcheck_url(b->link)));
}

/*
 * Lists/tree for the input links, check links and broken links.
 */
TAILQ_HEAD(, UrlLinkInput) UrlLinkInputHead =
    TAILQ_HEAD_INITIALIZER(UrlLinkInputHead);

TAILQ_HEAD(, UrlLinkCheck) UrlLinkBrokenHead =
    TAILQ_HEAD_INITIALIZER(UrlLinkBrokenHead);

RB_HEAD(UrlCheckTree, UrlLinkCheck) UrlLinkCheckHead =
    RB_INITIALIZER(&UrlLinkCheckHead);

RB_PROTOTYPE(UrlCheckTree, UrlLinkCheck, check, UrlLinkCheckCompare);
RB_GENERATE(UrlCheckTree, UrlLinkCheck, check, UrlLinkCheckCompare);

/*
 * Local function prototypes.
 */
static struct UrlLinkInput	*chklinks_input_create(const char *);
static void			 chklinks_input_insert(struct UrlLinkInput *);

static struct UrlLinkCheck	*chklinks_check_create(const char *, UrlCheck);
static int			 chklinks_check_insert(struct UrlLinkCheck *,
					UrlCheck);
static void			 chklinks_check_destroy(struct UrlLinkCheck *);

static void			 chklinks_timeval_diff(struct timeval *,
					struct timeval *, struct timeval *);

static void			 printf_verbose(const char *, ...);
static void			 printf_normal(const char *, ...);
static void			 printf_progress(const char *, size_t, size_t);

static void			 usage(const char *);

/*
 * Global variables.
 */
static struct UrlCheckStats	 stats;
static int			 options;

/*
 * main --
 *	Main entry point.
 */
int
main(int argc, char *argv[])
{
	struct UrlLinkInput	 *url;
	struct UrlLinkCheck	 *check;
	struct UrlLinkFrom	 *from;
	struct timeval		  start, stop, diff;
	const char		 *fin, *progname;
	char			**links, *p;
	char			  input[BUFSIZ];
	size_t			  i, count, done;
	int			  ch;
	FILE			 *fp;

	memset(&stats, 0, sizeof(stats));
	options = FLAG_QUIET;
	fin = NULL;

	/* Get the program name. */
	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	/* Disable quiet mode if we run from a terminal. */
	if (isatty(STDOUT_FILENO))
		options &= ~FLAG_QUIET;

	/* Parse command line arguments. */
	while ((ch = getopt(argc, argv, "f:qv")) != -1) {
		switch (ch) {
		case 'f':
			fin = optarg;
			break;
		case 'q':
			options |= FLAG_QUIET;
			break;
		case 'v':
			options |= FLAG_VERBOSE;
			break;
		default:
			usage(progname);
		}
	}

	if ((options & FLAG_QUIET) && (options & FLAG_VERBOSE)) {
		fprintf(stderr, "Options -q and -v are mutually exclusive.\n");
		exit(EXIT_FAILURE);
	}

	/* Use standard input or external file? */
	if (fin != NULL) {
		fp = fopen(fin, "r");
		if (fp == NULL) {
			fprintf(stderr, "Unable to open '%s': %s\n",
			    fin, strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else {
		fp = stdin;
	}

	/* Read the links from the input stream. */
	while (fgets(input, sizeof(input), fp) != NULL) {
		if ((p = strchr(input, '\n')) != NULL)
			*p = '\0';
		if (strlen(input) == 0)
			continue;

		url = chklinks_input_create(input);
		chklinks_input_insert(url);

		stats.input++;
	}

	gettimeofday(&start, NULL);

	/* If no links were found, then stop here. */
	if (TAILQ_EMPTY(&UrlLinkInputHead)) {
		printf("No links to parse.\n");
		exit(EXIT_SUCCESS);
	}

	done = 0;
	/* Parse all links from each input. */
	TAILQ_FOREACH(url, &UrlLinkInputHead, link) {
		printf_verbose("Parsing %s... ", urlcheck_url(url->input));
		printf_progress("Parsing input links", done, stats.input);

		done++;

		links = urlcheck_parse(url->input, &count);
		if (links == NULL) {
			printf_verbose("%s\n", urlcheck_message(url->input));
			continue;
		}
		if (count == 0) {
			printf_verbose("No links found\n");
			continue;
		}
		printf_verbose("Found %d links\n", count);

		for (i = 0; i < count; i++) {
			check = chklinks_check_create(links[i], url->input);

			if (!chklinks_check_insert(check, url->input))
				chklinks_check_destroy(check);
			else
				stats.total++;
		}
	}
	printf_progress("Parsing input links", done, stats.input);
	printf_normal("\n");

	/* If no links were found, then stop here. */
	if (RB_EMPTY(&UrlLinkCheckHead)) {
		printf("No links to test.\n");
		exit(EXIT_SUCCESS);
	}

	done = 0;
	/* Test all found links. */
	RB_FOREACH(check, UrlCheckTree, &UrlLinkCheckHead) {
		printf_verbose("Testing %s... ", urlcheck_url(check->link));
		printf_progress("Testing found links", done, stats.total);

		done++;

		if (urlcheck_test(check->link) == FALSE &&
		    urlcheck_status(check->link) != CHECK_SKIPPED) {
			TAILQ_INSERT_TAIL(&UrlLinkBrokenHead, check, broken);

			stats.broken++;
		}
		if (urlcheck_status(check->link) == CHECK_SKIPPED)
			stats.skipped++;

		printf_verbose("%s\n", urlcheck_message(check->link));

		usleep(50000);
	}
	printf_progress("Testing found links", done, stats.total);
	printf_normal("\n");

	gettimeofday(&stop, NULL);

	/* Get the passed time. */
	chklinks_timeval_diff(&stop, &start, &diff);

	/* Show the stats. */
	printf("\nTested %lu link%s ", (unsigned long)stats.total,
	    (stats.total != 1) ? "s" : "");
	if (stats.skipped > 0) {
		printf("(skipped %lu link%s) ", (unsigned long)stats.skipped,
		    (stats.skipped != 1) ? "s" : "");
	}
	printf("from %lu input link%s.\n", (unsigned long)stats.input,
	    (stats.input != 1) ? "s" : "");

	printf("Testing those links took %d.%d second%s "
	    "(with 50 ms delay for each link).\n",
	    (int)diff.tv_sec, (int)(diff.tv_usec / 1000),
	    (diff.tv_sec > 1) ? "s" : "");

	/* Report the broken links. */
	if (!TAILQ_EMPTY(&UrlLinkBrokenHead)) {
		printf("Found %lu broken link%s:\n",
		    (unsigned long)stats.broken,
		    (stats.broken != 1) ? "s" : "");

		count = 0;
		TAILQ_FOREACH(check, &UrlLinkBrokenHead, broken) {
			count++;

			printf("\n%lu. %s: %s\n", (unsigned long)count,
			    urlcheck_url(check->link),
			    urlcheck_message(check->link));

			/* Show the pages where the link is mentioned. */
			TAILQ_FOREACH(from, &check->fromhead, from) {
				printf("from page: %s\n",
				    urlcheck_url(from->input));
			}
		}
	} else {
		printf("No broken links found.\n");
	}

	return (EXIT_SUCCESS);
}

/*
 * chklinks_input_create --
 *	Create an UrlLinkInput object.
 */
static struct UrlLinkInput *
chklinks_input_create(const char *linkp)
{
	struct UrlLinkInput *url;

	url = xmalloc(sizeof(struct UrlLinkInput));
	url->input = urlcheck_create(linkp);

	return (url);
}

/*
 * chklinks_input_insert --
 *	Add an UrlLinkInput object to the Input List.
 */
static void
chklinks_input_insert(struct UrlLinkInput *url)
{
	TAILQ_INSERT_TAIL(&UrlLinkInputHead, url, link);
}

/*
 * chklinks_check_create --
 *	Create an UrlLinkCheck object.
 */
static struct UrlLinkCheck *
chklinks_check_create(const char *linkp, UrlCheck url)
{
	struct UrlLinkCheck	*check;
	struct UrlLinkFrom	*from;

	check = xmalloc(sizeof(struct UrlLinkCheck));
	check->link = urlcheck_create(linkp);
	TAILQ_INIT(&check->fromhead);

	from = xmalloc(sizeof(struct UrlLinkFrom));
	from->input = url;

	TAILQ_INSERT_TAIL(&check->fromhead, from, from);

	return (check);
}

/*
 * chklinks_check_insert --
 *	Add an UrlLinkCheck object to the Check Tree.
 */
static int
chklinks_check_insert(struct UrlLinkCheck *check, UrlCheck url)
{
	struct UrlLinkCheck	*find;
	struct UrlLinkFrom	*from;
	int			 found;

	find = RB_FIND(UrlCheckTree, &UrlLinkCheckHead, check);
	if (find == NULL) {
		RB_INSERT(UrlCheckTree, &UrlLinkCheckHead, check);
		return (TRUE);
	}

	found = FALSE;
	TAILQ_FOREACH(from, &find->fromhead, from) {
		if (from->input == url) {
			found = TRUE;
			break;
		}
	}

	if (!found) {
		from = xmalloc(sizeof(struct UrlLinkFrom));
		from->input = url;

		TAILQ_INSERT_TAIL(&find->fromhead, from, from);
	}

	return (FALSE);
}

/*
 * chklinks_check_destroy --
 *	Destroy an UrlLinkCheck object.
 */
static void
chklinks_check_destroy(struct UrlLinkCheck *check)
{
	urlcheck_destroy(check->link);
	xfree(check);
}

/*
 * chklinks_timeval_diff --
 *	Calculate the difference between two timevals.
 */
static void
chklinks_timeval_diff(struct timeval *stop, struct timeval *start,
    struct timeval *diff)
{
	diff->tv_sec = stop->tv_sec - start->tv_sec;
	diff->tv_usec = stop->tv_usec - start->tv_usec;
	if (diff->tv_usec < 0) {
		diff->tv_sec--;
		diff->tv_usec += 1000000;
	}
}

/*
 * printf_verbose --
 *	Print only when verbose option is enabled.
 */
static void
printf_verbose(const char *fmt, ...)
{
	va_list ap;
	
	if (!(options & FLAG_VERBOSE))
		return;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	fflush(stdout);
}

/*
 * printf_normal --
 *	Print only when the verbose and quiet options are not enabled.
 */
static void
printf_normal(const char *fmt, ...)
{
	va_list ap;

	if (options & (FLAG_QUIET | FLAG_VERBOSE))
		return;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	fflush(stdout);
}

/*
 * printf_progress --
 *	Print progress in percent (only if verbose and quiet are not enabled).
 */
static void
printf_progress(const char *str, size_t done, size_t total)
{
	printf_normal("\r%s... %3d%%", str,
	    (int)(((float)done / (float)total) * 100.0));
}

/*
 * usage --
 *	Show how to use this program.
 */
static void
usage(const char *progname)
{
	fprintf(stderr, "Usage: %s [-f filename] [-q | -v]\n", progname);
	exit(EXIT_FAILURE);
}
