#include <stdio.h>
#include <errno.h>
#include <poll.h>	   /* pollfd */
#include <string.h>	   /* strerror */
#include <arpa/inet.h> /* inet_ntoa */
#include <time.h>	   /* timespec */
#include <signal.h>	   /* sig_atomic_t */

#include "run.h"
#include "debug.h"

volatile sig_atomic_t g_keep_running = 1;

void signal_handler(int signal)
{
	DEBUG("signal received : %s", strsignal(signal));
	(void)signal;
	g_keep_running = 0;
}

static double timespec_diff_in_ms(struct timespec start, struct timespec end)
{
	long seconds = end.tv_sec - start.tv_sec;
	long nano_seconds = end.tv_nsec - start.tv_nsec;

	if (nano_seconds < 0)
	{
		seconds -= 1;
		nano_seconds += 1000000000L;
	}

	return (double)seconds * 1000.0 + (double)nano_seconds / 1000000.0;
}

bool echo_reply_handle(ping_env_t *env, icmp_packet_t *resp_packet, ssize_t recv_ret, struct sockaddr_in* sender_addr, double rtt)
{
	uint16_t identity = ntohs(resp_packet->icmp_header.un.echo.id);

	if ( identity != env->identity)
		return false;

	env->received_pings += 1;

	DEBUG("%zu bytes from %s received type = %d code = %d id = %d icmp_seq = %d checksum=%d time=%f\n",
	  recv_ret,
	  inet_ntoa(sender_addr->sin_addr),
	  resp_packet->icmp_header.type,
	  resp_packet->icmp_header.code,
	  ntohs(resp_packet->icmp_header.un.echo.id),
	  ntohs(resp_packet->icmp_header.un.echo.sequence),
	  resp_packet->icmp_header.checksum,
	  rtt);

	return true;
}

bool receive_pong(ping_env_t *env, int fd)
{
	struct timespec start_time, current_time;
	struct pollfd poll_fd =
		{
			.fd = fd,
			.events = POLLIN};

	clock_gettime(CLOCK_MONOTONIC, &start_time);

	while (g_keep_running)
	{
		uint8_t recv_buff[IP_MAXPACKET] = {0};
		socklen_t sender_address_len = sizeof(struct sockaddr_in);
		struct sockaddr_in sender_address;

		clock_gettime(CLOCK_MONOTONIC, &current_time);
		double elapsed_time_ms = timespec_diff_in_ms(start_time, current_time);
		double remaining_time_ms = env->interval_ms - elapsed_time_ms;

		if ( remaining_time_ms <= 0 )
			return true;

		int ret = poll(&poll_fd, 1, remaining_time_ms);

		if (ret == 0) /* in case of TIMEOUT */
			return true;

		if (ret < 0)
		{
			if (errno == EINTR) /* interruption */
				continue;

			PING_ERR("poll error: %s", strerror(errno));
		}

		clock_gettime(CLOCK_MONOTONIC, &current_time);

		ssize_t recv_ret = recvfrom(fd, (void *)recv_buff, IP_MAXPACKET, 0,
		                           (struct sockaddr *)&sender_address,
		                           &sender_address_len);

		if (recv_ret == -1)
		{
			printf("Failed to receive\n");
			continue;
		}

		struct iphdr *ip_hdr = (struct iphdr *)recv_buff;
		icmp_packet_t *resp_packet = (icmp_packet_t *)(recv_buff + (ip_hdr->ihl * 4));

		if (resp_packet->icmp_header.type == ICMP_ECHOREPLY)
		{
			double rtt = timespec_diff_in_ms(resp_packet->time_stamp, current_time);

			echo_reply_handle(env, resp_packet, recv_ret, &sender_address, rtt);
		}
		else if (resp_packet->icmp_header.type == ICMP_TIMESTAMPREPLY
			|| resp_packet->icmp_header.type == ICMP_ADDRESSREPLY
			|| resp_packet->icmp_header.type == ICMP_ECHO
			|| resp_packet->icmp_header.type == ICMP_TIMESTAMP
			|| resp_packet->icmp_header.type == ICMP_ADDRESS)
		{
			/* ignore these packets*/
			continue;
		}
		else
		{
			// check if it's my reply
		}
	}

	return true;
}

bool main_loop_run(ping_env_t *env, int fd)
{
	icmp_packet_t packet = icmp_packet_init(env->identity, 0);
	uint16_t sequence_id = 1;

	while (g_keep_running)
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
		else
			env->sent_pings += 1;

		DEBUG("send %ld bytes\n", len);

		receive_pong(env, fd);
		sequence_id++;
	}

	return true;
}
