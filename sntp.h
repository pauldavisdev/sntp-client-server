#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>

#define UNIX_EPOCH 0x83AA7E80;  // the seconds from Jan 1, 1900 to Jan 1, 1970

typedef struct {
  uint32_t second;
  uint32_t fraction;
} ntp_timestamp;

typedef struct {
  uint8_t flags;
  /* Eight-bit flags - leap indicator, version number and mode
   0 1 2 3 4 5 6 7
  +-+-+-+-+-+-+-+-+
  |LI | VN  |Mode |
  +-+-+-+-+-+-+-+-+
  */
  uint8_t stratum;            /* Eight-bit stratum indicator */
  uint8_t pollInterval;       /* Eight-bit max poll interval */
  uint8_t precision;           /* Eight-bit precision (in seconds) */

  /* Fixed-point indicating total roundtrip delay to primary reference source */
  uint32_t rootDelay;

  /* Fixed-point indicating max error due to clock freq tolerance in seconds*/
  uint32_t rootDispersion;

  /* 32-bit bitstring identifying the ref source */
  uint32_t refIdentifier;

  /* time system clock was last set or corrected, in 64-bit timestamp format*/
  ntp_timestamp refTimestamp;

  /* time request departed the client for server, in 64-bit timestamp format*/
  ntp_timestamp orgTimestamp;

  /* time the request arrived at server or the reply arrived at client */
  ntp_timestamp recvTimestamp;

  /* time the request departed the client or the reply departed the server*/
  ntp_timestamp transmitTimestamp;

} ntp_packet;

struct timeval tv;

ntp_timestamp ntp_temp;

ntp_timestamp getCurrentTimestamp();

void print_unix_time(struct timeval *tv);

void convert_ntp_to_unix(ntp_timestamp *ntp, struct timeval *unix_time);

void convert_unix_to_ntp(struct timeval *unix_time, ntp_timestamp *ntp);

void host_to_network(ntp_packet *p);
