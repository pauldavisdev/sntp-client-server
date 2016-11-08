typedef struct {
  uint32_t seconds;
  uint32_t fraction;
} ntp_timestamp;

typedef struct {
  unsigned li :   2;        /* Two-bit Leap indicator */
  unsigned vn :   3;        /* Three-bit version number indicator */
  unsigned mode : 3;        /* Three-bit protocol number */
  uint8_t stratum;          /* Eight-bit stratum indicator */
  uint8_t pollInterval;     /* Eight-bit max poll interval */
  int8_t precision;         /* Eight-bit precision (in seconds) */

  int32_t rootDelay;        /* 32-bit fixed-point indicating total roundtrip..
                            * ..delay to primary reference source */

  uint32_t rootDispersion;  /* 32-bit fixed-point indicating max error..
                            * ..due to clock freq tolerance in seconds*/

  uint32_t refIdentifier;   /* 32-bit bitstring identifying the ref source */

  refTimestamp  ntp_timestamp;  /* time the system clock was last set or..
                                ..corrected, in a 64-bit timestamp format*/

  orgTimestamp  ntp_timestamp;  /* time the request departed the client for..
                                ..server, in 64-bit timestamp format*/

  recvTimestamp ntp_timestamp;  /* time the request arrived at server or..
                                ..the reply arrived at client */

  tranTimestamp ntp_timestamp;  /* time the request departed the client..
                                ..or the reply departed the server*/
} ntp_packet;
