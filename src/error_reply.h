#ifndef ERROR_REPLY_H
#define ERROR_REPLY_H

#include "ft_ping.h"
#include "icmp_packet.h"

void echo_error_reply_handle(icmp_packet_t *resp_packet, struct sockaddr_in *sender_addr, size_t icmp_packet_size);


#endif /* ERROR_REPLY_H */
