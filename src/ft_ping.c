#include <sys/socket.h>         /* socket(), sendto(), SOCK_RAW, AF_INET*/
#include <unistd.h>             /* getuid() */
#include <stdlib.h>             /* EXIT_FAILURE EXIT_SUCCES */
#include <stdio.h>              /* printf() */
#include <netinet/ip_icmp.h>    /* IPPROTO_ICMP */
#include <stdbool.h>            /* bool */
#include <arpa/inet.h>          /*inet_ntoa*/
#include <netinet/ip.h>


#include "init.h"
#include "icmp_packet.h"
#include "dest.h"

#define MTU 1500

int main(int ac, char **av)
{
	int fd = 0;

	if ( !parse_args(ac, av) )
		return -1;

	if ( !init() )
		return -1;

	if ( getuid() == 0 )
		fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	else
		fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

	if ( fd < 2 )
		return 1;
	else
		printf("fd opened %d\n", fd);

	icmp_packet_t packet = init_echo_icmp_packet(get_process_id(), 1);

	//struct sockaddr_in addr = get_dest();

	ssize_t len = sendto(fd, &packet, sizeof(icmp_packet_t), 0, (struct sockaddr *)get_target_sock_addr(), sizeof(struct sockaddr_in) );

	if ( len == 0 )
	{
		fprintf(stderr, "Failed to send packet\n");
		return EXIT_FAILURE;
	}
	else
	{
		printf("send %ld bytes\n", len);
	}

	uint8_t recv_buff[MTU];
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int recv_ret = recvfrom(fd, (void *)recv_buff, MTU, 0, (struct sockaddr *)get_target_sock_addr(), &addr_len);

	if (recv_ret == -1)
		printf("Failed to receive\n");
	else
	{
		icmp_packet_t *resp_packet = (icmp_packet_t *) (recv_buff + sizeof(struct iphdr) );
		printf("size ip header = %ld", sizeof(struct iphdr));
		printf("received type = %d code = %d checksum = %d\n", resp_packet->icmp_header.type, resp_packet->icmp_header.code, resp_packet->icmp_header.checksum);
		printf("%d bytes from %s\n", recv_ret, inet_ntoa(get_target_sock_addr()->sin_addr));
	}

	return EXIT_SUCCESS;
}
