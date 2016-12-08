/* server functions */

#include "sntp.h"

void set_server_reply(ntp_packet *p)
{
  network_to_host(p);

  p->recvTimestamp = getCurrentTimestamp();
  p->orgTimestamp = p->transmitTimestamp;

  /* set up flag bitfield of struct, so li is 0, vn is 4, mode is 4 */
  // li
  p->flags = 0;
  p->flags <<= 3;
  // vn
  p->flags |= 4;
  p->flags <<= 3;
  // mode
  p->flags |= 4;
  // set stratum to 2
  p->stratum = 2;

  p->transmitTimestamp = getCurrentTimestamp();

  host_to_network(p);
}
