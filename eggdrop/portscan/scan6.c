/* $Id: scan6.c,v 1.2 2003-03-21 02:07:10 peter Exp $ */

/*
 * scan6.c - very simple portscanner for IPv6 
 *
 * scans a port and returns the status of the scanned port.
 * syntax: scan <host/ip> <port no.>
 *
 * by Peter Postma <peter@webdeveloping.nl>
 */

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>

/* Timeout handler, if our request doesn't get response, 
   then we assume the port is in stealth mode */
void timeout_handler(int s) {
  printf("%s\n", "stealth");
  exit(0);
}

int main(int argc, char *argv[]) {
  int sockid, conn, port;
  struct servent *service;
  struct hostent *hostaddr;
  struct sockaddr_in6 socketaddr;

  /* Check input */
  if (argc < 3) {
    printf("syntax: %s <host/ip> <port>\n", argv[0]);
    exit(0);
  } else {
    port = atoi(argv[2]);
  }

  /* Resolve the host name  */
  hostaddr = gethostbyname2(argv[1], AF_INET6);
  if (!hostaddr) {
    printf("%s\n", "Error resolving hostname");
    exit(0);
  } 
  
  /* Set signal for alarm call */
  signal(SIGALRM, timeout_handler);

  /* Build our socket */
  sockid = socket(PF_INET6, SOCK_STREAM, 0);
  
  /* Setup info about the remote host */
  memset(&socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin6_family = AF_INET6;
  socketaddr.sin6_port = htons(port);
     
  /* Alarm call after 5 seconds, we don't want to timeout forever ;) */
  alarm(5);
       
  /* Copy address from hostaddr to socketaddr */
  memcpy(&socketaddr.sin6_addr, hostaddr->h_addr, hostaddr->h_length);

  /* Connect to the host */
  conn = connect(sockid, (struct sockaddr *) &socketaddr, sizeof(socketaddr));
                                           
  /* Check if connection was successfull */
  if (conn >= 0) {
    printf("%i%s", port, " => ");
    service = getservbyport(htons(port), "tcp");
    if (service)
      printf("%s\n", service->s_name);
    else
      printf("%s\n", "unknown");
  } 
  else
    printf("%s\n", "closed");

  /* Close connection */
  if (conn >= 0) close(conn);
  close(sockid);
        
  return 0;
}
                                                                                                 
