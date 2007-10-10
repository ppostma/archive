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

#include <libxml/tree.h>

#include <string.h>

#include "feed.h"
#include "ircbot.h"
#include "queue.h"

/*
 * A web feed.
 */
struct feed {
	char			 errbuf[BUFSIZ]; /* error message storage */
	TAILQ_HEAD(, feed_item)	 items;
};

/*
 * An item (story) in a web feed.
 */
struct feed_item {
	char			*title;	/* title of the item */
	char			*link;	/* link to the item */
	char			*id;	/* unique identifier for the item */
	TAILQ_ENTRY(feed_item)	 item;
};

/*
 * Rich Site Summary (RSS 0.91) and Really Simple Syndication (RSS 2.0).
 */
static int		 feed_parse_rss(Feed, xmlDocPtr, xmlNodePtr);
static struct feed_item	*feed_parse_rss_item(xmlDocPtr, xmlNodePtr);

/*
 * RDF Site Summary (RSS 0.90 and RSS 1.0).
 */
static int		 feed_parse_rdf(Feed, xmlDocPtr, xmlNodePtr);
static struct feed_item	*feed_parse_rdf_item(xmlDocPtr, xmlNodePtr);

/*
 * Atom Syndication Format.
 */
static int		 feed_parse_atom(Feed, xmlDocPtr, xmlNodePtr);
static struct feed_item	*feed_parse_atom_item(xmlDocPtr, xmlNodePtr);

static struct feed_item	*feed_create_item(xmlChar *, xmlChar *, xmlChar *);
static void		 feed_destroy_item(struct feed_item *);

/*
 * feed_parse --
 *	Parse a web feed, which is provided in the buffer.
 */
int
feed_parse(Feed fp, const char *buffer)
{
	xmlDocPtr  ptr;
	xmlNodePtr root;
	int	   rv = FALSE;

	memset(fp->errbuf, '\0', sizeof(fp->errbuf));

	ptr = xmlParseMemory(buffer, strlen(buffer));
	if (ptr == NULL) {
		strlcpy(fp->errbuf, "unable to parse XML", sizeof(fp->errbuf));
		return (FALSE);
	}

	root = xmlDocGetRootElement(ptr);
	if (root == NULL) {
		strlcpy(fp->errbuf,
		    "unable to get root element from XML", sizeof(fp->errbuf));
		xmlFreeDoc(ptr);
		return (FALSE);
	}

	if (xmlStrcasecmp(root->name, "rss") == 0) {
		/*
		 * Rich Site Summary (RSS 0.91) and
		 * Really Simple Syndication (RSS 2.0).
		 */
		rv = feed_parse_rss(fp, ptr, root);

	} else if (xmlStrcasecmp(root->name, "rdf") == 0) {
		/*
		 * RDF Site Summary (RSS 0.90 and RSS 1.0).
		 */
		rv = feed_parse_rdf(fp, ptr, root);

	} else if (xmlStrcasecmp(root->name, "feed") == 0) {
		/*
		 * Atom Syndication Format.
		 */
		rv = feed_parse_atom(fp, ptr, root);

	} else {
		strlcpy(fp->errbuf, "unable to detect feed type",
		    sizeof(fp->errbuf));
	}

	xmlFreeDoc(ptr);

	return (rv);
}

/*
 * feed_parse_rss --
 *	Parse a RSS formatted feed.
 */
static int
feed_parse_rss(Feed fp, xmlDocPtr ptr, xmlNodePtr root)
{
	xmlNodePtr node;
	struct feed_item *ip;

	/* Search for the channel element. */
	for (node = root->children; node != NULL; node = node->next) {
		if (node->type == XML_ELEMENT_NODE &&
		    xmlStrcasecmp(node->name, "channel") == 0)
			break;
	}
	if (node == NULL) {
		strlcpy(fp->errbuf, "channel element not found",
		    sizeof(fp->errbuf));
		return (FALSE);
	}

	/* Search for the items. */
	for (node = node->children; node != NULL; node = node->next) {
		if (node->type != XML_ELEMENT_NODE)
			continue;
		if (xmlStrcasecmp(node->name, "item") != 0)
			continue;

		ip = feed_parse_rss_item(ptr, node);
		if (ip == NULL)
			continue;

		TAILQ_INSERT_TAIL(&fp->items, ip, item);
	}

	return (TRUE);
}

