/* $Id: scan.c,v 1.4 2003-03-30 16:24:38 peter Exp $ */

/*
 * scan.c - very simple portscanner for IPv4 & IPv6
 *
 * scans a port and returns the status of the scanned port.
 * syntax: scan [-b] <host/ip> <port/service>
 *
 * by Peter Postma <peter@webdeveloping.nl>
 */

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

#define TIMEOUT 2500000			/* 2500 milli-seconds */

int sockid;				/* socket() */
int timedout = 0;

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

int main(int argc, char *argv[])
{
	int opt, error, port, banner = 0, v6 = 0;
	char strport[NI_MAXSERV];
	char buf[NI_MAXSERV * 2];
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
		exit(0);

	port = atoi(argv[1]);

	/* Set alarm signal handler */
	signal(SIGALRM, timeout_handler);

	/* Translate address */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = (v6) ? AF_INET6 : AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((error = getaddrinfo(argv[0], argv[1], &hints, &res)) != 0) {
		printf("FAILED (%s)\n", gai_strerror(error));
		exit(0);
	}

	/* Build the socket */
	sockid = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockid < 0) {
		printf("FAILED (error building socket)\n");
		exit(0);
	}

	/* Alarm call after X seconds */
	ualarm(TIMEOUT, 0);

	/* Get service name */
	if ((error = getnameinfo(res->ai_addr, res->ai_addrlen, NULL, 0,
	    strport, sizeof(strport), 0) == 0)) {
		if ((port < 0) || (strlen(strport) == 0))
			snprintf(buf, sizeof(buf), "%d (unknown)", port);
		else if ((port == 0) || (port == atoi(strport)))
			snprintf(buf, sizeof(buf), "%s (unknown)", strport);
		else
			snprintf(buf, sizeof(buf), "%d (%s)", port, strport);
	}

	/* Connect to the host */
	if (connect(sockid, res->ai_addr, res->ai_addrlen) < 0) {
		if (timedout)
			printf("TIMEOUT (%s)\n", buf);
		else
			printf("CLOSED (%s)\n", buf);
	} else {
		if (banner) {
			grab_banner(port);
			if (timedout)
				printf("NOBANNER (%s)\n", buf);
		} else 
			printf("OPEN (%s)\n", buf);
	}

	close(sockid);
	freeaddrinfo(res);

	return (0);
}
