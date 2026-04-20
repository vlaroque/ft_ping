#ifndef __ICMP_PACKET_H
#define __ICMP_PACKET_H

#include <netinet/ip_icmp.h>
#include <time.h> /*timespec*/

#include <stdbool.h>

#define ICMP_PAYLOAD_SIZE 32
#define TIMEVAL_SIZE 2 * 8 /* 2 x long size */
#define DATA_SIZE ICMP_PAYLOAD_SIZE - TIMEVAL_SIZE

typedef struct icmp_packet_s
{
    struct icmphdr icmp_header;
    /*payload*/
    struct timespec time_stamp;
    uint8_t data[DATA_SIZE];
} icmp_packet_t;

icmp_packet_t icmp_packet_init(uint16_t identity, uint16_t seq);
void icmp_packet_update(icmp_packet_t *packet, uint16_t sequence);
bool open_icmp_socket(int *fd);

#endif  /* __ICMP_PACKET_H */