/*
 * feed_parse_rss_item --
 *	Parse a RSS formatted item.
 */
static struct feed_item *
feed_parse_rss_item(xmlDocPtr ptr, xmlNodePtr node)
{
	xmlChar *title, *link, *guid;
	xmlNodePtr item;

	title = NULL;
	link = NULL;
	guid = NULL;

	/* Search for elements in item. */
	for (item = node->children; item != NULL; item = item->next) {
		if (xmlStrcasecmp(item->name, "title") == 0) {
			if (title != NULL) {
				xmlFree(title);
				title = NULL;
			}
			title = xmlNodeListGetString(ptr, item->children, 1);
		} else if (xmlStrcasecmp(item->name, "link") == 0) {
			if (link != NULL) {
				xmlFree(link);
				link = NULL;
			}
			link = xmlNodeListGetString(ptr, item->children, 1);
		} else if (xmlStrcasecmp(item->name, "guid") == 0) {
			if (guid != NULL) {
				xmlFree(guid);
				guid = NULL;
			}
			guid = xmlNodeListGetString(ptr, item->children, 1);
		}
	}

	return (feed_create_item(title, link, guid));
}

/*
 * feed_parse_rdf --
 *	Parse a RDF formatted feed.
 */
static int
feed_parse_rdf(Feed fp, xmlDocPtr ptr, xmlNodePtr root)
{
	xmlNodePtr node;
	struct feed_item *ip;

	/* Search for the items. */
	for (node = root->children; node != NULL; node = node->next) {
		if (node->type != XML_ELEMENT_NODE)
			continue;
		if (xmlStrcasecmp(node->name, "item") != 0)
			continue;

		ip = feed_parse_rdf_item(ptr, node);
		if (ip == NULL)
			continue;

		TAILQ_INSERT_TAIL(&fp->items, ip, item);
	}

	return (TRUE);
}

/*
 * feed_parse_rdf_item --
 *	Parse a RDF formatted item.
 */
static struct feed_item *
feed_parse_rdf_item(xmlDocPtr ptr, xmlNodePtr node)
{
	xmlChar *title, *link;
	xmlNodePtr item;

	title = NULL;
	link = NULL;

	/* Search for elements in item. */
	for (item = node->children; item != NULL; item = item->next) {
		if (xmlStrcasecmp(item->name, "title") == 0) {
			if (title != NULL) {
				xmlFree(title);
				title = NULL;
			}
			title = xmlNodeListGetString(ptr, item->children, 1);
		} else if (xmlStrcasecmp(item->name, "link") == 0) {
			if (link != NULL) {
				xmlFree(link);
				link = NULL;
			}
			link = xmlNodeListGetString(ptr, item->children, 1);
		}
	}

	return (feed_create_item(title, link, NULL));
}

/*
 * feed_parse_atom --
 *	Parse an Atom formatted feed.
 */
static int
feed_parse_atom(Feed fp, xmlDocPtr ptr, xmlNodePtr root)
{
	xmlNodePtr node;
	struct feed_item *ip;

	/* Search for the entries. */
	for (node = root->children; node != NULL; node = node->next) {
		if (node->type != XML_ELEMENT_NODE)
			continue;
		if (xmlStrcasecmp(node->name, "entry") != 0)
			continue;

		ip = feed_parse_atom_item(ptr, node);
		if (ip == NULL)
			continue;

		TAILQ_INSERT_TAIL(&fp->items, ip, item);
	}

	return (TRUE);
}

/*
 * feed_parse_atom_item --
 *	Parse an Atom formatted item.
 */
