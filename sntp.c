#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
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
    printf("%s.%06d\n", tmbuf, (int)tv->tv_usec);
}

void convert_ntp_to_unix(ntp_timestamp *ntp, struct timeval *unix_time)
{
    unix_time->tv_sec = ntp->second - UNIX_EPOCH;
    unix_time->tv_usec = (uint32_t)( (double)ntp->fraction * 1.0e6 / (double)(1LL<<32) );
}

void convert_unix_to_ntp(struct timeval *unix_time, ntp_timestamp *ntp)
{
    ntp->second = unix_time->tv_sec + UNIX_EPOCH;
    ntp->fraction = (uint32_t)( (double)(unix_time->tv_usec+1) * (double)(1LL<<32) * 1.0e-6 );
}

void host_to_network(ntp_packet *p)
{
  p->rootDelay = htonl(p->rootDelay);
  p->rootDispersion = htonl(p->rootDispersion);
  p->refIdentifier = ntohl(p->refIdentifier);
  p->refTimestamp.second = htonl(p->refTimestamp.second);
  p->orgTimestamp.second = htonl(p->orgTimestamp.second);
  p->recvTimestamp.second = htonl(p->recvTimestamp.second);
  p->transmitTimestamp.second = htonl(p->transmitTimestamp.second);
}

void network_to_host(ntp_packet *p)
{
  p->rootDelay = ntohl(p->rootDelay);
  p->rootDispersion = ntohl(p->rootDispersion);
  p->refIdentifier = ntohl(p->refIdentifier);
  p->transmitTimestamp.second = ntohl(p->transmitTimestamp.second);
  p->refTimestamp.second = ntohl(p->refTimestamp.second);
  p->recvTimestamp.second = ntohl(p->recvTimestamp.second);
  p->orgTimestamp.second = ntohl(p->orgTimestamp.second);
}
