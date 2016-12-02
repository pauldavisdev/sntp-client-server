/*  client.c - SNTP client
 *
 */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include "sntp.h"

/* Default port and server to be used if none specified */
#define PORT 123         /* server port the client connects to */
#define SERVER "0.uk.pool.ntp.org"

/* #define PORT 63333
  #define SERVER "ntp.uwe.ac.uk" */

/* Client functions*/
void set_client_request(ntp_packet *p);
void print_sntp_output(ntp_packet *p, double offset, double delay,
                       struct sockaddr_in their_addr, char *host);
void check_reply(ntp_packet *p, ntp_packet *r);

int main(int argc, char *argv[]) {
  int sockfd, portno, numbytes;
  struct hostent *he;            /* server data struct */
  struct sockaddr_in their_addr; /* server addr info */
  socklen_t addr_len = (socklen_t)sizeof(struct sockaddr);
  char host[255];

  /* Checks args */
  if (argc == 3) {
    strcpy(host, argv[1]);
    portno = atoi(argv[2]);
  }
  else if (argc == 1) {
    strcpy(host, SERVER);
    portno = PORT;
  }
  else {
    fprintf(stderr, "usage: %s hostname port\n", argv[0]);
    exit(1);
  }

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

  printf("Packet sent!");

  /* Receive packet */
  if((numbytes = recvfrom(sockfd, &recvBuf, sizeof(ntp_packet), 0,
  (struct sockaddr *) &their_addr, &addr_len)) == -1) {
    perror("server recv from");
    exit(1);
  }

  /* Network to host byte order */
  network_to_host(&recvBuf);

  printf("\n\nReceived:\n");

  /* Perform basic checks to check validity of server reply */
  check_reply(&packet, &recvBuf);

  /* Destination timestamp created on packet arrival for use in offset and delay */
  ntp_timestamp destTimestamp = getCurrentTimestamp();

  /* Calculate offset and delay using received packet and destination timestamp */
  double offset = calculate_offset(&recvBuf, &destTimestamp);
  double delay = calculate_delay(&recvBuf, &destTimestamp);

  /* Print formatted output */
  print_sntp_output(&recvBuf, offset, delay, their_addr, host);

  return 0;
}

/* Client functions*/

void set_client_request(ntp_packet *p)
{
  /* set up flag bitfield of struct, so li is 0, vn is 4, mode is 3 */
  // li
  p->flags = 0;
  p->flags <<= 3;
  // vn
  p->flags |= 4;
  p->flags <<= 3;
  // mode
  p->flags |= 3;

  /* transmit timestamp set to current time in NTP timestamp format */
  p->transmitTimestamp = getCurrentTimestamp();

  /* host to network byte order */
  host_to_network(p);
}

void print_sntp_output(ntp_packet *p, double offset, double delay,
  struct sockaddr_in their_addr, char* host)
{
  /* get mode as int to print */
  int mode = p->flags & 0x07;

  print_unix_time(&tv);

  /* print offset and delay */
  printf("%+f +/- %f s%d ", offset, delay, p->stratum);

  /* server name to address and print */
  char str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(their_addr.sin_addr), str, INET_ADDRSTRLEN);
  printf("%s %s\n", host, str);

  printf("Mode: %d\n", mode);
}

void check_reply(ntp_packet *p, ntp_packet *r)
{
  network_to_host(p);

  if((p->transmitTimestamp.second != r->orgTimestamp.second) |
  (p->transmitTimestamp.fraction != r->orgTimestamp.fraction)) {
    perror("Invalid server reply - Originate timestamp");
    exit(1);
  }

  if((r->flags & 0x07) != 4) {
    perror("Invalid server reply - Mode");
    exit(1);
  }

  if((r->stratum) == 0) {
    perror("Invalid server reply - Stratum is 0");
    exit(1);
  }

  if((r->transmitTimestamp.second && r->transmitTimestamp.fraction) == 0) {
    perror("Invalid server reply - Transmit timestamp is 0");
    exit(1);
  }
}
