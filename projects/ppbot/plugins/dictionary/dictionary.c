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
#include <sys/stat.h>

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "plugin.h"

static const char dict_file[] = ".dict";

static void check_message(Connection, Message);

static void dict_learn(Connection, Message, const char *);
static void dict_explain(Connection, Message, const char *);
static void dict_forget(Connection, Message, const char *);

static const struct {
	const char *cmd;
	void	(*func)(Connection, Message, const char *);
} commands[] = {
	{ "learn",	dict_learn	},
	{ "define",	dict_learn	},
	{ "explain",	dict_explain	},
	{ "what is",	dict_explain	},
	{ "what's",	dict_explain	},
	{ "forget",	dict_forget	}
};

void
plugin_open(Plugin p)
{
	callback_register(p, "PRIVMSG", MSG_USER|MSG_DATA, 1, check_message);
}

static void
check_message(Connection conn, Message msg)
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

	(*commands[i].func)(conn, msg, buf);
}

static int
dict_checkfile(Connection conn, const char *path)
{
	struct stat	 sb;
	FILE		*fp;

	if (stat(path, &sb) < 0) {
		if ((fp = fopen(path, "w")) == NULL) {
			log_warn("[%s] Unable to open '%s' for writing",
			    connection_id(conn), path);
			return (FALSE);
		} else {
			log_warnx("[%s] Dictionary file created.",
			    connection_id(conn));
			fclose(fp);
		}
	}
	return (TRUE);
}

static int
dict_addword(Connection conn, const char *path, const char *word,
    const char *value)
{
	FILE	*fp;

	if ((fp = fopen(path, "a")) == NULL) {
		log_warn("[%s] Unable to open '%s' for appending",
		    connection_id(conn), path);
		return (FALSE);
	}
	fprintf(fp, "%s\t%s\n", word, value);
	fclose(fp);

	return (TRUE);
}

static int
dict_delword(Connection conn, const char *path, const char *word)
{
	FILE	*fpr, *fpw;
	char	 buf[BUFSIZ];
	char	 temp[PATH_MAX];
	char	*p, *line;
	int	 deleted = FALSE;

	snprintf(temp, sizeof(temp), "%s.temp", path);

	if ((fpr = fopen(path, "r")) == NULL) {
		log_warn("[%s] Unable to open '%s' for reading",
		    connection_id(conn), path);
		return (FALSE);
	}
	if ((fpw = fopen(temp, "w")) == NULL) {
		log_warn("[%s] Unable to open '%s' for writing",
		    connection_id(conn), temp);
		fclose(fpr);
		return (FALSE);
	}
	while (fgets(buf, sizeof(buf), fpr) != NULL) {
		if ((p = strchr(buf, '\n')) != NULL)
			*p = '\0';
		if (strlen(buf) == 0)
			continue;

		line = &buf[0];
		if ((p = strchr(line, '\t')) == NULL)
			continue;
                *p++ = '\0';

		if (strcasecmp(line, word) != 0) {
			fprintf(fpw, "%s\t%s\n", buf, p);
		} else {
			deleted = TRUE;
		}
        }
        fclose(fpr);
        fclose(fpw);

	if (rename(temp, path) == -1) {
		log_warn("[%s] Unable to rename '%s' to '%s'",
		    connection_id(conn), temp, path);
	}

	return (deleted);
}

static char *
dict_getword(Connection conn, const char *path, const char *word)
{
	char	 buf[BUFSIZ];
	char	*line, *p, *value = NULL;
	FILE	*fp;

	if ((fp = fopen(path, "r")) == NULL) {
		log_warn("[%s] Unable to open '%s' for reading",
		    connection_id(conn), path);
		return (NULL);
	}
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if ((p = strchr(buf, '\n')) != NULL)
			*p = '\0';
		if (strlen(buf) == 0)
			continue;

		line = &buf[0];
		if ((p = strchr(line, '\t')) == NULL)
			continue;
		*p++ = '\0';

                if (strcasecmp(line, word) == 0) {
			/* Return the value of the word. */
			value = xstrdup(p);
			break;
                }
        }
	fclose(fp);

	return (value);
}

static void
dict_learn(Connection conn, Message msg, const char *args)
{
	const char *channel = message_parameter(msg, 0);
	const char *sender = message_sender(msg);
	char	   *buf, *p, *explain, *word = NULL;
	char	    file[PATH_MAX];

	if (*args == '\0') {
		send_privmsg(conn, channel, "%s: learn what?", sender);
		return;
	}

	word = xstrdup(args);

	p = word;
	while (*p != '\0' && !isspace((unsigned char)*p))
		p++;
	if (*p != '\0')
		*p++ = '\0';

	if (*p == '\0') {
		send_privmsg(conn, channel,
		    "%s: what's that supposed to mean?", sender);
		goto out;
	}
	while (*p != '\0' && isspace((unsigned char)*p))
		p++;
	explain = p;

	snprintf(file, sizeof(file), "%s_%s", dict_file, connection_id(conn));

	if (!dict_checkfile(conn, file))
		goto out;

	buf = dict_getword(conn, file, word);
	if (buf == NULL) {
		if (dict_addword(conn, file, word, explain) == TRUE)
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

static void
dict_explain(Connection conn, Message msg, const char *args)
{
	const char	*channel = message_parameter(msg, 0);
	const char	*sender = message_sender(msg);
	char		*value, *p, *word = NULL;
	char		 file[PATH_MAX];

	if (*args == '\0') {
		send_privmsg(conn, channel, "%s: explain what?", sender);
		return;
	}

	word = xstrdup(args);

	p = &word[strlen(word) - 1];
	while (isspace((unsigned char)*p) || *p == '?')
		*p-- = '\0';

	snprintf(file, sizeof(file), "%s_%s", dict_file, connection_id(conn));

	if (!dict_checkfile(conn, file))
		goto out;

	value = dict_getword(conn, file, word);
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

static void
dict_forget(Connection conn, Message msg, const char *args)
{
	const char	*channel = message_parameter(msg, 0);
	const char	*sender = message_sender(msg);
	char		 file[PATH_MAX];

	if (*args == '\0') {
		send_privmsg(conn, channel, "%s: forget what?", sender);
		return;
	}

	snprintf(file, sizeof(file), "%s_%s", dict_file, connection_id(conn));

	if (!dict_checkfile(conn, file))
		return;

	if (dict_delword(conn, file, args) == TRUE)
		send_notice(conn, sender, "Word '%s' deleted.", args);
}
