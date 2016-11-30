#include "sntp.h"

/* unicast-client functions */

/* set up flags for standard unicast-client set up */
void set_client_flags(ntp_packet *p)
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
}
