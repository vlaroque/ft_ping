#include <stdio.h>
#include <arpa/inet.h> /* inet_ntoa */

#include "error_reply.h"
#include "icmp_packet.h"

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

void echo_error_reply_handle(icmp_packet_t *resp_packet, struct sockaddr_in *sender_addr, size_t icmp_packet_size)
{
	printf("%zu bytes from %s: ", icmp_packet_size, inet_ntoa(sender_addr->sin_addr));

	switch (resp_packet->icmp_header.type)
	{

	case ICMP_ECHOREPLY:
		printf("Echo Reply\n");
		break;

	case ICMP_DEST_UNREACH:
		unreach_icmp_code_print(resp_packet->icmp_header.code);
		break;

	case ICMP_SOURCE_QUENCH:
		printf("Source Quench\n");
		break;

	case ICMP_REDIRECT:
		redirect_icmp_code_print(resp_packet->icmp_header.code);
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
		break;

	case ICMP_PARAMETERPROB:
		printf("Parameter problem: pointer = %u\n", 
		       (ntohl(resp_packet->icmp_header.un.gateway) >> 24) & 0xFF);
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
