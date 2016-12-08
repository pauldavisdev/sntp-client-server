/* server functions */

#include "sntp.h"

/*  Function:     set_server_reply
*   Description:  Converts the client request packet from network to host byte
*   order. Then sets all fields that are needed for a reply to unicast client.
*   Finally converts the server reply packet from host to network byte order.
*
*   Parameters:   ntp_packet*
*
*   Returns:      none
*/
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