static struct feed_item *
feed_parse_atom_item(xmlDocPtr ptr, xmlNodePtr node)
{
	xmlChar *title, *link, *id;
	xmlNodePtr item;

	title = NULL;
	link = NULL;
	id = NULL;

	/* Search for elements in the entry. */
	for (item = node->children; item != NULL; item = item->next) {
		if (xmlStrcasecmp(item->name, "title") == 0) {
			if (title != NULL) {
				xmlFree(title);
				title = NULL;
			}
			title = xmlNodeListGetString(ptr, item->children, 1);
		} else if (xmlStrcasecmp(item->name, "link") == 0) {
			xmlChar *rel = xmlGetProp(item, "rel");
			if (rel == NULL ||
			    xmlStrcasecmp(rel, "alternate") == 0) {
				if (link != NULL) {
					xmlFree(link);
					link = NULL;
				}
				link = xmlGetProp(item, "href");
			}
			if (rel != NULL)
				xmlFree(rel);
		} else if (xmlStrcasecmp(item->name, "id") == 0) {
			if (id != NULL) {
				xmlFree(id);
				id = NULL;
			}
			id = xmlNodeListGetString(ptr, item->children, 1);
		}
	}

	return (feed_create_item(title, link, id));
}

/*
 * feed_create --
 *	Create a feed instance.
 */
Feed
feed_create(void)
{
	struct feed *fp;

	fp = xmalloc(sizeof(struct feed));
	TAILQ_INIT(&fp->items);

	return (fp);
}

/*
 * feed_destroy --
 *	Destroy a feed instance.
 */
void
feed_destroy(Feed fp)
{
	struct feed_item *ip;

	while ((ip = TAILQ_FIRST(&fp->items)) != NULL) {
		TAILQ_REMOVE(&fp->items, ip, item);
		feed_destroy_item(ip);
	}

	xfree(fp);
}

/*
 * feed_error --
 *	Returns the error string in 'errbuf'.
 */
const char *
feed_error(Feed fp)
{
	return (fp->errbuf);
}

/*
 * feed_create_item --
 *	Create an item instance.
 */
static struct feed_item *
feed_create_item(xmlChar *title, xmlChar *link, xmlChar *id)
{
	struct feed_item *ip;

	ip = xmalloc(sizeof(struct feed_item));
	ip->title = (char *)title;
	ip->link = (char *)link;
	ip->id = (char *)id;

	return (ip);
}

/*
 * feed_destroy_item --
 *	Destroy an item instance.
 */
static void
feed_destroy_item(struct feed_item *ip)
{
	xfree(ip->title);
	xfree(ip->link);
	xfree(ip->id);
	xfree(ip);
}

/*
 * feed_item_first --
 *	Returns the first item element in a Feed.
 */
FeedItem
feed_item_first(Feed fp)
{
	return (TAILQ_FIRST(&fp->items));
}

/*
 * feed_item_next --
 *	Returns the next item element from a FeedItem.
 */
FeedItem
feed_item_next(FeedItem ip)
{
	return (TAILQ_NEXT(ip, item));
}

/*
 * feed_item_title --
 *	Accessor function for the 'title' member.
 */
const char *
feed_item_title(FeedItem ip)
{
	return (ip->title);
}

/*
 * feed_item_link --
 *	Accessor function for the 'link' member.
 */
const char *
feed_item_link(FeedItem ip)
{
	return (ip->link);
}

/*
 * feed_item_compare --
 *	Compare a FeedItem to another FeedItem.
 *	Returns TRUE if they are the same, FALSE otherwise.
 *
 *	NOTE: comparison of feeds is done by comparing the unique
 *	identifiers if they exist.  If they are not set, then both
 *	the title and link must differ to yield a comparison of FALSE.
 */
int
feed_item_compare(FeedItem ip, FeedItem ip2)
{
	/* Compare the unique identifiers if available. */
	if (ip->id != NULL && ip2->id != NULL) {
		return (strcmp(ip->id, ip2->id) == 0);
	}

	if (ip->title != NULL && ip2->title != NULL) {
		if (strcmp(ip->title, ip2->title) == 0)
			return (TRUE);
	}
	if (ip->link != NULL && ip2->link != NULL) {
		if (strcmp(ip->link, ip2->link) == 0)
			return (TRUE);
	}

	return (FALSE);
}

/*
 * feed_parser_cleanup --
 *	Clean up used resources.
 */
void
feed_parser_cleanup(void)
{
	xmlCleanupParser();
}
