#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <math.h>
#include "sntp.h"

/*******************************************************************************
*   Function:     getCurrentTimestamp
*   Description:  Returns current time of day in NTP timestamp format
*
*   Parameters:   None
*
*   Returns:      ntp_timestamp
*******************************************************************************/
ntp_timestamp getCurrentTimestamp()
{
  ntp_timestamp current;
  gettimeofday(&tv, NULL);
  convert_unix_to_ntp(&tv, &current);

  return current;
}

/*  Function:     print_unix_time
*
*   Description:  Gets unix formatted time into buffer then prints. Includes
*                 microseconds to 6 decimal places and local time offset.
*
*   Parameters:   struct timeval*
*
*   Returns:      None
*/
void print_unix_time(struct timeval *tv) {
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64];
    nowtime = tv->tv_sec;
    nowtm = localtime(&nowtime);

    strftime(tmbuf, sizeof (tmbuf), "%Y-%m-%d %H:%M:%S", nowtm);
    printf("%s.%06d", tmbuf, (int)tv->tv_usec);
    strftime(tmbuf, sizeof (tmbuf), "%z", nowtm);
    printf(" (%s) ", tmbuf);
}

/*  Function:     print_ntp_time
*
*   Description:  Prints NTP timestamp in readable format of 'seconds.fractions'
*
*   Parameters:   ntp_timestamp*
*
*   Returns:      None
*/
void print_ntp_time(ntp_timestamp *ntp)
{
  printf("%ld.%ld\n", (long int)ntp->second, (long int)ntp->fraction);
}

/*  Function:     print_ntp_packet
*
*   Description:  Prints an NTP packet's timestamp values in readable format.
*
*   Parameters:   ntp_packet*
*
*   Returns:      None
*/
void print_ntp_packet(ntp_packet *p)
{
  printf("Ref timestamp: \t\t");
  print_ntp_time(&p->refTimestamp);
  printf("Org timestamp: \t\t");
  print_ntp_time(&p->orgTimestamp);
  printf("Recv timestamp: \t");
  print_ntp_time(&p->recvTimestamp);
  printf("Transmit timestamp: \t");
  print_ntp_time(&p->transmitTimestamp);
}

/*  Function:     convert_ntp_to_unix
*
*   Description:  Converts NTP timestamp time into unix time.
*
*   Parameters:   ntp_timestamp*, struct timeval*
*
*   Returns:      None
*/
void convert_ntp_to_unix(ntp_timestamp *ntp, struct timeval *unix_time)
{
    unix_time->tv_sec = ntp->second - UNIX_EPOCH;
    unix_time->tv_usec = (uint32_t)((double)ntp->fraction * 1.0e6 / (double)(1LL<<32) );
}

/*  Function:     convert_unix_to_ntp
*
*   Description:  Converts unix time into NTP timestamp time.
*
*   Parameters:   struct timeval*, ntp_timestamp*
*
*   Returns:      None
*/
void convert_unix_to_ntp(struct timeval *unix_time, ntp_timestamp *ntp)
{
    ntp->second = unix_time->tv_sec + UNIX_EPOCH;
    ntp->fraction = (uint32_t)((double)(unix_time->tv_usec+1) * (double)(1LL<<32) * 1.0e-6 );
}

/*  Function:     host_to_network
*
*   Description:  Converts NTP packet to network byte order.
*
*   Parameters:   ntp_packet*
*
*   Returns:      None
*/
void host_to_network(ntp_packet *p)
{
  p->rootDelay = htonl(p->rootDelay);
  p->rootDispersion = htonl(p->rootDispersion);
  p->refIdentifier = htonl(p->refIdentifier);
  p->refTimestamp.second = htonl(p->refTimestamp.second);
  p->refTimestamp.fraction = htonl(p->refTimestamp.fraction);
  p->orgTimestamp.second = htonl(p->orgTimestamp.second);
  p->orgTimestamp.fraction = htonl(p->orgTimestamp.fraction);
  p->recvTimestamp.second = htonl(p->recvTimestamp.second);
  p->recvTimestamp.fraction = htonl(p->recvTimestamp.fraction);
  p->transmitTimestamp.second = htonl(p->transmitTimestamp.second);
  p->transmitTimestamp.fraction = htonl(p->transmitTimestamp.fraction);
}

/*  Function:     network_to_host
*
*   Description:  Converts NTP packet to host byte order.
*
*   Parameters:   ntp_packet*
*
*   Returns:      None
*/
void network_to_host(ntp_packet *p)
{
  p->rootDelay = ntohl(p->rootDelay);
  p->rootDispersion = ntohl(p->rootDispersion);
  p->refIdentifier = ntohl(p->refIdentifier);
  p->transmitTimestamp.second = ntohl(p->transmitTimestamp.second);
  p->transmitTimestamp.fraction = ntohl(p->transmitTimestamp.fraction);
  p->refTimestamp.second = ntohl(p->refTimestamp.second);
  p->refTimestamp.fraction = ntohl(p->refTimestamp.fraction);
  p->recvTimestamp.second = ntohl(p->recvTimestamp.second);
  p->recvTimestamp.fraction = ntohl(p->recvTimestamp.fraction);
  p->orgTimestamp.second = ntohl(p->orgTimestamp.second);
  p->orgTimestamp.fraction = ntohl(p->orgTimestamp.fraction);
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
