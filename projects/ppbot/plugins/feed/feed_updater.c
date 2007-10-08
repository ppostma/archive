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
#include <stdlib.h>
#include <unistd.h>

#include "feed.h"
#include "ircbot.h"
#include "queue.h"

/*
 * Feed updater object.
 */
struct feed_updater {
	Feed			 old;	   /* old feed */
	Feed			 new;	   /* new feed (compare with old) */
	FeedConfig		 fcp;      /* feed configuration */
	char			*modified; /* last recorded Modified header */
	char			*etag;	   /* last recorded ETag header */
	void			(*new_item)(FeedConfig, FeedItem);
};

/*
 * feed_updater_create --
 *	Create an instance of feed_updater.
 */
FeedUpdater
feed_updater_create(FeedConfig fcp)
{
	struct feed_updater *fup;

	fup = xcalloc(1, sizeof(struct feed_updater));
	fup->fcp = fcp;
	fup->modified = NULL;
	fup->etag = NULL;

	return (fup);
}

/*
 * feed_updater_destroy --
 *	Destroy a feed_updater instance.
 */
void
feed_updater_destroy(FeedUpdater fup)
{
	/* Free the old and new feeds. */
	if (fup->old != NULL)
		feed_destroy(fup->old);
	if (fup->new != NULL)
		feed_destroy(fup->new);

	xfree(fup->modified);
	xfree(fup->etag);
	xfree(fup);
}

/*
 * feed_updater_set_new_item --
 *	Sets the new item function.
 */
void
feed_updater_set_new_item(FeedUpdater fup,
    void (*new_item)(FeedConfig, FeedItem))
{
	fup->new_item = new_item;
}

/*
 * feed_read --
 *	Update the feed into the 'new' member of feed_updater.
 *	Returns FALSE if updating failed or if there were no changes
 *	in the feed.
 */
static int
feed_read(FeedUpdater fup)
{
	HttpResult	result;
	Feed		fp;
	int		rv;

	result = http_receive(feed_config_location(fup->fcp),
	    fup->modified, fup->etag);
	if (http_result_status(result) == FALSE) {
		log_warnx("Unable to update feed %s: %s",
		    feed_config_id(fup->fcp), http_result_error(result));
		http_destroy(result);
		return (FALSE);
	}

	if (http_result_code(result) == 304) {
		log_debug("No changes in feed %s", feed_config_id(fup->fcp));
		http_destroy(result);
		return (FALSE);
	}

	fp = feed_create();

	rv = feed_parse(fp, http_result_body(result));
	if (rv == FALSE) {
		log_warnx("Unable to parse feed %s: %s",
		    feed_config_id(fup->fcp), feed_error(fp));
		feed_destroy(fp);
		http_destroy(result);
                return (FALSE);
	}
	fup->new = fp;

	xstrdup2(&fup->modified,
	    http_header_value(result, "Last-modified"));
	xstrdup2(&fup->etag,
	    http_header_value(result, "ETag"));

	http_destroy(result);

	return (TRUE);
}

/*
 * feed_check_new --
 *	Check for new items in the feed and announce them with
 *	with the new item function. This is O(n2) but the lists are
 *	generally not very large.
 */
static void
feed_check_new(FeedUpdater fup)
{
	FeedItem	np, op;
	int		exists;

	if (fup->new_item == NULL)
		return;

	for (np = feed_item_first(fup->new); np != NULL;
	     np = feed_item_next(np)) {
		exists = FALSE;

		for (op = feed_item_first(fup->old); op != NULL;
		     op = feed_item_next(op)) {
			if (feed_item_compare(np, op) == TRUE) {
				exists = TRUE;
				break;
			}
		}

		if (!exists) {
			(*fup->new_item)(fup->fcp, np);
		}
	}
}

/*
 * feed_update --
 *	Update a feed and send new items events.
 */
int
feed_update(FeedUpdater fup)
{
	log_debug("Updating feed %s", feed_config_id(fup->fcp));

	if (feed_read(fup) == FALSE)
		return (FALSE);

	if (fup->new != NULL && fup->old != NULL)
		feed_check_new(fup);

	if (fup->new != NULL) {
		if (fup->old != NULL)
			feed_destroy(fup->old);
		fup->old = fup->new;
		fup->new = NULL;
	}

	return (TRUE);
}
