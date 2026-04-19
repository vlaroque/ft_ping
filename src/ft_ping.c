#include <sys/socket.h>		 /* socket(), sendto(), SOCK_RAW, AF_INET*/
#include <stdlib.h>			 /* EXIT_FAILURE EXIT_SUCCES */
#include <stdio.h>			 /* printf() */
#include <netinet/ip_icmp.h> /* IPPROTO_ICMP */
#include <stdbool.h>		 /* bool */
#include <arpa/inet.h>		 /* inet_ntoa*/
#include <netinet/ip.h>
#include <errno.h>
#include <string.h>
#include <time.h> /*timespec*/
#include <poll.h> /*pollfd*/

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

double timespec_diff_in_ms(struct timespec start, struct timespec end)
{
	long seconds = end.tv_sec - start.tv_sec;
	long nano_seconds = end.tv_nsec - start.tv_nsec;

	if (nano_seconds < 0)
	{
		seconds -= 1;
		nano_seconds += 1000000000L;
	}

	return (double) seconds * 1000.0 + (double) nano_seconds / 1000000.0;
}

bool receive_pong(int fd, int timeout_ms)
{
	struct timespec start_time, current_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	while (true)
	{
		uint8_t recv_buff[IP_MAXPACKET] = {0};
		socklen_t sender_address_len = sizeof(struct sockaddr_in);
		struct sockaddr_in sender_address;

		struct pollfd poll_fd = {
			.fd = fd,
			.events = POLLIN};

		int ret = poll(&poll_fd, 1, timeout_ms);

		if ( ret == 0 )
		{
			break;
		}
		else if ( ret == -1 )
		{
			PING_ERR("poll error: %s", strerror(errno));
		}

		clock_gettime(CLOCK_MONOTONIC, &current_time);

		int recv_ret = recvfrom(fd,
								(void *)recv_buff,
								IP_MAXPACKET,
								0,
								(struct sockaddr *)&sender_address,
								&sender_address_len);

		if (recv_ret == -1)
			printf("Failed to receive\n");
		else
		{
			struct iphdr *ip_hdr = (struct iphdr *)recv_buff;

			icmp_packet_t *resp_packet = (icmp_packet_t *)(recv_buff + (ip_hdr->ihl * 4));

			double rtt = timespec_diff_in_ms(resp_packet->time_stamp, current_time);

			printf("%d bytes from %s received type = %d code = %d id = %d icmp_seq = %d checksum=%d time=%f\n",
				   recv_ret,
				   inet_ntoa(sender_address.sin_addr),
				   resp_packet->icmp_header.type,
				   resp_packet->icmp_header.code,
				   ntohs(resp_packet->icmp_header.un.echo.id),
				   ntohs(resp_packet->icmp_header.un.echo.sequence),
				   resp_packet->icmp_header.checksum,
				   rtt);
		}
	}

	return true;
}

bool main_loop_run(ping_env_t *env, int fd)
{
	icmp_packet_t packet = icmp_packet_init(env->process_id, 0);
	uint16_t sequence_id = 1;

	while (true)
	{
		// struct sockaddr_in addr = get_dest();
		icmp_packet_update(&packet, sequence_id);

		ssize_t len = sendto(fd, &packet, sizeof(icmp_packet_t), 0, (struct sockaddr *)&env->target_sock_addr, sizeof(struct sockaddr_in));

		if (len == 0)
		{
			DEBUG("Failed to send packet\n");
			return false;
		}
		else if (len == -1)
		{
			PING_ERR("connect: %s", strerror(errno));
			return false;
		}

		DEBUG("send %ld bytes\n", len);

		receive_pong(fd, env->interval_ms);
		sequence_id++;
	}

	return true;
}

int main(int ac, char **av)
{
	int fd = 0;
	ping_env_t env = {
		.interval_ms = 1000
	};

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
