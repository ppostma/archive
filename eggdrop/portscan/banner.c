/* $Id: banner.c,v 1.2 2003-03-21 02:07:10 peter Exp $ */

/*
 * banner.c - banner grabber for IPv4
 *
 * grabs the banner on an open port
 * syntax: banner <host/ip> <port no.>
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

/* This is only used as request to port 80, we need it to grab the banner */ 
#define HTTP_REQUEST "HEAD / HTTP/1.0\n\n" 

/* Timeout handler, if our request doesn't get response,
   then we assume the port is in stealth mode */
void timeout_handler(int s) {
  printf("%s\n", "stealth");
  exit(0);
}

int main(int argc, char **argv) {
  int sockid, conn, port;
  char buffer[4096];
  struct hostent *hostaddr;
  struct sockaddr_in socketaddr;

  /* Check input */
  if (argc < 3) {
    printf("syntax: %s <host/ip> <port>\n", argv[0]);
    exit(0); 
  } else {
     port = atoi(argv[2]);
  }
  
  /* Resolve the host name. */
  hostaddr = gethostbyname(argv[1]);
  if (!hostaddr) {
    printf("%s\n", "Error resolving hostname");
    exit(0);  
  }

  /* Set signal for alarm call */
  signal(SIGALRM, timeout_handler);
                             
  /* Build our socket */
  sockid = socket(PF_INET, SOCK_STREAM, 0);
  
  /* Setup info about the remote host */
  memset(&socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;
  socketaddr.sin_port = htons(port);
     
  /* Copy address from hostaddr to socketaddr */                                 
  memcpy(&socketaddr.sin_addr, hostaddr->h_addr, hostaddr->h_length);

  /* Alarm call after 5 seconds, we don't want to timeout forever ;) */
  alarm(5);

  /* Connect to the host */                                             
  conn = connect(sockid, (struct sockaddr *) &socketaddr, sizeof(socketaddr));
                                               
  /* Grab the banner if connection was succesfull */
  if (conn >= 0) { 
    switch (port) { /* different ports and different requests */
      case 80: write(sockid, HTTP_REQUEST, strlen(HTTP_REQUEST));
               break;  
      default: write(sockid, "", 0);
    }
    write(1, buffer, read(sockid, buffer, sizeof(buffer))); /* read results */
  }
  else 
    printf("%s\n", "closed");
 
  /* Close connection */
  if (conn >= 0) close(conn);
  close(sockid);
        
  return 0;
}
                                                                                                 
