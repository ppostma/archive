/* $Id: scan.c,v 1.6 2003-06-22 12:39:10 peter Exp $ */

/*
 * scan.c - very simple portscanner for IPv4 & IPv6
 *
 * scans a port and returns the status of the scanned port.
 * syntax: scan [-6] [-b] <host/ip> <port/service>
 *
 * by Peter Postma <peter@webdeveloping.nl>
 */

#include <ctype.h>
#include <errno.h>            
#include <netdb.h>
#include <signal.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HTTP_REQUEST "HEAD / HTTP/1.0\n\n"

#define ARGC	5000			/* Symbolic values */
#define SOCKET	6000

#define TIMEOUT	2500000			/* 2500 milli-seconds */

int sockid;				/* socket() */
int timedout = 0;

static void fail(int errnum)
{
	printf("FAILED ");
	
	switch (errnum) {
	case ARGC:
		printf("(Syntax: scan host/ip port/service)\n");
		break;
	case SOCKET:
		printf("(Problem building socket)\n");
		break;
	case EAI_SYSTEM:
		printf("(A system error occurred)\n");
		break;
	case EAI_FAIL:
		printf("(A non-recoverable error occurred)\n");
		break;
	case EAI_MEMORY:
		printf("(There was a memory allocation failure)\n");
		break;
	case EAI_AGAIN:
		printf("(The name could not be resolved at this time)\n");
		break;
	case EAI_NONAME:
		printf("(The name does not resolve for the supplied parameters)\n");
		break;
	case EAI_SERVICE:
		printf("(Unknown service name)\n");
		break;
	case EAI_SOCKTYPE:
		printf("(Unsupported socket type)\n");
		break;
	default:
		printf("(%s)\n", gai_strerror(errnum));
	}
	exit(0);
}

static void grab_banner(int port)
{
	char buf[4096];

	switch (port) {
	case 80:
		write(sockid, HTTP_REQUEST, strlen(HTTP_REQUEST));
		break;
	default:
		write(sockid, "", 0);
		break;
	}
	write(1, buf, read(sockid, buf, sizeof(buf)));
}

static void timeout_handler(int s)
{
	timedout = 1;
	close(sockid);	
}

static char *getserv(char *port)
{
	struct servent *service;
	static char buf[NI_MAXSERV];
	int i;

	for (i=0; i<(strlen(port)); i++)
		if (isalpha((int)port[i]) != 0)
			goto name;

	service = getservbyport(htons(atoi(port)), "tcp");
	if (service != NULL)
		snprintf(buf, sizeof(buf), "%s (%s)", port, service->s_name);
	else
		snprintf(buf, sizeof(buf), "%s", port);
	return buf;
name:
	service = getservbyname(port, "tcp");
	if (service != NULL)
		snprintf(buf, sizeof(buf), "%u (%s)", ntohs(service->s_port), port);
	else
		snprintf(buf, sizeof(buf), "%s", port);
	return buf;
}

int main(int argc, char *argv[])
{
	int opt, error, banner = 0, v6 = 0;
	struct addrinfo hints, *res;

	/* Parse command-line arguments. */
	while ((opt = getopt(argc, argv, "6bv")) != -1) {
		switch (opt) {
		case '6':
			v6 = 1;
			break;
		case 'b':
		case 'v':
			banner = 1;
			break;
		}
	}
	argc -= optind;
	argv += optind;

	/* Need a hostname/ip and a port/service */
	if (argc != 2)
		fail(ARGC);

	/* Set alarm signal handler */
	signal(SIGALRM, timeout_handler);

	/* Setup Address info */
	bzero((char *)&hints, sizeof(hints));
	hints.ai_family = (v6) ? AF_INET6 : AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((error = getaddrinfo(argv[0], argv[1], &hints, &res)) != 0)
		fail(error);

	/* Build the socket */
	sockid = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockid < 0)
		fail(SOCKET);

	/* Alarm call after X seconds */
	ualarm(TIMEOUT, 0);

	/* Connect to the host */
	if (connect(sockid, res->ai_addr, res->ai_addrlen) < 0) {
		if (timedout)
			printf("TIMEOUT (%s)\n", getserv(argv[1]));
		else
			printf("CLOSED (%s)\n", getserv(argv[1]));
	} else {
		if (banner) {
			grab_banner(atoi(argv[1]));
			if (timedout)
				printf("NOBANNER (%s)\n", getserv(argv[1]));
		} else
			printf("OPEN (%s)\n", getserv(argv[1]));
	}

	close(sockid);
	freeaddrinfo(res);

	return (0);
}
