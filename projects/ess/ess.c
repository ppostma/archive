/*
 * Copyright (c) 2003 Peter Postma <peter@webdeveloping.nl>
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
 * $Id: ess.c,v 1.38 2003-11-06 22:41:45 peter Exp $
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VERSION		"0.3.5-beta"
#define HTTP_REQUEST	"HEAD / HTTP/1.0\r\n\r\n"
#define CONNECT_TIMEOUT	3	/* seconds when connect will timeout */
#define BANNER_TIMEOUT	1	/* seconds after last banner recv    */
#define BANNER_SIZE	2048	/* max recv size for banner	     */

size_t	 readln(int, char *, size_t);
size_t	 readall(int);
int	 readcode(int);
char	*get_af(int);
char	*get_addr(struct sockaddr *, socklen_t, int);
char	*get_serv(char *);
void	 banner_scan(u_short);
int	 ident_scan(char *, int, u_short, u_short, char *, size_t);
int	 ftp_scan(char *);
int	 relay_scan(char *, char *);
void	 timeout_handler(int);
void	 usage(char *);
void	 fatal(int);

int	ssock, isock;
int	timedout = 0;
int	verbose_flag = 0;
int	banner_flag = 0;
int	relay_flag = 0;
int	quiet_flag = 0;

#ifdef IPV4_DEFAULT
int	IPv4or6 = AF_INET;
#else
int	IPv4or6 = AF_UNSPEC;
#endif

#ifdef RESOLVE
int	resolve_flag = 1;
#else
int	resolve_flag = 0;
#endif

int
main(int argc, char *argv[])
{
	struct sockaddr_storage	ss;
	struct addrinfo	 hints, *res, *ai;
	struct servent	*serv;
	char		*progname;
	char		*host = NULL;
	char		*port = NULL;
	char		 ip[NI_MAXHOST];
	char		 name[NI_MAXHOST];
	char		 sbuf[NI_MAXSERV];
	char		 result[128], owner[128];
	int		 ch, err, ret = 70;
	int		 all_flag = 0;
	int		 ftp_flag = 0;
	int		 ident_flag = 0;
	u_short		 remoteport, localport;
	socklen_t	 len;

	progname = argv[0];

	while ((ch = getopt(argc, argv, "46abfhinqrvV")) != -1) {
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
			banner_flag++;
			break;
		case 'f':
			ftp_flag = 1;
			port = "21";
			break;
		case 'i':
			ident_flag = 1;
			break;
		case 'n':
			resolve_flag = (resolve_flag) ? 0 : 1;
			break;
		case 'q':
			if (verbose_flag) {
				fprintf(stderr,
				    "Options -v (verbose) and -q (quiet) "
				    "cannot be used together!\n");
				exit(65);
			}
			quiet_flag = 1;
			break;
		case 'r':
			relay_flag++;
			port = "25";
			break;
		case 'v':
			if (quiet_flag) {
				fprintf(stderr,
				    "Options -v (verbose) and -q (quiet) "
				    "cannot be used together!\n");
				exit(65);
			}
			verbose_flag = 1;
			break;
		case 'V':
			fprintf(stderr, "Easy Service Scan v%s by Peter Postma "
					"<peter@webdeveloping.nl>\n", VERSION);
			exit(99);
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
	if (argc < 2 && (port == NULL || host == NULL))
		usage(progname);

	if (port == NULL || argv[1] != NULL)
		port = argv[1];

	signal(SIGALRM, &timeout_handler);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = IPv4or6;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo(host, port, &hints, &res);
	if (err)
		fatal(err);

	if (res->ai_next != NULL && !all_flag && !quiet_flag)
		printf("Resolved to multiple addresses! "
		       "Use option -a to scan them all.\n");

	for (ai = res; ai != NULL; ai = ai->ai_next) {
		if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6)
			continue;
		ssock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (ssock < 0) {
			perror("socket");
			continue;
		}
		if (verbose_flag) {
			strncpy(ip, get_addr(ai->ai_addr, ai->ai_addrlen, 0),
			    sizeof(ip));
			strncpy(name, get_addr(ai->ai_addr, ai->ai_addrlen, 1),
			    sizeof(name));
			printf("Trying %s", name);
			if (strcmp(ip, name) != 0)
				printf(" (%s)...", ip);
			else
				printf("...");
			fflush(stdout);
		}
		alarm(CONNECT_TIMEOUT);
		if (connect(ssock, ai->ai_addr, ai->ai_addrlen) < 0) {
			if (verbose_flag)
				printf(" connection failed!\n");
			if (timedout) {
				strcpy(result, "no response");
				timedout = 0;
				ret = 2;
			} else {
				strcpy(result, "closed");
				ret = 1;
			}
			goto print_results;
		}
		alarm(0);
		ret = 0;
		if (verbose_flag)
			printf(" connection successful!\n");
		if (ident_flag) {
			len = sizeof(ss);
			if (getsockname(ssock,(struct sockaddr *)&ss,&len) <0) {
				perror("getsockname");
				exit(255);
			}
			err = getnameinfo((struct sockaddr *)&ss, len, NULL, 0,
			    sbuf, sizeof(sbuf), NI_NUMERICSERV);
			if (err)
				fatal(err);
			serv = getservbyname(port, "tcp");
			if (serv != NULL)
				remoteport = ntohs(serv->s_port);
			else
				remoteport = atoi(port);
			localport = atoi(sbuf);
			strncpy(ip, get_addr(ai->ai_addr, ai->ai_addrlen, 0),
			    sizeof(ip));
			memset(&owner, 0, sizeof(owner));
			ret = ident_scan(ip, ai->ai_family, remoteport,
			    localport, owner, sizeof(owner));
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
			ret = ftp_scan("anonymous");
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
				ret = relay_scan(name, ip);
			} else
				ret = relay_scan(NULL, NULL);

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
		} else if (banner_flag) {
			strcpy(result, "banner:\n");
		} else
			strcpy(result, "open");

print_results:
		/* Print af, address, port, and result */
		if (!quiet_flag) {
			printf("%s host (%s) ", get_af(ai->ai_family),
			   get_addr(ai->ai_addr, ai->ai_addrlen, resolve_flag));
			printf("port %s -> %s\n", get_serv(port), result);

			/* Print banner at last */
			if (banner_flag && ret == 0) {
				serv = getservbyname(port, "tcp");
				if (serv != NULL)
					banner_scan(ntohs(serv->s_port));
				else
					banner_scan(atoi(port));
			}
		}

		if (!all_flag)
			break;
	}

	close(ssock);
	freeaddrinfo(res);

	return ret;
}

