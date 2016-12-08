/*  client.c
 *  SNTP      client
 *  Usage:    Accepts 1 or 3 arguments from command line.
 *
 *            Passing one argument will make the client switch to use the
 *            default NTP server and default NTP port.
 *
 *            Accepts three arguments in the form:
 *            ./client hostname portno
 *            allows the user to specify a valid NTP server hostname and portno
 *
 *  Author:   Paul Davis
 *  Date:     08/12/2016
 */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include "sntp.h"
#include "client_functions.h"

/* Default port and server to be used if none specified */
#define PORT 123
#define SERVER "0.uk.pool.ntp.org"

/* #define PORT 63333
  #define SERVER "ntp.uwe.ac.uk" */

int main(int argc, char *argv[]) {
  int sockfd, portno, numbytes;
  struct hostent *he;            /* server data struct */
  struct sockaddr_in their_addr; /* server addr info */
  char host[255];

  /* Check args */
  if (argc == 3) {
    strcpy(host, argv[1]);
    portno = atoi(argv[2]);
  }
  else if (argc == 1) {
    strcpy(host, SERVER);
    portno = PORT;
    printf("Server and port not specified.\n");
    printf("Using default server: %s\nUsing default port: %d", host, portno);
  }
  else {
    fprintf(stderr, "usage: %s hostname port\n", argv[0]);
    exit(1);
  }
  /* End check args */

  /* Resolve server host name or IP address */
  if((he = gethostbyname(host)) == NULL) {
    perror("client gethostbyname");
    exit(1);
  }

  /* Create socket */
  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("client socket");
    exit(1);
  }

  /* Set timeout for sendto and recvfrom socket functions */
  tv.tv_sec = 10;
  tv.tv_usec = 0;
  if((setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))) < 0) {
    printf("socket timeout error no: %d", errno);
  }

  printf("\nSending..\n");

  /* Zero server address struct */
  memset(&their_addr, 0, sizeof(their_addr));
  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(portno);
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);

  /* Initialise and zero NTP send and receive packet struct */
  ntp_packet packet;
  memset(&packet, 0, sizeof(packet));
  ntp_packet recvBuf;
  memset(&recvBuf, 0, sizeof(recvBuf));

  /* Set contents of unicast client msg */
  set_client_request(&packet);

  /* Send packet */
  if((numbytes = sendto(sockfd, &packet, sizeof(ntp_packet), 0,
      (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
    perror("client sendto");
    exit(1);
  }

  /* Receive packet */
  socklen_t addr_len = (socklen_t)sizeof(struct sockaddr);
  if((numbytes = recvfrom(sockfd, &recvBuf, sizeof(ntp_packet), 0,
  (struct sockaddr *) &their_addr, &addr_len)) == -1) {
    perror("server recvfrom");
    exit(1);
  }

  /* Network to host byte order */
  network_to_host(&recvBuf);

  printf("\n\nReceived:\n");

  /* Destination timestamp created on packet arrival for use in offset and delay */
  ntp_timestamp destTimestamp = getCurrentTimestamp();

  /* Perform basic checks to check validity of server reply */
  check_reply(&packet, &recvBuf);

  /* Calculate offset and delay using received packet and destination timestamp */
  double offset = calculate_offset(&recvBuf, &destTimestamp);
  double delay = calculate_delay(&recvBuf, &destTimestamp);

  /* Print formatted output */
  print_sntp_output(&recvBuf, offset, delay, their_addr, host);

  print_ntp_packet(&recvBuf);

  return 0;
}
