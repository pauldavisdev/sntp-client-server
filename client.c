/*  client.c - a datagram 'client'
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include "sntp.h"

#define PORT 123         /* server port the client connects to */

int main(int argc, char const *argv[]) {
  int sockfd, numbytes;
  struct hostent *he;            /* server data struct */
  struct sockaddr_in their_addr; /* server addr info */

  // NTP server address
  char serverIP[] = "0.uk.pool.ntp.org";

  // Initialise NTP packet struct
  ntp_packet packet;

  // zero packet struct
  memset(&packet, 0, sizeof(packet));

  /* set up flag bitfield of struct, so li is 0, vn is 4, mode is 3 */
  // li
  packet.flags = 0;
  packet.flags <<= 3;
  // vn
  packet.flags |= 4;
  packet.flags <<= 3;
  // mode
  packet.flags |= 3;


  /*if(argc != 2) {
    fprintf(stderr, "usage: client serverIP\n");
    exit(1);
  }*/

  /* resolve server host name or IP address */
  if((he = gethostbyname(serverIP)) == NULL) {
    perror("client gethostbyname");
    exit(1);
  }

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("client socket");
    exit(1);
  }

  printf("\nSending..\n\n");

  gettimeofday(&tv, NULL);
  print_unix_time(&tv);

  memset(&their_addr, 0, sizeof(their_addr));
  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(PORT);
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);

  packet.transmitTimestamp = getCurrentTimestamp();

/*  printf("Reference:\t");
  print_ntp_time(&packet.refTimestamp);
  printf("Originate:\t");
  print_ntp_time(&packet.orgTimestamp);
  printf("Receive:\t");
  print_ntp_time(&packet.recvTimestamp);*/
  printf("\nTransmit:\t");
  print_ntp_time(&packet.transmitTimestamp);

  host_to_network(&packet);

  if((numbytes = sendto(sockfd, &packet, sizeof(ntp_packet), 0,
      (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
    perror("client sendto");
    exit(1);
  }

  printf("\n\n\nReceiving..\n");

  socklen_t addr_len = (socklen_t)sizeof(struct sockaddr);

  ntp_packet recvBuf;
  memset(&recvBuf, 0, sizeof(recvBuf));

  if((numbytes = recvfrom(sockfd, &recvBuf, sizeof(ntp_packet), 0,
    (struct sockaddr *) &their_addr, &addr_len)) == -1) {
    perror("listener recv from");
    exit(1);
}

ntp_timestamp destTimestamp = getCurrentTimestamp();

network_to_host(&packet);
network_to_host(&recvBuf);

printf("\nReceived:\n\n");

double offset = (((ntp_to_double(&recvBuf.recvTimestamp)) - ntp_to_double(&recvBuf.orgTimestamp)) -
                ((ntp_to_double(&recvBuf.transmitTimestamp)) - ntp_to_double(&destTimestamp))) / 2;



gettimeofday(&tv, NULL);
print_unix_time(&tv);
printf("%+f ", offset);

printf("s%d \n", recvBuf.stratum );
int mode = recvBuf.flags & 0x07;
printf("Mode: %d\n", mode);

/*printf("Reference:\t");
print_ntp_time(&recvBuf.refTimestamp);
printf("Originate:\t");
print_ntp_time(&recvBuf.orgTimestamp);
printf("Receive:\t");
print_ntp_time(&recvBuf.recvTimestamp);
printf("Transmit:\t");
print_ntp_time(&recvBuf.transmitTimestamp);
printf("Destination:\t");
print_ntp_time(&destTimestamp); */




  return 0;
}