size_t
readln(int fd, char *line, size_t len)
{
	ssize_t	b, i = 0;
	char	temp[1];

	do {
		if ((b = recv(fd, temp, 1, 0)) < 0) {
			return b;
		} else if (b == 0)
			break;
		if (temp[0] != 0)
			line[i] = temp[0];
	} while (++i < len && temp[0] != '\n');

	if (i > 0)
		line[i] = '\0';

	return i;
}

size_t
readall(int fd)
{
	char	response[1024];
	size_t	b, count = 0;

	do {
		if ((b = readln(fd, response, sizeof(response))) > 0)
			count += b;
		if (verbose_flag)
			printf("<<< %s", response);
	} while (isdigit((int)response[0]) &&
		 isdigit((int)response[1]) &&
		 isdigit((int)response[2]) &&
		 response[3] == '-');

	return count;
}

int
readcode(int fd)
{
	char	response[1024];
	char	code[4];

	do {
		(void)readln(fd, response, sizeof(response));
		if (verbose_flag)
			printf("<<< %s", response);
	} while (response[3] == '-');

	if (isdigit((int)response[0]) &&
	    isdigit((int)response[1]) &&
	    isdigit((int)response[2])) {
		strncpy(code, response, 3);
		return atoi(code);
	}

	return -1;
}

char *
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

char *
get_addr(struct sockaddr *addr, socklen_t len, int resolve)
{
	struct sockaddr_storage	ss;
	static char	host[NI_MAXHOST];

	memcpy(&ss, addr, (size_t)len);
	if (getnameinfo((struct sockaddr *)&ss, len, host,
	    sizeof(host), NULL, 0, (resolve) ? 0 : NI_NUMERICHOST) == 0)
		return host;

	return "?";
}

