/* Client functions*/

void set_client_request(ntp_packet *p);

void print_sntp_output(ntp_packet *p, double offset, double delay,
                       struct sockaddr_in their_addr, char *host);

void check_reply(ntp_packet *p, ntp_packet *r);

double ntp_to_double(ntp_timestamp *p);

double calculate_offset(ntp_packet *p, ntp_timestamp *t);

double calculate_delay(ntp_packet *p, ntp_timestamp *t);
