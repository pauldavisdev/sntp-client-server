/*  client.c - SNTP client
 *
 */

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
//#define PORT 63333

int main(int argc, char const *argv[]) {
  int sockfd, numbytes;
  struct hostent *he;            /* server data struct */
  struct sockaddr_in their_addr; /* server addr info */

  /* NTP server address */
  //char host[] = "0.uk.pool.ntp.org";
  char host[] = "ntp.uwe.ac.uk";
  //char host[] = "localhost";

  /* Initialise and zero NTP packet structs for to send and receive */
  ntp_packet packet;
  memset(&packet, 0, sizeof(packet));

  set_client_flags(&packet);

  /* resolve server host name or IP address */
  if((he = gethostbyname(host)) == NULL) {
    perror("client gethostbyname");
    exit(1);
  }

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("client socket");
    exit(1);
  }

  printf("Sending..\n");

  /* get current unix time*/
  gettimeofday(&tv, NULL);

  /* print unix time before sending */
  print_unix_time(&tv);

  /* zero server address struct */
  memset(&their_addr, 0, sizeof(their_addr));
  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(PORT);
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);

  /* transmit timestamp set to current time in NTP timestamp format */
  packet.transmitTimestamp = getCurrentTimestamp();

  print_packet(&packet);

  /* host to network byte order */
  host_to_network(&packet);

  /* send packet */
  if((numbytes = sendto(sockfd, &packet, sizeof(ntp_packet), 0,
      (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
    perror("client sendto");
    exit(1);
  }

  printf("\n\nReceiving..\n");

  socklen_t addr_len = (socklen_t)sizeof(struct sockaddr);

  if((numbytes = recvfrom(sockfd, &packet, sizeof(ntp_packet), 0,
    (struct sockaddr *) &their_addr, &addr_len)) == -1) {
    perror("listener recv from");
    exit(1);
}

/* destination timestamp created on packet arrival for use in offset and delay */
ntp_timestamp destTimestamp = getCurrentTimestamp();

/* network to host byte order */
network_to_host(&packet);

printf("\nReceived:\n");

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
