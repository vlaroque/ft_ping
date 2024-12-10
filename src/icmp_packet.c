#include "icmp_packet.h"
#include <string.h> /* memset */
#include <unistd.h>
#include <arpa/inet.h> /* htons */

static uint16_t calculate_checksum(uint8_t *buffer, int length) {
    uint32_t result = 0;
    uint16_t *data = (uint16_t *)buffer;

    /* Add all 16 bytes words together */
    while (length > 1) {
        result += *data;
        data++;
        length -= 2;
    }

    /* Add the last remaining byte if it remains */
    if (length == 1) {
        result += *(uint8_t *)data;
    }

    /* Put 32 bit result into 16 bits, loop needed if carries remain */
    while (result >> 16) {
        result = (result & 0xFFFF) + (result >> 16);
    }

    /* Return one's complement */
    return ~result;
}

icmp_packet_t init_echo_icmp_packet(pid_t pid, uint16_t seq)
{
    icmp_packet_t res;
    memset(&res, 0, sizeof(icmp_packet_t));

    res.icmp_header.type             = ICMP_ECHO;
    res.icmp_header.code             = 0;
    res.icmp_header.checksum         = 0;
    res.icmp_header.un.echo.id       = htons(pid & 0xFFFF);
    res.icmp_header.un.echo.sequence = htons(seq);
    gettimeofday(&res.time_stamp, 0);
    res.icmp_header.checksum         = calculate_checksum((uint8_t *)&res, sizeof(res));

    return res;
}
