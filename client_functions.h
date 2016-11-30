void set_client_flags(ntp_packet *p);

double ntp_to_double(ntp_timestamp *p);

double calculate_offset(ntp_packet *p, ntp_timestamp *t);

double calculate_delay(ntp_packet *p, ntp_timestamp *t);
