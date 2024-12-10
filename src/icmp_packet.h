#ifndef __ICMP_PACKET_H
#define __ICMP_PACKET_H

#include <netinet/ip_icmp.h>
#include <sys/time.h>

#define ICMP_PAYLOAD_SIZE 32
#define TIMEVAL_SIZE 2 * 8 /* 2 x long size */
#define DATA_SIZE ICMP_PAYLOAD_SIZE - TIMEVAL_SIZE

typedef struct icmp_packet_s
{
    struct icmphdr icmp_header;
    /*payload*/
    struct timeval time_stamp;
    uint8_t data[DATA_SIZE];
} icmp_packet_t;

icmp_packet_t init_echo_icmp_packet(pid_t pid, uint16_t seq);

#endif  /* __ICMP_PACKET_H */
