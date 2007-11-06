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

/*
 * Dictionary plugin for ppbot.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "plugin.h"

/*
 * Name & version of the plugin.
 */
#define PLUGIN_NAME	"dictionary"
#define PLUGIN_VERSION	"20071104"

/*
 * Prefix for the dictionary file.
 */
static const char dict_file[] = ".dict";

/*
 * Local function prototypes.
 */
static void dict_check_message(Plugin, Connection, Message);

static void dict_learn(Plugin, Connection, Message, const char *);
static void dict_explain(Plugin, Connection, Message, const char *);
static void dict_forget(Plugin, Connection, Message, const char *);

/*
 * Table with command <-> function mapping.
 */
static const struct {
	const char *cmd;
	void	  (*func)(Plugin, Connection, Message, const char *);
} commands[] = {
	{ "learn",	dict_learn	},
	{ "define",	dict_learn	},
	{ "explain",	dict_explain	},
	{ "what is",	dict_explain	},
	{ "what's",	dict_explain	},
	{ "forget",	dict_forget	}
};

/*
 * plugin_open --
 *	Called when the plugin is loaded/opened.
 */
void
plugin_open(Plugin p)
{
	callback_register(p, "PRIVMSG", MSG_USER|MSG_DATA, 1,
	    dict_check_message);

	log_plugin(LOG_INFO, p, "Loaded plugin %s v%s.",
	    PLUGIN_NAME, PLUGIN_VERSION);
}

/*
 * dict_check_message --
 *	Check the table with commands for a match.
 */
static void
dict_check_message(Plugin p, Connection conn, Message msg)
{
	const char *buf;
	int	    found = FALSE;
	size_t	    i;

	/* Check if the message was for us. */
	if (message_to_me(conn, msg, &buf) == FALSE)
		return;

	for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
		if (!strncmp(buf, commands[i].cmd, strlen(commands[i].cmd))) {
			found = TRUE;
			break;
		}
	}
	if (found == FALSE)
		return;

	buf += strlen(commands[i].cmd);
	if (*buf != '\0' && !isspace((unsigned char)*buf))
		return;

	while (*buf != '\0' && isspace((unsigned char)*buf))
		buf++;

	(*commands[i].func)(p, conn, msg, buf);
}

/*
 * dict_checkfile --
 *	Check if the dictionary file exists and create it if not.
 */
static int
dict_checkfile(Plugin p, const char *path)
{
	struct stat	 sb;
	FILE		*fp;

	if (stat(path, &sb) < 0) {
		if ((fp = fopen(path, "w")) == NULL) {
			log_plugin(LOG_WARNING, p,
			    "Unable to open '%s' for writing", path);
			return (FALSE);
		} else {
			log_plugin(LOG_INFO, p, "Dictionary file created.");
			fclose(fp);
		}
	}
	return (TRUE);
}

/*
 * dict_addword --
 *	Add a word to the dictionary specified by 'path'.
 */
static int
dict_addword(Plugin p, const char *path, const char *word,
    const char *value)
{
	FILE	*fp;

	if ((fp = fopen(path, "a")) == NULL) {
		log_plugin(LOG_WARNING, p,
		    "Unable to open '%s' for appending", path);
		return (FALSE);
	}
	fprintf(fp, "%s\t%s\n", word, value);
	fclose(fp);

	return (TRUE);
}

/*
 * dict_delword --
 *	Delete a word from the dictionary specified by 'path'.
 */
static int
dict_delword(Plugin p, const char *path, const char *word)
{
	FILE	*fpr, *fpw;
	char	 buf[BUFSIZ];
	char	 temp[PATH_MAX];
	char	*q, *line;
	int	 deleted = FALSE;

	snprintf(temp, sizeof(temp), "%s.temp", path);

	if ((fpr = fopen(path, "r")) == NULL) {
		log_plugin(LOG_WARNING, p,
		    "Unable to open '%s' for reading", path);
		return (FALSE);
	}
	if ((fpw = fopen(temp, "w")) == NULL) {
		log_plugin(LOG_WARNING, p,
		    "Unable to open '%s' for writing", temp);
		fclose(fpr);
		return (FALSE);
	}
	while (fgets(buf, sizeof(buf), fpr) != NULL) {
		if ((q = strchr(buf, '\n')) != NULL)
			*q = '\0';
		if (strlen(buf) == 0)
			continue;

		line = &buf[0];
		if ((q = strchr(line, '\t')) == NULL)
			continue;
                *q++ = '\0';

		if (strcasecmp(line, word) != 0) {
			fprintf(fpw, "%s\t%s\n", buf, q);
		} else {
			deleted = TRUE;
		}
        }
        fclose(fpr);
        fclose(fpw);

	if (rename(temp, path) == -1) {
		log_plugin(LOG_WARNING, p,
		    "Unable to rename '%s' to '%s'", temp, path);
	}

	return (deleted);
}

