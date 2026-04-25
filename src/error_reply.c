/*****************************************************************************/
/*                                                                           */
/*                                                🭈🭆🭂███🭞🭜🭘  ███🭞🭜🭘██████    */
/*   echo_reply.c                              🭈🭆🭂███🭞🭜🭘     🭞🭜🭘   ██████    */
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

#include "error_reply.h"
#include "icmp_packet.h"

void dump_ip_data(struct iphdr *ip_header)
{
	printf("IP Hdr Dump:\n");

	for( size_t index = 0; index < sizeof(struct iphdr); index++ )
	{
		printf("%02x", *((uint8_t *)ip_header + index) );
		if (index % 2)
			printf(" ");
	}
	printf("\n");
}

void ip_header_print(ping_env_t *env, struct iphdr *ip_header)
{
	if ( env->verbose )
		dump_ip_data(ip_header);

	printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
	printf(" %1x", ip_header->version);
	printf("  %1x", ip_header->ihl);
	printf("  %02x", ip_header->tos);
	printf(" %04x", ntohs(ip_header->tot_len));
	printf(" %04x", ntohs(ip_header->id));
	printf("   %1x", (ntohs(ip_header->frag_off) & 0xe000) >> 13);
	printf(" %04x", ntohs(ip_header->frag_off) & 0x1fff);
	printf("  %02x", ip_header->ttl);
	printf("  %02x", ip_header->protocol);
	printf(" %04x", ntohs(ip_header->check));
	struct in_addr src = { .s_addr = ip_header->saddr };
	struct in_addr dst = { .s_addr = ip_header->daddr };
	printf(" %s ", inet_ntoa(src));
	printf(" %s ", inet_ntoa(dst) );

	/* here we print the options of ip packet's header */
	size_t   ip_hlen   = ip_header->ihl << 2;
	uint8_t *cp = (uint8_t *)ip_header + sizeof(*ip_header);

	while (ip_hlen > sizeof(*ip_header))
	{
		printf("%02x", *cp);
		cp++;
		ip_hlen--;
	}
	printf("\n");
}

void ip_data_print(ping_env_t *env, struct icmphdr *icmp_header)
{
	/* header of the packet that was replied to the ping */
	struct iphdr *ip_header = (struct iphdr *)(icmp_header + 1);
	int           ip_hlen   = ip_header->ihl << 2;
	int           icmp_size = ntohs(ip_header->tot_len) - ip_hlen;

	ip_header_print(env, ip_header);

	struct icmphdr *orig_icmp = (struct icmphdr *)((unsigned char *)ip_header+ (ip_header->ihl << 2));
	printf("ICMP: type %u, code %u, size %d", orig_icmp->type, orig_icmp->code, icmp_size);

	if (orig_icmp->type == ICMP_ECHOREPLY || orig_icmp->type == ICMP_ECHO)
		printf(", id 0x%04x, seq 0x%04x", ntohs(orig_icmp->un.echo.id), ntohs(orig_icmp->un.echo.sequence));

	printf("\n");
}

void unreach_icmp_code_print(int code)
{
	switch (code)
	{
		case ICMP_NET_UNREACH:
			printf("Destination Net Unreachable");
			break;
		case ICMP_HOST_UNREACH:
			printf("Destination Host Unreachable");
			break;
		case ICMP_PROT_UNREACH:
			printf("Destination Protocol Unreachable");
			break;
		case ICMP_PORT_UNREACH:
			printf("Destination Port Unreachable");
			break;
		case ICMP_FRAG_NEEDED:
			printf("Fragmentation needed and DF set");
			break;
		case ICMP_SR_FAILED:
			printf("Source Route Failed");
			break;
		case ICMP_NET_UNKNOWN:
			printf("Network Unknown");
			break;
		case ICMP_HOST_UNKNOWN:
			printf("Host Unknown");
			break;
		case ICMP_HOST_ISOLATED:
			printf("Host Isolated");
			break;
		case ICMP_NET_UNR_TOS:
			printf("Destination Network Unreachable At This TOS");
			break;
		case ICMP_HOST_UNR_TOS:
			printf("Destination Host Unreachable At This TOS");
			break;
		case ICMP_PKT_FILTERED:
			printf("Packet Filtered");
			break;
		case ICMP_PREC_VIOLATION:
			printf("Precedence Violation");
			break;
		case ICMP_PREC_CUTOFF:
			printf("Precedence Cutoff");
			break;
		default:
			printf("Dest Unreachable, Unknown Code: %d", code);
	}
	printf("\n");
}

