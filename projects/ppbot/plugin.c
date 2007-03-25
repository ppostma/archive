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

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "ircbot.h"
#include "queue.h"

/*
 * Plugin & callback objects.
 */
struct plugin {
	char			*path;
	void			*handle;
	LIST_HEAD(, callback)	 callbacks;
	TAILQ_ENTRY(plugin)	 link;
};

struct callback {
	char			*cmd;
	int			 flags;
	size_t			 paramcount;
	void			(*fn)(Connection, Message);
	LIST_ENTRY(callback)	 link;
};

static TAILQ_HEAD(, plugin) plugins = TAILQ_HEAD_INITIALIZER(plugins);

/*
 * plugin_add --
 *	Add a plugin to the plugin list.
 */
int
plugin_add(const char *path)
{
	struct plugin	*plug;
	struct stat	 sb;
	void		*handle;

	if (stat(path, &sb) == -1) {
		log_warn("Unable to stat '%s'", path);
		return (FALSE);
	}
	if (!S_ISREG(sb.st_mode)) {
		log_warnx("Not a regular file: '%s'.", path);
		return (FALSE);
	}

	handle = dlopen(path, RTLD_LAZY);
	if (handle == NULL) {
		log_warnx("Unable to open shared object: %s.", dlerror());
		return (FALSE);
	}

	plug = xmalloc(sizeof(struct plugin));
	plug->path = xstrdup(path);
	plug->handle = handle;

	LIST_INIT(&plug->callbacks);
	TAILQ_INSERT_TAIL(&plugins, plug, link);

	return (TRUE);
}

/*
 * plugin_init --
 *	Run the initialization routine for all plugins.
 */
void
plugin_init(void)
{
	struct plugin	*plug;
	void		(*func)(Plugin);

	TAILQ_FOREACH(plug, &plugins, link) {
		func = dlsym(plug->handle, "plugin_open");
		if (func != NULL)
			(*func)(plug);
	}
}

/*
 * plugin_deinit --
 *	Run the de-initialization routine for all plugins.
 */
void
plugin_deinit(void)
{
	struct plugin	*plug;
	void		(*func)(Plugin);

	TAILQ_FOREACH(plug, &plugins, link) {
		func = dlsym(plug->handle, "plugin_close");
		if (func != NULL)
			(*func)(plug);
	}
}

/*
 * plugin_destroy --
 *	Remove all plugins from the plugin list.
 */
void
plugin_destroy(void)
{
	struct plugin	*plug;
	struct callback	*cb;

	while ((plug = TAILQ_FIRST(&plugins)) != NULL) {
		/* Close the shared object. */
		dlclose(plug->handle);

		/* Free memory for the callbacks. */
		while ((cb = LIST_FIRST(&plug->callbacks)) != NULL) {
			LIST_REMOVE(cb, link);
			xfree(cb->cmd);
			xfree(cb);
		}

		/* Free memory for the plugin. */
		TAILQ_REMOVE(&plugins, plug, link);
		xfree(plug->path);
		xfree(plug);
	}
}

/*
 * plugin_execute --
 *	Execute the callbacks for each plugin.
 */
void
plugin_execute(Connection conn, Message msg)
{
	struct plugin	*plug;
	struct callback *cb;

	TAILQ_FOREACH(plug, &plugins, link) {
		LIST_FOREACH(cb, &plug->callbacks, link) {
			/* Check for a match in the command. */
			if (strcmp(message_command(msg), cb->cmd) != 0)
				continue;
			/* Check the parameters. */
			if (message_check_parameters(conn, msg,
			    cb->flags, cb->paramcount) == FALSE)
				continue;
			/* Execute the callback function. */
			(*cb->fn)(conn, msg);
		}
	}
}

/*
 * callback_lookup --
 *	Look up a callback object.
 */
static struct callback *
callback_lookup(Plugin plug, const char *cmd, int flags, size_t argc,
    void (*fn)(Connection, Message))
{
	struct callback *cb;

	LIST_FOREACH(cb, &plug->callbacks, link) {
		if ((cb->cmd == NULL || strcmp(cb->cmd, cmd) == 0) &&
		    (cb->paramcount == argc) &&
		    (cb->flags == flags) && (cb->fn == fn)) {
			return (cb);
		}
	}
	return (NULL);
}

/*
 * callback_register --
 *	Register a callback function in a plugin.
 */
int
callback_register(Plugin plug, const char *cmd, int flags, size_t argc,
    void (*fn)(Connection, Message))
{
	struct callback	*cb;

	cb = xmalloc(sizeof(struct callback));
	cb->cmd = xstrdup(cmd);
	cb->flags = flags;
	cb->paramcount = argc;
	cb->fn = fn;

	LIST_INSERT_HEAD(&plug->callbacks, cb, link);

	return (TRUE);
}

/*
 * callback_deregister --
 *	De-register a callback function in a plugin.
 */
int
callback_deregister(Plugin plug, const char *cmd, int flags, size_t argc,
    void (*fn)(Connection, Message))
{
	struct callback *cb;

	cb = callback_lookup(plug, cmd, flags, argc, fn);
	if (cb != NULL) {
		xfree(cb->cmd);
		xfree(cb);
		return (TRUE);
	}

	return (FALSE);
}
