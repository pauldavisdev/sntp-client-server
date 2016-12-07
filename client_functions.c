/* Client functions*/

#include "sntp.h"

/*  Function: set_client_request
*   Description:
*
*   Parameters:
*/
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
  print_ntp_packet(p);
  /* host to network byte order */
  host_to_network(p);
}

/*  Function: print_sntp_output
*   Description:
*
*   Parameters:
*/
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

/*  Function: check_reply
*   Description:
*
*   Parameters:
*/
void check_reply(ntp_packet *p, ntp_packet *r)
{
  network_to_host(p);

  if((p->transmitTimestamp.second != r->orgTimestamp.second)
  || (p->transmitTimestamp.fraction != r->orgTimestamp.fraction)) {
    perror("Invalid server reply: Originate timestamp");
    exit(1);
  }

  if((r->flags & 0x07) != 4) {
    perror("Invalid server reply: Mode");
    exit(1);
  }

  if((r->stratum <= 0) || (r->stratum > 15)) {
    perror("Invalid server reply: Stratum is too high or set to zero");
    exit(1);
  }

  if((r->transmitTimestamp.second && r->transmitTimestamp.fraction) == 0) {
    perror("Invalid server reply: Transmit timestamp is 0");
    exit(1);
  }
}

/*  Function:     ntp_to_double
*
*   Description:  Converts the seconds and fractions in an NTP timestamp into
*                 a double.
*
*   Parameters:   ntp_timestamp*
*
*   Returns:      double t1_s
*/
double ntp_to_double(ntp_timestamp *p)
{
  double t1_s = (p->second);
  double t1_f = (p->fraction) / (double)(1LL<<32);
  t1_s += t1_f;
  return t1_s;
}

/*  Function:     calculate_offset
*
*   Description:  Calcluates time offset
*
*   Parameters:   ntp_packet*, ntp_timestamp*
*
*   Returns:      double offset
*/
double calculate_offset(ntp_packet *p, ntp_timestamp *t)
{
  double offset = (((ntp_to_double(&p->recvTimestamp)) - ntp_to_double(&p->orgTimestamp)) +
                  ((ntp_to_double(&p->transmitTimestamp)) - ntp_to_double(t))) / 2;

  return offset;
}

/*  Function:     calculate_delay
*
*   Description:  Calculates time delay
*
*   Parameters:   ntp_packet*
*                 ntp_timestamp*
*
*   Returns:      double delay
*/
double calculate_delay(ntp_packet *p, ntp_timestamp *t)
{
  double delay = (((ntp_to_double(t) - ntp_to_double(&p->orgTimestamp)) ) -
                 ((ntp_to_double(&p->transmitTimestamp) - ntp_to_double(&p->recvTimestamp))));

  return delay;
}
