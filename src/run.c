#include <stdio.h>
#include <errno.h>
#include <poll.h>	   /* pollfd */
#include <string.h>	   /* strerror */
#include <arpa/inet.h> /* inet_ntoa */
#include <time.h>	   /* timespec */
#include <signal.h>	   /* sig_atomic_t */
#include <netinet/ip.h>

#include "run.h"
#include "debug.h"
#include "duplicate.h"
#include "error_reply.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

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

void echo_reply_take_stats(ping_env_t *env, int rtt)
{
	env->total_time += rtt;
	env->square_root_time += ( rtt * rtt );
	if ( rtt < env->min_time )
		env->min_time = rtt;
	if ( rtt > env->max_time )
		env->max_time = rtt;
}

void echo_reply_print(size_t len, struct in_addr *sender, uint16_t seq,
                      uint8_t ttl, bool time, double rtt, bool duplicate)
{
	printf("%ld bytes from %s: icmp_seq=%u ttl=%d", len, inet_ntoa(*sender), seq, ttl);

	if (time)
		printf(" time=%.3f ms", rtt);
	if (duplicate)
		printf(" (DUP!)");

	printf("\n");
}

bool echo_reply_handle(ping_env_t *env, icmp_packet_t *resp_packet, struct sockaddr_in* sender_addr,
                       double rtt, size_t icmp_packet_size, uint8_t ttl)
{
	bool     is_duplicate = false;
	bool     contain_time = true;

	uint16_t identity     = ntohs(resp_packet->icmp_header.un.echo.id);
	
	if ( identity != env->identity)
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

	echo_reply_take_stats(env, rtt);

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

bool is_echo_reply(ping_env_t *env, struct icmphdr *icmp_header)
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

bool receiving_loop(ping_env_t *env, int fd)
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

		struct iphdr  *ip_header        = (struct iphdr *)recv_buff;
		size_t         ip_header_size   = ip_header->ihl * 4;
		icmp_packet_t *resp_packet      = (icmp_packet_t *)(recv_buff + ip_header_size);
		size_t         icmp_packet_size = recv_ret - ip_header_size;

		if (resp_packet->icmp_header.type == ICMP_ECHOREPLY)
		{
			double rtt_in_ms = timespec_diff_in_ms(resp_packet->time_stamp, current_time);

			echo_reply_handle(env, resp_packet, &sender_address, rtt_in_ms, icmp_packet_size, ip_header->ttl);
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
			if (!is_echo_reply(env, (struct icmphdr *)resp_packet))
				continue;
			
			echo_error_reply_handle(resp_packet, &sender_address, icmp_packet_size);
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
		icmp_packet_update(&packet, sequence_id, env->size);

		ssize_t len = sendto(fd, &packet, env->size, 0, (struct sockaddr *)&env->target_sock_addr, sizeof(struct sockaddr_in));

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

		receiving_loop(env, fd);
		sequence_id++;
	}

	return true;
}
