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

#ifndef _FEED_H_
#define _FEED_H_

#include <sys/types.h>

/*
 * Name & version of the plugin.
 */
#define PLUGIN_NAME	"ppbot web feed reader"
#define PLUGIN_VERSION	"20071007"

/* URL flags. */
#define URL_SCHEME	0x01
#define URL_HOST	0x02
#define URL_PORT	0x04
#define URL_AUTHORITY	(URL_HOST | URL_PORT)
#define URL_PATH	0x08
#define URL_COMPLETE	(URL_SCHEME | URL_AUTHORITY | URL_PATH)
#define URL_PATHBASE	0x10

/* Feed object. */
typedef struct feed *Feed;

/* Feed config object. */
typedef struct feed_config *FeedConfig;

/* Feed destination object. */
typedef struct feed_dest *FeedDestination;

/* Feed updater object. */
typedef struct feed_updater *FeedUpdater;

/* Feed item object. */
typedef struct feed_item *FeedItem;

/* HTTP result object. */
typedef struct http_result *HttpResult;

/* URL object. */
typedef struct url *Url;

/*
 * Function prototypes.
 */

/* feed_config_scan.l */
void		 feed_lex_cleanup(void);

/* feed_config_parse.y */
void		 feed_config_set_listener(void (*)(FeedConfig));

int		 feed_config_parse(const char *);
void		 feed_config_destroy(FeedConfig);

const char	*feed_config_id(FeedConfig);
const char	*feed_config_name(FeedConfig);
const char	*feed_config_location(FeedConfig);
int		 feed_config_refresh(FeedConfig);
int		 feed_config_update(FeedConfig);
unsigned int	 feed_config_showcount(FeedConfig);

FeedDestination	 feed_destination_first(FeedConfig);
FeedDestination	 feed_destination_next(FeedDestination);

const char	*feed_destination_id(FeedDestination);
const char	*feed_destination_channel(FeedDestination);

void		 feed_yacc_cleanup(void);

/* feed_connection.c */
int		 feed_connection_create(const char *, const char *);
void		 feed_connection_close(int);
char		*feed_connection_recv_data(int);
ssize_t		 feed_connection_recv(int, char *, size_t);
ssize_t		 feed_connection_send(int, const char *, size_t);
const char	*feed_connection_error(void);

/* feed_http.c */
HttpResult	 http_receive(const char *, const char *, const char *);
void		 http_destroy(HttpResult);

int		 http_result_code(HttpResult);
int		 http_result_status(HttpResult);
const char	*http_result_body(HttpResult);
const char	*http_result_error(HttpResult);

const char	*http_header_value(HttpResult, const char *);

/* feed_parser.c */
int		 feed_parse(Feed, const char *);

Feed		 feed_create(void);
void		 feed_destroy(Feed);

const char	*feed_error(Feed);

FeedItem	 feed_item_first(Feed);
FeedItem	 feed_item_next(FeedItem);	

const char	*feed_item_title(FeedItem);
const char	*feed_item_link(FeedItem);

int		 feed_item_compare(FeedItem, FeedItem);

/* feed_updater.c */
FeedUpdater	 feed_updater_create(FeedConfig);
void		 feed_updater_destroy(FeedUpdater);

void		 feed_updater_set_new_item(FeedUpdater,
			void (*)(FeedConfig, FeedItem));

int		 feed_update(FeedUpdater);

/* feed_url.c */
Url		 url_parse(const char *, const char *);
char		*url_asprintf(Url, int);
char		*url_encode(Url, int);
void		 url_free(Url);

const char	*url_scheme(Url);
const char	*url_host(Url);
const char	*url_port(Url);
const char	*url_path(Url);

#endif /* _FEED_H_ */
