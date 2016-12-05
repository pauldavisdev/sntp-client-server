#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <math.h>
#include "sntp.h"

ntp_timestamp getCurrentTimestamp()
{
  ntp_timestamp current;
  gettimeofday(&tv, NULL);
  convert_unix_to_ntp(&tv, &current);

  return current;
}

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

void print_ntp_time(ntp_timestamp *ntp)
{
  printf("%ld.%ld\n", (long int)ntp->second, (long int)ntp->fraction);
}

void convert_ntp_to_unix(ntp_timestamp *ntp, struct timeval *unix_time)
{
    unix_time->tv_sec = ntp->second - UNIX_EPOCH;
    unix_time->tv_usec = (uint32_t)((double)ntp->fraction * 1.0e6 / (double)(1LL<<32) );
}

void convert_unix_to_ntp(struct timeval *unix_time, ntp_timestamp *ntp)
{
    ntp->second = unix_time->tv_sec + UNIX_EPOCH;
    ntp->fraction = (uint32_t)((double)(unix_time->tv_usec+1) * (double)(1LL<<32) * 1.0e-6 );
}

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

double ntp_to_double(ntp_timestamp *p)
{
  double t1_s = (p->second);
  double t1_f = (p->fraction) / (double)(1LL<<32);
  t1_s += t1_f;
  return t1_s;
}

double calculate_offset(ntp_packet *p, ntp_timestamp *t)
{
  double offset = (((ntp_to_double(&p->recvTimestamp)) - ntp_to_double(&p->orgTimestamp)) +
                  ((ntp_to_double(&p->transmitTimestamp)) - ntp_to_double(t))) / 2;

  return offset;
}

double calculate_delay(ntp_packet *p, ntp_timestamp *t)
{
  double delay = (((ntp_to_double(t) - ntp_to_double(&p->orgTimestamp)) ) -
                 ((ntp_to_double(&p->transmitTimestamp) - ntp_to_double(&p->recvTimestamp))));

  return delay;
}
