/*****************************************************************************/
/*                                                                           */
/*                                                🭈🭆🭂███🭞🭜🭘  ███🭞🭜🭘██████    */
/*   icmp_packet.h                             🭈🭆🭂███🭞🭜🭘     🭞🭜🭘   ██████    */
/*                                          🭈🭆🭂███🭞🭜🭘           🭈🭆🭂███🭞🭜🭘    */
/*   By: vlaroque                        🭈🭆🭂███🭞🭜🭘           🭈🭆🭂███🭞🭜🭘       */
/*       vlaroque@student.42.fr          ██████████████████  ██████   🭈🭆🭂    */
/*                                       ██████████████████  ██████🭈🭆🭂███.fr */
/*                                                   ██████   🬚🬢🬞🬋🬏🬚🬢🬦🬞🬋🬃    */
/*                                                   ██████   🬕🬀▐🬂▌🬕🬣▐🬞🬰🬃    */
/*                                                                           */
/*****************************************************************************/

#ifndef __ICMP_PACKET_H
#define __ICMP_PACKET_H

#include <netinet/ip_icmp.h>
#include <time.h>		/*timespec*/
#include <netinet/ip.h> /*timespec*/

#include <stdbool.h>

#include "ft_ping.h"

typedef struct icmp_packet_s
{
	struct icmphdr icmp_header;
	/*payload*/
	struct timespec time_stamp;
	uint8_t data[IP_MAXPACKET];
} icmp_packet_t;

icmp_packet_t icmp_packet_init(uint16_t identity, uint16_t seq);
void icmp_packet_update(icmp_packet_t *packet, uint16_t sequence, size_t size, bool ping_support_timing);
bool open_icmp_socket(ping_env_t *env, int *fd);

#endif /* __ICMP_PACKET_H */
