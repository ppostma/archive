/*
 * Copyright (c) 2003,2004 Peter Postma <peter@webdeveloping.nl>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: ess.c,v 1.47 2004-03-06 12:58:05 peter Exp $
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BANNER_SIZE	2048	/* max receive size for banner */
#define BANNER_TIMEOUT	1000	/* milliseconds after last banner recv */
#define CONNECT_TIMEOUT	3000	/* milliseconds when connect timeouts */

#define HTTP_REQUEST	"HEAD / HTTP/1.0\r\n\r\n"
#define VERSION		"0.4.0-beta"

static int	   tconnect(int, struct sockaddr *, socklen_t, long);
static size_t	   readln(int, char *, size_t);
static size_t	   readall(int);
static int	   readcode(int);
static const char *get_af(int);
static const char *get_addr(struct sockaddr *, socklen_t, int);
static const char *get_serv(const char *, int);
static void	   banner_scan(int, u_short, long);
static int	   ident_scan(const char *, int, u_short, u_short,
			char *, size_t, long);
static int	   ftp_scan(int, const char *);
static int	   relay_scan(int, const char *, const char *, int);
static int	   http_proxy_scan(int);
static int	   socks_proxy_scan(int);
static void	   usage(const char *);
static void	   fatal(int);

static int verbose_flag = 0;

#ifdef IPV4_DEFAULT
static int IPv4or6 = AF_INET;
#else
static int IPv4or6 = AF_UNSPEC;
#endif

#ifdef RESOLVE
static int resolve_flag = 1;
#else
static int resolve_flag = 0;
#endif