void redirect_icmp_code_print(int code)
{
	switch (code)
	{
		case ICMP_REDIR_NET:
			printf("Redirect Network");
			break;
		case ICMP_REDIR_HOST:
			printf("Redirect Host");
			break;
		case ICMP_REDIR_NETTOS:
			printf("Redirect Type of Service and Network");
			break;
		case ICMP_REDIR_HOSTTOS:
			printf("Redirect Type of Service and Host");
			break;
		default:
			printf("Redirect, Unknown Code: %d", code);
	}
	printf("\n");
}

void time_exceeded_code_print(int code)
{
	if ( code == ICMP_EXC_TTL )
		printf("Time to live exceeded\n");
	else if ( code == ICMP_EXC_FRAGTIME )
		printf("Frag reassembly time exceeded\n");
	else
		printf("Time exceeded, Unknown Code: %d", code);
}

void echo_error_reply_handle(ping_env_t *env, icmp_packet_t *resp_packet, struct sockaddr_in *sender_addr, size_t icmp_packet_size)
{
	printf("%zu bytes from %s: ", icmp_packet_size, inet_ntoa(sender_addr->sin_addr));

	switch (resp_packet->icmp_header.type)
	{

	case ICMP_ECHOREPLY:
		printf("Echo Reply\n");
		break;

	case ICMP_DEST_UNREACH:
		unreach_icmp_code_print(resp_packet->icmp_header.code);
		if (env->verbose)
			ip_data_print(env, &resp_packet->icmp_header);

		break;

	case ICMP_SOURCE_QUENCH:
		printf("Source Quench\n");
		ip_data_print(env, &resp_packet->icmp_header);
		break;

	case ICMP_REDIRECT:
		redirect_icmp_code_print(resp_packet->icmp_header.code);
		if (env->verbose)
			ip_data_print(env, &resp_packet->icmp_header);

		break;

	case ICMP_ECHO:
		printf("Echo Request\n");
		break;

	case ICMP_ROUTERADVERT:
		printf("Router Advertisement\n");
		break;

	case ICMP_ROUTERSOLICIT:
		printf("Router Discovery\n");
		break;

	case ICMP_TIME_EXCEEDED:
		time_exceeded_code_print(resp_packet->icmp_header.code);
		if (env->verbose)
			ip_data_print(env, &resp_packet->icmp_header);
		break;

	case ICMP_PARAMETERPROB:
		printf("Parameter problem: pointer = %u\n", 
		       (ntohl(resp_packet->icmp_header.un.gateway) >> 24) & 0xFF);
		ip_data_print(env, &resp_packet->icmp_header);
		break;

	case ICMP_TIMESTAMP:
		printf("Timestamp\n");
		break;

	case ICMP_TIMESTAMPREPLY:
		printf("Timestamp Reply\n");
		break;

	case ICMP_INFO_REQUEST:
		printf("Information Request\n");
		break;

	case ICMP_INFO_REPLY:
		printf("Information Reply\n");
		break;

	case ICMP_ADDRESS:
		printf("Address Mask Request\n");
		break;

	case ICMP_ADDRESSREPLY:
		printf("Address Mask Reply\n");
		break;

	default:
		printf("Bad ICMP type: %d\n", resp_packet->icmp_header.type);
	}
}
