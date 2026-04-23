#ifndef REPLY_HANDLE_H
#define REPLY_HANDLE_H

#include "ft_ping.h"
#include "icmp_packet.h"

#define ICMP_MIN_SIZE_FOR_TIMING (sizeof(struct icmphdr) + sizeof(struct timespec))

bool echo_reply_handle(ping_env_t *env, icmp_packet_t *resp_packet, struct sockaddr_in* sender_addr,
                       double rtt, size_t icmp_packet_size, uint8_t ttl);

bool is_icmp_packet_echo_reply(ping_env_t *env, struct icmphdr *icmp_header);

#endif /* REPLY_HANDLE_H */