int
main(int argc, char *argv[])
{
	struct sockaddr_storage	 ss;
	struct addrinfo		*res, *ai, hints;
	const char		*host, *port, *progname; 
	char			 ip[NI_MAXHOST], name[NI_MAXHOST];
	char			 portnr[NI_MAXSERV], service[NI_MAXSERV];
	char			 result[128], owner[128], *proxy_type, *p;
	int			 ch, error, ret = 70, ssock = -1;
	int			 all_flag = 0;
	int			 ftp_flag = 0;
	int			 ident_flag = 0;
	int			 proxy_flag = 0;
	int			 quiet_flag = 0;
	int			 relay_flag = 0;
	int			 banner_flag = 0;
	u_short			 remoteport, localport;
	long			 timeout = 0;
	socklen_t		 len;

	host = port = NULL;
	progname = argv[0];

	while ((ch = getopt(argc, argv, "46abfhinp:rqst:vV")) != -1) {
		switch (ch) {
		case '4':
			IPv4or6 = AF_INET;
			break;
		case '6':
			IPv4or6 = AF_INET6;
			break;
		case 'a':
			all_flag = 1;
			break;
		case 'b':
			banner_flag = 1;
			break;
		case 'f':
			ftp_flag = 1;
			port = "21";
			break;
		case 'i':
			ident_flag = 1;
			break;
		case 'n':
			resolve_flag = resolve_flag ? 0 : 1;
			break;
		case 'p':
			proxy_flag = 1;
			proxy_type = optarg;
			if (strcmp(proxy_type, "http") == 0)
				port = "8080";
			else if (strcmp(proxy_type, "socks") == 0)
				port = "1080";
			else
				errx(65, "Unknown proxy type '%s'", optarg);
			break;
		case 'q':
			if (verbose_flag)
				errx(65, "Options -v (verbose) and -q (quiet) "
					 "cannot be used together!");
			quiet_flag = 1;
			break;
		case 'r':
			relay_flag++;
			port = "25";
			break;
		case 't':
			timeout = strtol(optarg, &p, 10);
			if (optarg[0] == '\0' || *p == '\0')
				errx(65, "Invalid timeout value '%s'", optarg);
			break;
		case 'v':
			if (quiet_flag)
				errx(65, "Options -v (verbose) and -q (quiet) "
					 "cannot be used together!");
			verbose_flag = 1;
			break;
		case 'V':
			errx(99, "Easy Service Scan v%s by Peter Postma "
				 "<peter@webdeveloping.nl>", VERSION);
			/* NOTREACHED */
		case 'h':
		case '?':
		default:
			usage(progname);
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	host = argv[0];
	if (argc < 2 && (port == NULL || host == NULL)) {
		usage(progname);
		/* NOTREACHED */
	}

	if (port == NULL || argv[1] != NULL)
		port = argv[1];

	if (timeout == 0)
		timeout = CONNECT_TIMEOUT;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = IPv4or6;
	hints.ai_socktype = SOCK_STREAM;

	if ((error = getaddrinfo(host, port, &hints, &res)))
		fatal(error);

	if (res->ai_next != NULL && !all_flag && !quiet_flag)
		printf("Resolved to multiple addresses! "
		       "Use option -a to scan them all.\n");

	for (ai = res; ai != NULL; ai = ai->ai_next) {
		if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6)
			continue;
		if (verbose_flag) {
			strncpy(ip, get_addr(ai->ai_addr, ai->ai_addrlen, 0),
			    sizeof(ip));
			if (resolve_flag) {
				strncpy(name, get_addr(ai->ai_addr,
				    ai->ai_addrlen, 1), sizeof(name));
				printf("Trying %s", name);
				if (strcmp(ip, name) != 0)
					printf(" (%s)... ", ip);
				else
					printf("... ");
			} else
				printf("Trying %s... ", ip);
			fflush(stdout);
		}
		ssock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (ssock < 0) {
			perror("socket");
			continue;
		}
		ret = tconnect(ssock, ai->ai_addr, ai->ai_addrlen, timeout);
		if (ret != 0) {		/* not open? */
			if (verbose_flag)
				printf("connection failed!\n");
			switch (ret) {
			case 1:
				strcpy(result, "closed");
				break;
			case 2:
				strcpy(result, "no response");
				break;
			}
			goto print_results;
		}
		if (verbose_flag)
			printf("connection successful!\n");
		if (ident_flag) {
			len = sizeof(ss);
			if (getsockname(ssock,(struct sockaddr *)&ss, &len) < 0)
				err(255, "getsockname");
			if ((error = getnameinfo((struct sockaddr *)&ss, len,
			    NULL, 0, service, sizeof(service), NI_NUMERICSERV)))
				fatal(error);
			localport = atoi(service);
			remoteport = atoi(get_serv(port, 0));
			strncpy(ip, get_addr(ai->ai_addr, ai->ai_addrlen, 0),
			    sizeof(ip));
			memset(&owner, 0, sizeof(owner));
			ret = ident_scan(ip, ai->ai_family, remoteport,
			    localport, owner, sizeof(owner), timeout);
			switch (ret) {
			case 0:
				snprintf(result, sizeof(result),
				    "owner: %s", owner);
				break;
			case 1:
				strcpy(result, "owner: ?");
				break;
			case 2:
				strcpy(result, "cannot connect to identd!");
				break;
			}
		} else if (ftp_flag) {
			ret = ftp_scan(ssock, "anonymous");
			switch (ret) {
			case 0:
				strcpy(result, "anonymous login allowed!");
				break;
			case 1:
				strcpy(result, "anonymous login denied!");
				break;
			case 2:
				strcpy(result, "could not login.");
				break;
			}
			if (verbose_flag)
				printf("\n");
		} else if (relay_flag) {
			if (relay_flag > 1) {
				strncpy(ip, get_addr(ai->ai_addr,
				    ai->ai_addrlen, 0), sizeof(ip));
				if (strcmp(ip, host) == 0)
					strncpy(name, get_addr(ai->ai_addr,
					    ai->ai_addrlen, 1), sizeof(name));
				else
					strncpy(name, host, sizeof(name));
				ret = relay_scan(ssock, name, ip, relay_flag);
			} else
				ret = relay_scan(ssock, NULL, NULL, relay_flag);

			switch (ret) {
			case 0:
				strcpy(result, "relay access allowed!");
				break;
			case 1:
				strcpy(result, "relay access denied!");
				break;
			case 2:
				strcpy(result, "could not login.");
				break;
			}
			if (verbose_flag)
				printf("\n");
		} else if (proxy_flag) {
			if (strcmp(proxy_type, "http") == 0)
				ret = http_proxy_scan(ssock);
			else if (strcmp(proxy_type, "socks") == 0)
				ret = socks_proxy_scan(ssock);

			switch (ret) {
			case 0:
				strcpy(result, "proxy connection allowed!");
				break;
			case 1:
				strcpy(result, "proxy connection denied!");
				break;
			case 2:
				strcpy(result, "invalid response (no proxy?)");
				break;
			}
			if (verbose_flag)
				printf("\n");
		} else if (banner_flag) {
			strcpy(result, "banner:\n");
		} else
			strcpy(result, "open");

print_results:
		if (!quiet_flag) {
			/* Print address family */
			printf("%s host ", get_af(ai->ai_family));

			/* Print address/hostname */
			printf("(%s) ", get_addr(ai->ai_addr, ai->ai_addrlen,
			    resolve_flag));

			/* Get port and service */
			strncpy(portnr, get_serv(port, 0), sizeof(portnr));
			strncpy(service, get_serv(port, 1), sizeof(service));

			/* Print port number */
			printf("port %s ", portnr);

			/* Print service if available */
			if (strcmp(portnr, service) != 0)
				printf("(%s) ", service);

			/* Print result */
			printf("-> %s\n", result);

			/* Print banner at last */
			if (banner_flag && ret == 0)
				banner_scan(ssock, atoi(get_serv(port, 0)),
				    BANNER_TIMEOUT);
		}

		close(ssock);

		if (!all_flag)
			break;
	}

	freeaddrinfo(res);

	return ret;
}

/*
 * tconnect - connect() with timeout
 * returns 0: ok, 1: refused, 2: timeout
 */
static int
tconnect(int sock, struct sockaddr *addr, socklen_t len, long timeout)
{
	struct timeval tv;
	socklen_t optlen;
	int flags, val, ret = 0;
	fd_set fds;

	if ((flags = fcntl(sock, F_GETFL, 0)) < 0)
		err(255, "fcntl(F_GETFL)");
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
		err(255, "fcntl(F_SETFL)");

	if (connect(sock, addr, len) < 0) {
		if (errno == EINPROGRESS) {
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = timeout % 1000;
			FD_ZERO(&fds);
			FD_SET(sock, &fds);
			if (select(sock + 1, NULL, &fds, NULL, &tv) > 0) {
				optlen = sizeof(int);
				if (getsockopt(sock, SOL_SOCKET, SO_ERROR,
				    (void *)&val, &optlen) < 0)
					err(255, "getsockopt");
				if (val)
					ret = 1;
			} else
				ret = 2;
		} else
			ret = 1;
	}
	if (fcntl(sock, F_SETFL, flags) < 0)
		err(255, "fcntl(F_SETFL)");

	return ret;
}

static size_t
readln(int fd, char *line, size_t len)
{
	ssize_t	b, i = 0;
	char	temp;

	do {
		if ((b = recv(fd, &temp, 1, 0)) < 0)
			return b;
		else if (b == 0)
			break;
		*line++ = temp;
	} while (++i < len && temp != '\n');

	if (i > 0)
		*line = '\0';

	return i;
}

static size_t
readall(int fd)
{
	char	response[1024];
	size_t	b, count = 0;

	do {
		if ((b = readln(fd, response, sizeof(response))) > 0)
			count += b;
		if (verbose_flag)
			printf("<<< %s", response);
	} while (isdigit((unsigned char)response[0]) &&
		 isdigit((unsigned char)response[1]) &&
		 isdigit((unsigned char)response[2]) &&
		 response[3] == '-');

	return count;
}

static int
readcode(int fd)
{
	char	response[1024];
	char	code[4];

	memset(&code, 0, sizeof(code));

	do {
		(void)readln(fd, response, sizeof(response));
		if (verbose_flag)
			printf("<<< %s", response);
		if (isdigit((unsigned char)response[0]) &&
		    isdigit((unsigned char)response[1]) &&
		    isdigit((unsigned char)response[2]))
			strncpy(code, response, 3);
	} while (response[3] == '-');

	if (code)
		return atoi(code);

	return -1;
}

static const char *
get_af(int af)
{
	switch (af) {
	case AF_INET:
		return "IPv4";
		break;	/* NOTREACHED */
	case AF_INET6:
		return "IPv6";
		break;	/* NOTREACHED */
	}
	return "?";
}

static const char *
get_addr(struct sockaddr *addr, socklen_t len, int resolve)
{
	struct sockaddr_storage	ss;
	static char	host[NI_MAXHOST];

	memcpy(&ss, addr, (size_t)len);
	if (getnameinfo((struct sockaddr *)&ss, len, host,
	    sizeof(host), NULL, 0, resolve ? 0 : NI_NUMERICHOST) == 0)
		return host;

	return "?";
}

static const char *
get_serv(const char *port, int resolve)
{
	struct servent	*serv;
	static char	 buf[NI_MAXSERV];

	if (strspn(port, "0123456789") != strlen(port)) {
		serv = getservbyname(port, "tcp");
		if (serv != NULL && !resolve)
			snprintf(buf, sizeof(buf), "%u", ntohs(serv->s_port));
		else
			snprintf(buf, sizeof(buf), "%s", port);
        } else {
		serv = getservbyport(htons(atoi(port)), "tcp");
		if (serv != NULL && resolve)
			snprintf(buf, sizeof(buf), "%s", serv->s_name);
		else
			snprintf(buf, sizeof(buf), "%s", port);
	}

	return buf;
}

static void
banner_scan(int sock, u_short port, long timeout)
{
	struct timeval	 tv;
	fd_set		 read_fds;
	char		*buf, *save;
	int		 count;
	unsigned char	 ch;

	tv.tv_sec = timeout / 1000;
	tv.tv_usec = timeout % 1000;

	FD_ZERO(&read_fds);
	FD_SET(sock, &read_fds);

	if ((buf = (char *)malloc(BANNER_SIZE)) == NULL)
		err(255, "malloc");

	if (port == 80)
		send(sock, HTTP_REQUEST, strlen(HTTP_REQUEST), 0);

	for (;;) {
		select(sock + 1, &read_fds, NULL, NULL, &tv);
		if (!FD_ISSET(sock, &read_fds))
			break;
		if ((count = recv(sock, buf, BANNER_SIZE - 1, 0)) < 1)
			break;
		buf[count] = '\0';
		save = buf;
		while ((ch = *save++) != '\0' && count-- > 0) {
			if (ch == '\r')
				printf("\\r");
			else if (ch == '\n')
				printf("\\n\n"); /* +newline for nice output */
			else if (ch == '\t')
				printf("\\t");
			else if (ch == '\\')
				printf("\\\\");
			else if (isprint(ch))
				printf("%c", ch);
			else
				printf("\\%03d", ch);
		}
		fflush(stdout);
	}

	free(buf);
	return;
}

static int
ident_scan(const char *ip, int ai_family, u_short remoteport, u_short localport,
    char *owner, size_t len, long timeout)
{
	struct addrinfo	 hints, *ai;
	char		*temp, *p;
	char		 buf[1024];
	int		 isock, error, bytes;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = ai_family;
	hints.ai_socktype = SOCK_STREAM;

	if ((error = getaddrinfo(ip, "113", &hints, &ai)))
		fatal(error);

	isock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (isock < 0)
		return 2;

	if (tconnect(isock, ai->ai_addr, ai->ai_addrlen, timeout) != 0) {
		close(isock);
		return 2;
	}

	snprintf(buf, sizeof(buf), "%u,%u\r\n", remoteport, localport);
	send(isock, buf, strlen(buf), 0); 
	if ((bytes = recv(isock, buf, sizeof(buf) - 1, 0)) < 0)
		return 255;
	else if (bytes == 0)
		return 1;

	close(isock);
	freeaddrinfo(ai);

	buf[bytes] = '\0';

	if ((temp = strrchr(buf, ':')) == NULL)
		return 1;
	while (*++temp && isspace((unsigned char)*temp))
		continue;

	if ((p = strpbrk(temp, "\r\n")))
		*p = '\0';

	strncpy(owner, temp, len);

	return 0;
}

static int
ftp_scan(int sock, const char *name)
{
	char	request[1024];
	int	code;

	/* Read ftp banner */
	(void)readall(sock);

	/* Send USER command */
	snprintf(request, sizeof(request), "USER %s\r\n", name);
	send(sock, request, strlen(request), 0);
	if (verbose_flag)
		printf(">>> %s", request);

	code = readcode(sock);

	/* Problem? */
	if (code < 0)
		return 2;

	/* User not logged in reply */
	if (code == 530)
		return 1;

	/* Send PASS command */
	strcpy(request, "PASS anonymous@pointless.nl\r\n");
	send(sock, request, strlen(request), 0);
	if (verbose_flag)
		printf(">>> %s", request);

	code = readcode(sock); 

	/* User not logged in reply */
	if (code == 530)
		return 1;

	/* User logged in reply */
	if (code == 230)
		return 0;

	return 2;
}

static int
relay_scan(int sock, const char *host, const char *ip, int flag)
{
	typedef struct {
	    char from[128];
	    char rcpt[128];
	} RELAY_INFO;
	char	   buf[1024];
	int	   i, n = 1;
	RELAY_INFO s[20];

	if (flag > 1)
		n = sizeof(s) / sizeof(RELAY_INFO);

	/*
	 * Setup info for the relay scan. The tests are from
	 * http://www.reedmedia.net/misc/mail/open-relay.html
	 *
	 * XXX - pointless.nl should be replaced with something else
	 */
	strcpy (s[0].from,  "<test@pointless.nl>");
	strcpy (s[0].rcpt,  "<test@pointless.nl>");
	strcpy (s[1].from,  "<test@localhost>");
	strcpy (s[1].rcpt,  "<test@pointless.nl>");
	strcpy (s[2].from,  "<test>");
	strcpy (s[2].rcpt,  "<test@pointless.nl>");
	strcpy (s[3].from,  "<>");
	strcpy (s[3].rcpt,  "<test@pointless.nl>");
	sprintf(s[4].from,  "<test@%s>", host);
	strcpy (s[4].rcpt,  "<test@pointless.nl>");
	sprintf(s[5].from,  "<test@[%s]>", ip);
	strcpy (s[5].rcpt,  "<test@pointless.nl>");
	sprintf(s[6].from,  "<test@%s>", host);
	sprintf(s[6].rcpt,  "<test%%pointless.nl@%s>", host);
	sprintf(s[7].from,  "<test@%s>", host);
	sprintf(s[7].rcpt,  "<test%%pointless.nl@[%s]>", ip);
	sprintf(s[8].from,  "<test@%s>", host);
	strcpy (s[8].rcpt,  "<\"test@pointless.nl\">");
	sprintf(s[9].from,  "<test@%s>", host);
	strcpy (s[9].rcpt,  "<\"test%pointless.nl\">");
	sprintf(s[10].from, "<test@%s>", host);
	sprintf(s[10].rcpt, "<test@pointless.nl@%s>", host);
	sprintf(s[11].from, "<test@%s>", host);
	sprintf(s[11].rcpt, "<\"test@pointless.nl\"@%s>", host);
	sprintf(s[12].from, "<test@%s>", host);
	sprintf(s[12].rcpt, "<test@pointless.nl@[%s]>", ip);
	sprintf(s[13].from, "<test@%s>", host);
	sprintf(s[13].rcpt, "<\"test@pointless.nl\"@[%s]>", ip);
	sprintf(s[13].from, "<test@%s>", host);
	sprintf(s[13].rcpt, "<@%s:test@pointless.nl>", host);
	sprintf(s[14].from, "<test@%s>", host);
	sprintf(s[14].rcpt, "<@[%s]:test@pointless.nl>", ip);
	sprintf(s[15].from, "<test@%s>", host);
	strcpy (s[15].rcpt, "<pointless.nl!test>");
	sprintf(s[16].from, "<test@[%s]>", ip);
	strcpy (s[16].rcpt, "<pointless.nl!test>");
	sprintf(s[17].from, "<test@%s>", host);
	sprintf(s[17].rcpt, "<pointless.nl!test@%s>", host);
	sprintf(s[18].from, "<test@%s>", host);
	sprintf(s[18].rcpt, "<pointless.nl!test@[%s]>", ip);
	sprintf(s[19].from, "<postmaster@%s>", host);
	strcpy (s[19].rcpt, "<test@pointless.nl>");

	/* Read banner */
	(void)readall(sock);

	/* Send HELO, quit if return code is not 250 */
	strcpy(buf, "HELO www.pointless.nl\r\n");
	send(sock, buf, strlen(buf), 0);
	if (verbose_flag)
		printf(">>> %s", buf);
	if (readcode(sock) != 250)
		return 2;

	/* Do requested tests */
	for (i = 0; i < n; i++) {
		if (verbose_flag)
			printf("\nRelay test %d\n", i+1);

		/* Reset */
		strcpy(buf, "RSET\r\n");
		send(sock, buf, strlen(buf), 0);
		if (verbose_flag)
			printf(">>> %s", buf);
		(void)readall(sock);

		/* Send MAIL FROM */
		snprintf(buf, sizeof(buf), "MAIL FROM: %s\r\n", s[i].from);
		send(sock, buf, strlen(buf), 0);
		if (verbose_flag)
			printf(">>> %s", buf);
		(void)readall(sock);

		/* Send RCPT TO */
		snprintf(buf, sizeof(buf), "RCPT TO: %s\r\n", s[i].rcpt);
		send(sock, buf, strlen(buf), 0);
		if (verbose_flag)
			printf(">>> %s", buf);

		/* 250 Ok = relay accepted */
		if (readcode(sock) == 250)
			return 0;

		if (n > 1)
			sleep(1);
	}

	return 1;
}

static int
http_proxy_scan(int sock)
{
	char	buf[1024];
	int	code;

	/* Send the connect request */
	strcpy(buf, "CONNECT www.pointless.nl:25 HTTP/1.0\r\n");
	send(sock, buf, strlen(buf), 0);
	if (verbose_flag)
		printf(">>> %s", buf);
	/* Add more \r\n */
	strcpy(buf, "\r\n");
	send(sock, buf, strlen(buf), 0);
	if (verbose_flag)
		printf(">>> %s", buf);

	/* The first line should contain the status code */
	readln(sock, buf, sizeof(buf));
	if (verbose_flag)
		printf("<<< %s", buf);
	if (sscanf(buf, "HTTP/1.%*[0-1] %d %*[a-zA-Z0-9 ]", &code) == 1) {
		/* 200 = Connection established */
		if (code == 200)
			return 0;
		else
			return 1;
	}

	return 2;
}

static int
socks_proxy_scan(int sock)
{
	typedef struct {
	    int		code;
	    const char *name;
	} SOCKS_REPLIES;
	SOCKS_REPLIES replies[] = {
	    { 0x00, "Connection succeeded" },
	    { 0x01, "General SOCKS server failure" },
	    { 0x02, "Connection not allowed by ruleset" },
	    { 0x03, "Network unreachable" },
	    { 0x04, "Host unreachable" },
	    { 0x05, "Connection refused" },
	    { 0x06, "TTL expired" },
	    { 0x07, "Command not supported" },
	    { 0x08, "Address type not supported" },
	    { 0xff, NULL }
	};
	unsigned char	buf[32];
	struct in_addr	daddr;
	uint16_t	port;
	int		len, i;

	/* XXX - this is my server */
	daddr.s_addr = inet_addr("62.194.156.128");
	port = htons(25);

	memset(&buf, 0, sizeof(buf));

	buf[0] = 0x05;	/* version: socks 5	     */
	buf[1] = 0x01;	/* number of methods	     */
	buf[2] = 0x00;	/* method: no authentication */

	/* Send authenication method packet */
	if (send(sock, buf, 3, 0) < 0)
		err(1, "send");
	if (verbose_flag)
		printf(">>> 0x05 0x01 0x00\n");

	/* Read reply */
	if ((i = recv(sock, buf, sizeof(buf) - 1, 0)) < 0)
		err(1, "recv");
	else if (i == 0)
		return 2;
	if (verbose_flag)
		printf("<<< 0x0%d 0x0%d\n", buf[0], buf[1]);

	/* Authentication method not accepted */
	if (buf[1] == 0xff)
		return 1;

	/* SOCKS v5 */
	if (buf[0] == 0x05) {
		memset(&buf, 0, sizeof(buf));

		buf[0] = 0x05;	/* Version: SOCKS 5   */
		buf[1] = 0x01;	/* Command: CONNECT   */
		buf[2] = 0x00;	/* Reserved           */
		buf[3] = 0x01;	/* Address type: IPv4 */

		len = 4;
		/* Destionation address */
		memcpy(buf + len, &daddr.s_addr, sizeof(daddr));
		len += sizeof(daddr);
		/* Destination port */
		memcpy(buf + len, &port, sizeof(port));
		len += sizeof(port);

		/* Request connection */
		if (send(sock, buf, len, 0) < 0)
			err(1, "send");
		if (verbose_flag)
			printf(">>> 0x05 0x01 0x00 0x01 ...\n");

		/* Read reply */
		if ((i = recv(sock, buf, sizeof(buf), 0)) < 0)
			err(1, "recv");
		else if (i == 0)
			return 2;

		if (verbose_flag) {
			printf("<<< 0x0%d 0x0%d\n", buf[0], buf[1]);
			for (i = 0; replies[i].code != 0xff; i++) {
				if (replies[i].code == buf[1]) {
					printf("socks: %s\n", replies[i].name);
					break;
				}
			}
		}

		/* 0x00 status means succeeded */
		if (buf[1] == 0x00)
			return 0;
		else
			return 1;
	}

	return 2;
}

static void
usage(const char *progname)
{
	fprintf(stderr,
"Usage: %s [options] hostname [port]\n"
"Options:\n"
"  -4          Force the use of IPv4 only.\n"
"  -6          Force the use of IPv6 only.\n"
"  -a          When resolved to multiple addresses, scan them all.\n"
"  -b          Grab the banner from an open port.\n"
"  -f          Anon FTP scan, checks if the server allows anonymous logins.\n"
"  -i          Ident scan, queries ident/auth (port 113) to get the\n"
"              identity of the service we're connecting to.\n"
#ifdef RESOLVE
"  -n          Don't try to resolve addresses to names.\n"
#else
"  -n          Try to resolve addresses to names.\n"
#endif
"  -p type     Open Proxy test, checks if the proxy allow connections.\n"
"  -q          Be quiet. Don't output anything to stdout.\n"
"  -r          Mail Relay test, performs a test to check for open-relay.\n"
"              Use twice for an extensive open-relay test.\n"
"  -t timeout  Specify a timeout, used when connecting to a service.\n"
"  -v          Be verbose. It's use is recommended.\n",
	progname);

	exit(64);
}

static void
fatal(int errornum)
{
	fprintf(stderr, "Fatal error: ");

	switch (errornum) {
	case EAI_AGAIN:
		fprintf(stderr,
		    "The name could not be resolved at this time.");
		break;
	case EAI_BADFLAGS:
		fprintf(stderr, "The flags had an invalid value.");
		break;
	case EAI_FAIL:
		fprintf(stderr, "A non-recoverable error occurred.");
		break;
	case EAI_FAMILY:
		fprintf(stderr,
		    "The address family was not recognized or the address "
		    "length was invalid for the specified family.");
		break;
	case EAI_MEMORY:
		fprintf(stderr, "There was a memory allocation failure.");
		break;
	case EAI_NONAME:
		fprintf(stderr,
		    "The name does not resolve for the supplied parameters.");
		break;
	case EAI_SERVICE:
		fprintf(stderr, "Unknown service name.");
		break;
	case EAI_SOCKTYPE:
		fprintf(stderr, "Unsupported socket type.");
		break;
	case EAI_SYSTEM:
		fprintf(stderr, "A system error occurred.");
		break;
	default:
		fprintf(stderr, "%s", gai_strerror(errornum));
	}

	fprintf(stderr, "\n");
	exit(128);
}