char *
get_serv(char *port)
{
	struct servent	*serv;
	static char	 buf[NI_MAXSERV];
	unsigned int	 i;

	for (i=0; i<(strlen(port)); i++)
		if (isalpha((int)port[i]) != 0)
			goto name;

	serv = getservbyport(htons(atoi(port)), "tcp");
	if (serv != NULL)
		snprintf(buf, sizeof(buf), "%s (%s)", port, serv->s_name);
	else
		snprintf(buf, sizeof(buf), "%s", port);
	return buf;

name:
	serv = getservbyname(port, "tcp");
	if (serv != NULL)
		snprintf(buf, sizeof(buf), "%u (%s)", ntohs(serv->s_port),port);
	else
		snprintf(buf, sizeof(buf), "%s", port);
	return buf;
}

void
banner_scan(u_short port)
{
	struct timeval	 tv;
	fd_set		 read_fds;
	char		*buf, *save;
	int		 count;
	char		 ch;

	tv.tv_sec = BANNER_TIMEOUT;
	tv.tv_usec = 0;

	FD_ZERO(&read_fds);
	FD_SET(ssock, &read_fds);

	if ((buf = (char *)malloc(BANNER_SIZE)) == NULL) {
		perror("malloc");
		exit(255);
	}

	if (port == 80)
		send(ssock, HTTP_REQUEST, sizeof(HTTP_REQUEST), 0);

	for (;;) {
		select(ssock+1, &read_fds, NULL, NULL, &tv);
		if (!FD_ISSET(ssock, &read_fds))
			goto cleanup;
		count = recv(ssock, buf, sizeof(buf), 0);
		if (count < 1)
			goto cleanup;
		buf[count] = '\0';
		if (banner_flag > 1) {
			printf("%s", buf);
			fflush(stdout);
			continue;
		}
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
			else if (isprint((int)ch))
				printf("%c", ch);
			else
				printf("\\%03d", (int)ch);
		}
		fflush(stdout);
	}

cleanup:
	free(buf);
	return;
}

int
ident_scan(char *ip, int ai_family, u_short remoteport, u_short localport,
	   char *owner, size_t len)
{
	struct addrinfo	 hints, *res;
	char		*temp, *p;
	char		 request[16], response[1024];
	int		 err, bytes;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = ai_family;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo(ip, "113", &hints, &res);
	if (err)
		fatal(err);
	if (res->ai_next != NULL)
		return 1;

	isock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (isock < 0)
		return 2;

	alarm(CONNECT_TIMEOUT);
	if (connect(isock, res->ai_addr, res->ai_addrlen) < 0) {
		close(isock);
		return 2;
	}
	alarm(0);

	snprintf(request, sizeof(request), "%u,%u\r\n", remoteport, localport);
	send(isock, request, strlen(request), 0); 
	bytes = recv(isock, response, sizeof(response), 0);
	if (bytes < 0)
		return 255;
	else if (bytes == 0)
		return 1;

	close(isock);
	freeaddrinfo(res);

	response[bytes] = '\0';

	temp = strrchr(response, ':');
	while (*(++temp) == ' ')
		;

	if ((p = strchr(temp, '\r')))
		*p = '\0';
	if ((p = strchr(temp, '\n')))
		*p = '\0';

	strncpy(owner, temp, len);

	return 0;
}

int
ftp_scan(char *name)
{
	char	 request[1024];
	int	 code;

	/* Read ftp banner */
	(void)readall(ssock);

	/* Send USER command */
	snprintf(request, sizeof(request), "USER %s\r\n", name);
	send(ssock, request, strlen(request), 0);
	if (verbose_flag)
		printf(">>> %s", request);

	/* User not logged in reply */
	if (readcode(ssock) == 530)
		return 1;

	/* Send PASS command */
	strcpy(request, "PASS anonymous@moo.com\r\n");
	send(ssock, request, strlen(request), 0);
	if (verbose_flag)
		printf(">>> %s", request);

	code = readcode(ssock); 

	/* User not logged in reply */
	if (code == 530)
		return 1;

	/* User logged in reply */
	if (code == 230)
		return 0;

	return 2;
}

