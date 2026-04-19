#include <sys/socket.h>		 /* socket(), sendto(), SOCK_RAW, AF_INET*/
#include <stdlib.h>			 /* EXIT_FAILURE EXIT_SUCCES */
#include <stdio.h>			 /* printf() */
#include <netinet/ip_icmp.h> /* IPPROTO_ICMP */
#include <stdbool.h>		 /* bool */
#include <arpa/inet.h>		 /* inet_ntoa*/
#include <netinet/ip.h>
#include <errno.h>
#include <string.h>

#include "ft_ping.h"
#include "arguments_parsing.h"
#include "init.h"
#include "icmp_packet.h"
#include "dest.h"
#include "debug.h"

#define MTU 1500

bool open_icmp_socket(int *fd)
{
	int ret_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	// socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

	if (ret_fd < 2)
	{
		printf("ping: icmp open socket: %s.\n", strerror(errno));

		return false;
	}

	DEBUG("fd opened %d\n", ret_fd);
	*fd = ret_fd;

	return true;
}

bool main_loop_run(ping_env_t *env, int fd)
{
	icmp_packet_t packet = icmp_packet_init(0);
	uint16_t sequence_id = 1;

	while (true)
	{
		// struct sockaddr_in addr = get_dest();
		icmp_packet_update(&packet, sequence_id);

		ssize_t len = sendto(fd, &packet, sizeof(icmp_packet_t), 0, (struct sockaddr *)&env->target_sock_addr, sizeof(struct sockaddr_in));

		if (len == 0)
		{
			fprintf(stderr, "Failed to send packet\n");
			return false;
		}
		else if (len == -1)
		{
			PING_ERR("connect: %s", strerror(errno));
			return false;
		}
		else
		{
			printf("send %ld bytes\n", len);
		}

		uint8_t recv_buff[MTU];
		socklen_t addr_len = sizeof(struct sockaddr_in);
		int recv_ret = recvfrom(fd, (void *)recv_buff, MTU, 0, (struct sockaddr *)&env->target_sock_addr, &addr_len);

		if (recv_ret == -1)
			printf("Failed to receive\n");
		else
		{
			icmp_packet_t *resp_packet = (icmp_packet_t *)(recv_buff + sizeof(struct iphdr));
			printf("size ip header = %ld", sizeof(struct iphdr));
			printf("received type = %d code = %d checksum = %d\n",
			       resp_packet->icmp_header.type, resp_packet->icmp_header.code, resp_packet->icmp_header.checksum);
			printf("%d bytes from %s\n", recv_ret, inet_ntoa(env->target_sock_addr.sin_addr));
		}
		
		sequence_id++;
	}

	return true;
}

int main(int ac, char **av)
{
	int fd = 0;
	ping_env_t env = {0};

	if (!parse_args(&env, ac, av))
		return EXIT_FAILURE;

	if (!init(&env))
		return EXIT_FAILURE;

	if (!open_icmp_socket(&fd))
		return EXIT_FAILURE;

	if (!main_loop_run(&env, fd))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
