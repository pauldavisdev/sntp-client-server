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
  // set the first 8 bits of struct, so li is 0, vn is 4, mode is 3
  *((char *)&packet) = 0b00100011;

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

  printf("Packet left client at: \n");
  gettimeofday(&tv, NULL);
  print_unix_time(tv);
  convert_unix_to_ntp(&tv, &ntp_t);
  printf("NTP time: %ld.%ld \n", (long int)ntp_t.second, (long int)ntp_t.fraction);

  memset(&their_addr, 0, sizeof(their_addr));
  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(PORT);
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);

  packet.transmitTimestamp = getCurrentTimestamp();
  packet.transmitTimestamp.second = htonl(packet.transmitTimestamp.second);
  packet.transmitTimestamp.fraction = htonl(packet.transmitTimestamp.fraction);

  if((numbytes = sendto(sockfd, &packet, sizeof(ntp_packet), 0,
      (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
    perror("client sendto");
    exit(1);
  }

  socklen_t addr_len = (socklen_t)sizeof(struct sockaddr);
  numbytes = 0;

  if((numbytes = recvfrom(sockfd, &packet, sizeof(ntp_packet), 0,
    (struct sockaddr *) &their_addr, &addr_len)) == -1) {
    perror("listener recv from");
    exit(1);
}

packet.transmitTimestamp.second = ntohl(packet.transmitTimestamp.second);
packet.transmitTimestamp.fraction = ntohl(packet.transmitTimestamp.fraction);

printf("\nPacket left server at: \n");
convert_ntp_to_unix(&packet.transmitTimestamp, &tv);
print_unix_time(tv);
convert_unix_to_ntp(&tv, &ntp_t);
printf("NTP time: %ld.%ld \n", (long int)ntp_t.second, (long int)ntp_t.fraction);

printf("Stratum: %d \n", packet.stratum );

close(sockfd);

  return 0;
}
