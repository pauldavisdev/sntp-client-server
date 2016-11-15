#define UNIX_EPOCH 0x83AA7E80;

typedef struct {
  uint32_t seconds;
  uint32_t fraction;
} ntp_timestamp;

typedef struct {
  unsigned li     : 2;        /* Two-bit Leap indicator */
  unsigned vn     : 3;        /* Three-bit version number indicator */
  unsigned mode   : 3;        /* Three-bit protocol number */
  uint8_t stratum;            /* Eight-bit stratum indicator */
  uint8_t pollInterval;       /* Eight-bit max poll interval */
  int8_t precision;           /* Eight-bit precision (in seconds) */

  /* Fixed-point indicating total roundtrip delay to primary reference source */
  int32_t rootDelay;

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


void convert_ntp_to_unix(struct ntp_time_t *ntp, struct timeval *unix_time)
{
    unix_time->tv_sec = ntp->second - UNIX_EPOCH; // the seconds from Jan 1, 1900 to Jan 1, 1970
    unix_time->tv_usec = (uint32_t)( (double)ntp->fraction * 1.0e6 / (double)(1LL<<32) );
}

void convert_unix_to_ntp(struct timeval *unix_time, struct ntp_time_t *ntp)
{
    ntp->second = unix_time->tv_sec + 0x83AA7E80;
    ntp->fraction = (uint32_t)( (double)(unix_time->tv_usec+1) * (double)(1LL<<32) * 1.0e-6 );
}
