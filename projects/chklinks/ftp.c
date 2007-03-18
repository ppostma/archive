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
#include <sys/socket.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat/compat.h"
#include "chklinks.h"

/*
 * FTP quit types (QUIT: send QUIT and close, CLOSE: close only).
 */
enum ftp_quit {
	FTP_QUIT,
	FTP_CLOSE
};

/*
 * Local function protoypes.
 */
static int	ftp_connect(Url);
static void	ftp_close(int, enum ftp_quit);
static int	ftp_readreply(int);
static ssize_t	ftp_send(int, const char *, ...);

/*
 * ftp_test --
 *	Test a FTP Url.
 */
int
ftp_test(UrlCheck check, Url url)
{
	const char	*username;
	const char	*password;
	int		 fd, code;

	/* Connect to the FTP server. */
	fd = ftp_connect(url);
	if (fd == -1) {
		urlcheck_set_status(check, CONNECT_FAIL);
		urlcheck_set_message(check, connection_error());
		return (FALSE);
	}

	/* Read the FTP banner. */
	code = ftp_readreply(fd);
	if (code != 220) {
		urlcheck_set_status(check, INVALID_RESPONSE);
		urlcheck_set_message(check, "Invalid response");
		ftp_close(fd, FTP_CLOSE);
		return (FALSE);
	}

	/* Send the USER command. */
	username = url_username(url) ? url_username(url) : "anonymous";
	ftp_send(fd, "USER %s\r\n", username);

	code = ftp_readreply(fd);
	if (code != 331) {
		urlcheck_set_status(check, CHECK_ERROR);
		urlcheck_set_message(check, "FTP login error: %d", code);
		ftp_close(fd, FTP_QUIT);
		return (FALSE);
	}

	/* Send the PASS command. */
	password = url_password(url) ? url_password(url) : "anonymous";
	ftp_send(fd, "PASS %s\r\n", password);

	code = ftp_readreply(fd);
	if (code != 230) {
		urlcheck_set_status(check, CHECK_ERROR);
		urlcheck_set_message(check, "FTP login error: %d", code);
		ftp_close(fd, FTP_QUIT);
		return (FALSE);
	}

	/*
	 * Try RETR on the path.  If we got a 550 then it's probably a
	 * directory, we then try to CWD into it.  We don't setup a data
	 * channel so we assume that the server can send the file if we
	 * receive a 425.
	 * XXX This might be an invalid assumption!
	 */
	ftp_send(fd, "RETR %s\r\n", url_path(url));

	code = ftp_readreply(fd);
	if (code == 550) {
		ftp_send(fd, "CWD %s\r\n", url_path(url));

		code = ftp_readreply(fd);
		if (code != 250) {
			urlcheck_set_status(check, CHECK_ERROR);
			urlcheck_set_message(check, "FTP error: %d", code);
			ftp_close(fd, FTP_QUIT);
			return (FALSE);
		}
	} else if (code != 425) {
		urlcheck_set_status(check, CHECK_ERROR);
		urlcheck_set_message(check, "FTP error: %d", code);
		ftp_close(fd, FTP_QUIT);
		return (FALSE);
	}

	urlcheck_set_status(check, CHECK_SUCCESS);
	urlcheck_set_message(check, "OK");

	ftp_close(fd, FTP_QUIT);

	return (TRUE);
}

/*
 * ftp_connect -
 *	Connect to the FTP Url.
 */
static int
ftp_connect(Url url)
{
	return (connection_create(url_host(url), url_port(url)));
}

/*
 * ftp_close --
 *	Close a FTP connection.
 */
void
ftp_close(int fd, enum ftp_quit quit)
{
	if (quit == FTP_QUIT)
		ftp_send(fd, "QUIT\r\n");

	connection_close(fd);
}

/*
 * ftp_readline --
 *	Read a line from the file descriptor.
 */
static ssize_t
ftp_readline(int fd, char *line, size_t len)
{
	ssize_t b;
	size_t	i = 0;
	char	temp;

	do {
		b = connection_recv(fd, &temp, sizeof(temp));
		if (b == -1)
			return (-1);
		else if (b == 0)
			break;
		*line++ = temp;
	} while (++i < len && temp != '\n');

	if (i > 0)
		*line = '\0';

	return (i);
}

#define ftp_iscode(x)						\
    (((x[0]) != '\0') && (isdigit((unsigned char)(x[0]))) &&	\
     ((x[1]) != '\0') && (isdigit((unsigned char)(x[1]))) &&	\
     ((x[2]) != '\0') && (isdigit((unsigned char)(x[2]))))

/*
 * ftp_readreply --
 *	Read reply from the FTP server and return the reply code.
 */
static int
ftp_readreply(int fd)
{
	char	response[BUFSIZ];
	char	code[4];
	int	continuation;
	ssize_t	rv;

	memset(code, 0, sizeof(code));
	continuation = TRUE;

	do {
		rv = ftp_readline(fd, response, sizeof(response));
		if (rv == -1)
			break;
		if (ftp_iscode(response)) {
			if (response[3] != '\0' && response[3] != '-') {
				continuation = FALSE;
				strlcpy(code, response, sizeof(code));
			}
		}
	} while (continuation == TRUE);

	if (strlen(code) != 0)
		return (atoi(code));

	return (-1);
}

/*
 * ftp_send --
 *	Send a command to the FTP server.
 */
static ssize_t
ftp_send(int fd, const char *fmt, ...)
{
	va_list	 ap;
	char	*str;
	ssize_t	 rv;

	va_start(ap, fmt);
	str = xvsprintf(fmt, ap);
	va_end(ap);

	rv = connection_send(fd, str, strlen(str));

	xfree(str);

	return (rv);
}
