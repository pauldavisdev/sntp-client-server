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
#include "client_functions.h"

#define PORT 123         /* server port the client connects to */
//#define PORT 63333

int main(int argc, char const *argv[]) {
  int sockfd, numbytes;
  struct hostent *he;            /* server data struct */
  struct sockaddr_in their_addr; /* server addr info */

  /* NTP server address */
  char host[] = "0.uk.pool.ntp.org";
  //char host[] = "ntp.uwe.ac.uk";
  //char host[] = "localhost";

  /* resolve server host name or IP address */
  if((he = gethostbyname(host)) == NULL) {
    perror("client gethostbyname");
    exit(1);
  }

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("client socket");
    exit(1);
  }

  /* Initialise and zero NTP packet structs for to send and receive */
  ntp_packet packet;
  struct timeval tv;
  memset(&packet, 0, sizeof(packet));

  set_client_flags(&packet);

  printf("Packet Sent:\n");

  /* get current unix time and print */
  gettimeofday(&tv, NULL);
  print_unix_time(&tv);

  /* transmit timestamp set to current time in NTP timestamp format */
  packet.transmitTimestamp = getCurrentTimestamp(&tv);

  print_packet(&packet);

  /* host to network byte order */
  host_to_network(&packet);

  /* zero server address struct */
  memset(&their_addr, 0, sizeof(their_addr));
  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(PORT);
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);

  /* send packet */
  if((numbytes = sendto(sockfd, &packet, sizeof(ntp_packet), 0,
      (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
    perror("client sendto");
    exit(1);
  }

  socklen_t addr_len = (socklen_t)sizeof(struct sockaddr);

  if((numbytes = recvfrom(sockfd, &packet, sizeof(ntp_packet), 0,
    (struct sockaddr *) &their_addr, &addr_len)) == -1) {
    perror("listener recv from");
    exit(1);
}

/* destination timestamp created on packet arrival for use in offset and delay */
ntp_timestamp destTimestamp = getCurrentTimestamp(&tv);

/* network to host byte order */
network_to_host(&packet);

printf("\nPacket Received:\n");

/* calculate offset and delay using received packet and destination timestamp */
double offset = calculate_offset(&packet, &destTimestamp);
double delay = calculate_delay(&packet, &destTimestamp);

print_unix_time(&tv);

/* print offset and delay */
printf("%+f +/- %f s%d ", offset, delay, packet.stratum);

/* server name to address and print */
char str[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &(their_addr.sin_addr), str, INET_ADDRSTRLEN);
printf("%s %s\n", host, str);

print_packet(&packet);

  return 0;
}