int
relay_scan(char *host, char *ip)
{
	char	 request[1024];
	struct	 scans {
	    char	from[128];
	    char	rcpt[128];
	};
	struct	 scans	s[20];
	int	 i, n = 1;

	if (relay_flag > 1)
		n = sizeof(s) / sizeof(struct scans);

	/*
	 * Setup info for the relay scan. Tests from
	 * http://www.reedmedia.net/misc/mail/open-relay.html
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
	(void)readall(ssock);

	/* Send HELO, quit if return code is not 250 */
	strcpy(request, "HELO www.pointless.nl\r\n");
	send(ssock, request, strlen(request), 0);
	if (verbose_flag)
		printf(">>> %s", request);
	if (readcode(ssock) != 250)
		return 2;

	/* Do requested tests */
	for (i=0; i<n; i++) {

		if (verbose_flag)
			printf("\nRelay test %d\n", i+1);

		/* Reset */
		strcpy(request, "RSET\n");
		send(ssock, request, strlen(request), 0);
		if (verbose_flag)
	                printf(">>> %s", request);
		(void)readall(ssock);

		/* Send MAIL FROM */
		snprintf(request, sizeof(request), "MAIL FROM: %s\r\n",
		   s[i].from);
		send(ssock, request, strlen(request), 0);
		if (verbose_flag)
			printf(">>> %s", request);
		(void)readall(ssock);

		/* Send RCPT TO */
		snprintf(request, sizeof(request), "RCPT TO: %s\r\n",
		   s[i].rcpt);
		send(ssock, request, strlen(request), 0);
		if (verbose_flag)
			printf(">>> %s", request);

		/* 250 Ok = relay accepted */
		if (readcode(ssock) == 250)
			return 0;

		if (n > 1)
			sleep(1);
	}

	return 1;
}

void
timeout_handler(int s)
{
	timedout = 1;
	close(ssock);
}

void
usage(char *progname)
{
	fprintf(stderr,
"Usage: %s [options] hostname port\n"
"Options:\n"
"  -4      Force the use of IPv4 only.\n"
"  -6      Force the use of IPv6 only.\n"
"  -a      When resolved to multiple addresses, scan them all.\n"
"  -b      Grab the banner from an open port.  If you specify this\n"
"          option twice then special characters won't be translated.\n"
"  -f      Anonymous FTP scan, checks if the server allows anonymous logins.\n"
"  -i      Ident scan, queries ident/auth (port 113) and tries to get the\n"
"          identity of the service we're connecting to.\n"
#ifdef RESOLVE
"  -n      Don't try to resolve addresses to names.\n"
#else
"  -n      Don't try to resolve names to addresses.\n"
#endif
"  -q      Be quiet. Don't output anything to stdout.\n"
"  -r      Mail Relay test, performs a simple test to check for open-relay.\n"
"          Use twice for an extensive open-relay test.\n"
"  -v      Be verbose. It's use is recommended.\n\n",
	progname);

	exit(64);
}

void
fatal(int errornum)
{
	fprintf(stderr, "Fatal error: ");

	switch (errornum) {
	case EAI_AGAIN:
		fprintf(stderr,
		    "The name could not be resolved at this time.\n");
		break;
	case EAI_BADFLAGS:
		fprintf(stderr, "The flags had an invalid value.\n");
		break;
	case EAI_FAIL:
		fprintf(stderr, "A non-recoverable error occurred.\n");
		break;
	case EAI_FAMILY:
		fprintf(stderr,
		    "The address family was not recognized or the address "
		    "length was invalid for the specified family.\n");
		break;
	case EAI_MEMORY:
		fprintf(stderr, "There was a memory allocation failure.\n");
		break;
	case EAI_NONAME:
		fprintf(stderr,
		    "The name does not resolve for the supplied parameters.\n");
		break;
	case EAI_SERVICE:
		fprintf(stderr, "Unknown service name.\n");
		break;
	case EAI_SOCKTYPE:
		fprintf(stderr, "Unsupported socket type.\n");
		break;
	case EAI_SYSTEM:
		fprintf(stderr, "A system error occurred.\n");
		break;
	default:
		fprintf(stderr, "%s\n", gai_strerror(errornum));
	}
	exit(128);
}