/*
 * dict_getword --
 *	Retrieve a word from the dictionary specified by 'path'.
 */
static char *
dict_getword(Plugin p, const char *path, const char *word)
{
	char	 buf[BUFSIZ];
	char	*line, *q, *value = NULL;
	FILE	*fp;

	if ((fp = fopen(path, "r")) == NULL) {
		log_plugin(LOG_WARNING, p,
		    "Unable to open '%s' for reading", path);
		return (NULL);
	}
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if ((q = strchr(buf, '\n')) != NULL)
			*q = '\0';
		if (strlen(buf) == 0)
			continue;

		line = &buf[0];
		if ((q = strchr(line, '\t')) == NULL)
			continue;
		*q++ = '\0';

                if (strcasecmp(line, word) == 0) {
			/* Return the value of the word. */
			value = xstrdup(q);
			break;
                }
        }
	fclose(fp);

	return (value);
}

/*
 * dict_learn --
 *	The learn command, used to add words to the dictionary.
 */
static void
dict_learn(Plugin p, Connection conn, Message msg, const char *args)
{
	const char *channel = message_parameter(msg, 0);
	const char *sender = message_sender(msg);
	char	   *buf, *q, *explain, *word = NULL;
	char	    file[PATH_MAX];

	if (*args == '\0') {
		send_privmsg(conn, channel, "%s: learn what?", sender);
		return;
	}

	word = xstrdup(args);

	q = word;
	while (*q != '\0' && !isspace((unsigned char)*q))
		q++;
	if (*q != '\0')
		*q++ = '\0';

	if (*q == '\0') {
		send_privmsg(conn, channel,
		    "%s: what's that supposed to mean?", sender);
		goto out;
	}
	while (*q != '\0' && isspace((unsigned char)*q))
		q++;
	explain = q;

	snprintf(file, sizeof(file), "%s_%s", dict_file, connection_id(conn));

	if (!dict_checkfile(p, file))
		goto out;

	buf = dict_getword(p, file, word);
	if (buf == NULL) {
		if (dict_addword(p, file, word, explain) == TRUE)
			send_notice(conn, sender,
			    "Word '%s' added.", word);
	} else {
		xfree(buf);

		send_notice(conn, sender,
		    "I already know what '%s' is.", word);
	}

 out:
	xfree(word);
}

/*
 * dict_explain --
 *	The explain command, used to retrieve words from the dictionary.
 */
static void
dict_explain(Plugin p, Connection conn, Message msg, const char *args)
{
	const char	*channel = message_parameter(msg, 0);
	const char	*sender = message_sender(msg);
	char		*value, *q, *word = NULL;
	char		 file[PATH_MAX];

	if (*args == '\0') {
		send_privmsg(conn, channel, "%s: explain what?", sender);
		return;
	}

	word = xstrdup(args);

	q = &word[strlen(word) - 1];
	while (isspace((unsigned char)*q) || *q == '?')
		*q-- = '\0';

	snprintf(file, sizeof(file), "%s_%s", dict_file, connection_id(conn));

	if (!dict_checkfile(p, file))
		goto out;

	value = dict_getword(p, file, word);
	if (value == NULL) {
		send_privmsg(conn, channel,
		    "%s: I don't know what that is.", sender);
	} else {
		send_privmsg(conn, channel, "%s is: %s", word, value);

		xfree(value);
	}

 out:
	xfree(word);
}

/*
 * dict_forget --
 *	The forget command, used to delete words in the dictionary.
 */
static void
dict_forget(Plugin p, Connection conn, Message msg, const char *args)
{
	const char	*channel = message_parameter(msg, 0);
	const char	*sender = message_sender(msg);
	char		 file[PATH_MAX];

	if (*args == '\0') {
		send_privmsg(conn, channel, "%s: forget what?", sender);
		return;
	}

	snprintf(file, sizeof(file), "%s_%s", dict_file, connection_id(conn));

	if (!dict_checkfile(p, file))
		return;

	if (dict_delword(p, file, args) == TRUE)
		send_notice(conn, sender, "Word '%s' deleted.", args);
}
