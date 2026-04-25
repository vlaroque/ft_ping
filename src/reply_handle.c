/*****************************************************************************/
/*                                                                           */
/*                                                🭈🭆🭂███🭞🭜🭘  ███🭞🭜🭘██████    */
/*   reply_handle.c                            🭈🭆🭂███🭞🭜🭘     🭞🭜🭘   ██████    */
/*                                          🭈🭆🭂███🭞🭜🭘           🭈🭆🭂███🭞🭜🭘    */
/*   By: vlaroque                        🭈🭆🭂███🭞🭜🭘           🭈🭆🭂███🭞🭜🭘       */
/*       vlaroque@student.42.fr          ██████████████████  ██████   🭈🭆🭂    */
/*                                       ██████████████████  ██████🭈🭆🭂███.fr */
/*                                                   ██████   🬚🬢🬞🬋🬏🬚🬢🬦🬞🬋🬃    */
/*                                                   ██████   🬕🬀▐🬂▌🬕🬣▐🬞🬰🬃    */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <arpa/inet.h> /* inet_ntoa */

#include "reply_handle.h"
#include "duplicate.h"
#include "printing.h"

static void echo_reply_take_stats(ping_env_t *env, double rtt)
{
	env->total_time += rtt;
	env->square_time += ( rtt * rtt );
	if ( rtt < env->min_time )
		env->min_time = rtt;
	if ( rtt > env->max_time )
		env->max_time = rtt;
}

static void echo_reply_print(size_t len, struct in_addr *sender, uint16_t seq,
                      uint8_t ttl, bool time, double rtt, bool duplicate)
{
	printf("%ld bytes from %s: icmp_seq=%u ttl=%d", len, inet_ntoa(*sender), seq, ttl);

	if (time)
		printf(" time=%.3f ms", rtt);
	if (duplicate)
		printf(" (DUP!)");

	printf("\n");
}

bool is_icmp_packet_echo_reply(ping_env_t *env, struct icmphdr *icmp_header)
{
	struct iphdr *ping_ip_header = (struct iphdr *)(icmp_header + 1);

	struct icmphdr *ping_icmp_header = (struct icmphdr *)((unsigned char *)ping_ip_header + (ping_ip_header->ihl << 2));

	if (ping_ip_header->daddr == env->target_sock_addr.sin_addr.s_addr
	        && ping_ip_header->protocol == IPPROTO_ICMP
	        && ping_icmp_header->type == ICMP_ECHO
	        && (ntohs(ping_icmp_header->un.echo.id) == env->identity))
	{
		return true;
	}

	return false;
}

bool echo_reply_handle(ping_env_t *env, icmp_packet_t *resp_packet, struct sockaddr_in* sender_addr,
                       double rtt, size_t icmp_packet_size, uint8_t ttl)
{
	bool     is_duplicate = false;
	bool     contain_time = false;

	uint16_t identity = ntohs(resp_packet->icmp_header.un.echo.id);
	
	if ( env->using_raw_socket && identity != env->identity)
		return false;

	uint16_t sequence = ntohs(resp_packet->icmp_header.un.echo.sequence);

	if ( !seq_bit_is_duplicate(env, sequence) )
	{
		seq_bit_ack_received(env, sequence);
		env->received_pings += 1;
	}
	else
	{
		env->duplicated_pings += 1;
		is_duplicate = true;
	}
	
	if ( env->ping_support_timing && icmp_packet_size >= ICMP_MIN_SIZE_FOR_TIMING)
	{
		echo_reply_take_stats(env, rtt);
		contain_time = true;
	}

	echo_reply_print(icmp_packet_size, &sender_addr->sin_addr, sequence, ttl, contain_time, rtt, is_duplicate);

	DEBUG("%zu bytes from %s received type = %d code = %d id = %d icmp_seq = %d checksum=%d time=%f\n",
	  icmp_packet_size,
	  inet_ntoa(sender_addr->sin_addr),
	  resp_packet->icmp_header.type,
	  resp_packet->icmp_header.code,
	  ntohs(resp_packet->icmp_header.un.echo.id),
	  ntohs(resp_packet->icmp_header.un.echo.sequence),
	  resp_packet->icmp_header.checksum,
	  rtt);

	return true;
}
