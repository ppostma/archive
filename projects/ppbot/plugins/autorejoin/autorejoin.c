/*
 * Copyright (c) 2006-2007 Peter Postma <peter@pointless.nl>
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

#include <string.h>

#include "plugin.h"

/*
 * Local function prototypes.
 */
static void	handle_kick(Connection, Message);

/*
 * plugin_open --
 *	Called when the plugin is loaded/opened.
 */
void
plugin_open(Plugin p)
{
	callback_register(p, "KICK", MSG_USER, 2, handle_kick);
}

/*
 * handle_kick --
 *	Called when a KICK message is received.
 */
static void
handle_kick(Connection conn, Message msg)
{
	const char	*nick = connection_current_nick(conn);
	const char	*kicked = message_parameter(msg, 1);
	const char	*channel  = message_parameter(msg, 0);
	const char	*key;
	ChannelList	 list = connection_channels(conn);
	Channel		 chan;

	/* Re-join if the bot was kicked. */
        if (strcmp(nick, kicked) == 0) {
		log_debug("[%s] Re-joining channel %s.", connection_id(conn),
		    channel);

		/* Look up the channel and check if a key is configured. */
		chan = channel_lookup(list, channel);
		if (chan != NULL)
			key = channel_key(chan);
		else
			key = NULL;

		/* Send the JOIN command to the server. */
		send_join(conn, channel, key);
	}
}
