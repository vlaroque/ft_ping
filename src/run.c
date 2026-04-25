/*****************************************************************************/
/*                                                                           */
/*                                                🭈🭆🭂███🭞🭜🭘  ███🭞🭜🭘██████    */
/*   run.c                                     🭈🭆🭂███🭞🭜🭘     🭞🭜🭘   ██████    */
/*                                          🭈🭆🭂███🭞🭜🭘           🭈🭆🭂███🭞🭜🭘    */
/*   By: vlaroque                        🭈🭆🭂███🭞🭜🭘           🭈🭆🭂███🭞🭜🭘       */
/*       vlaroque@student.42.fr          ██████████████████  ██████   🭈🭆🭂    */
/*                                       ██████████████████  ██████🭈🭆🭂███.fr */
/*                                                   ██████   🬚🬢🬞🬋🬏🬚🬢🬦🬞🬋🬃    */
/*                                                   ██████   🬕🬀▐🬂▌🬕🬣▐🬞🬰🬃    */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <poll.h>	   /* pollfd */
#include <string.h>	   /* strerror */
#include <arpa/inet.h> /* inet_ntoa */
#include <time.h>	   /* timespec */
#include <signal.h>	   /* sig_atomic_t */
#include <netinet/ip.h>

#include "run.h"
#include "printing.h"
#include "duplicate.h"
#include "error_reply.h"
#include "reply_handle.h"
#include "utils.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

volatile sig_atomic_t g_keep_running = 1;

void signal_handler(int signal)
{
	(void)signal;
	g_keep_running = 0;
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
		struct sockaddr_in sender_address;

		clock_gettime(CLOCK_MONOTONIC, &current_time);
		double elapsed_time_ms = timespec_diff_in_ms(start_time, current_time);
		double remaining_time_ms = env->interval_ms - elapsed_time_ms;

		if ( remaining_time_ms <= 0 )
			return true;

		int ret = poll(&poll_fd, 1, remaining_time_ms);

		if (ret == 0) /* in case of TIMEOUT */
		{
			DEBUG("POLL TIMEOUT");
			return true;
		}
		else if (ret < 0)
		{
			if (errno == EINTR) /* interruption */
				continue;

			PING_ERR("poll error: %s", strerror(errno));
		}

		clock_gettime(CLOCK_MONOTONIC, &current_time);

		struct iovec iov = { .iov_base = recv_buff, .iov_len = sizeof(recv_buff) };
		uint8_t cmsg_buf[256];
		struct msghdr msg = {
			.msg_name = &sender_address,
			.msg_namelen = sizeof(sender_address),
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_control = cmsg_buf,
			.msg_controllen = sizeof(cmsg_buf)
		};

		errno = 0;
		ssize_t recv_ret = recvmsg(fd, &msg, 0);

		DEBUG("PACKET RECEIVED");
		if (recv_ret == -1)
		{
			if (errno != EINTR && errno != EAGAIN)
				printf("Failed to receive: %s\n", strerror(errno));

			continue;
		}

		size_t         ip_header_size   = 0;
		uint8_t        ttl              = 0;

		if (env->using_raw_socket)
		{
			struct iphdr *ip_header = (struct iphdr *)recv_buff;
			ip_header_size = ip_header->ihl * 4;
			ttl = ip_header->ttl;
		}
		else
		{
			struct cmsghdr *cmsg;
			for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
				if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TTL) {
					ttl = *((int *)CMSG_DATA(cmsg));
				}
			}
		}

		icmp_packet_t *resp_packet      = (icmp_packet_t *)(recv_buff + ip_header_size);
		size_t         icmp_packet_size = recv_ret - ip_header_size;

		DEBUG("icmp_packet_size = %zu", icmp_packet_size);

		if (resp_packet->icmp_header.type == ICMP_ECHOREPLY)
		{
			double rtt_in_ms = 0;
			
			if ( env->ping_support_timing && icmp_packet_size >= ICMP_MIN_SIZE_FOR_TIMING)
				rtt_in_ms = timespec_diff_in_ms(resp_packet->time_stamp, current_time);

			echo_reply_handle(env, resp_packet, &sender_address, rtt_in_ms, icmp_packet_size, ttl);
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
			if (!is_icmp_packet_echo_reply(env, (struct icmphdr *)resp_packet))
				continue;
			
			echo_error_reply_handle(env, resp_packet, &sender_address, icmp_packet_size);
		}
	}

	return true;
}

bool send_echo_request(ping_env_t *env, int fd, icmp_packet_t *packet, uint16_t sequence_id)
{
	size_t total_size = env->size + sizeof(struct icmphdr);
	icmp_packet_update(packet, sequence_id, total_size, env->ping_support_timing);

	ssize_t len = sendto(fd, packet, total_size, 0, (struct sockaddr *)&env->target_sock_addr, sizeof(struct sockaddr_in));

	if (len == 0)
	{
		PING_ERR("Failed to send packet\n");
		return false;
	}
	else if (len == -1)
	{
		PING_ERR("sendto: %s", strerror(errno));
		return false;
	}

	DEBUG("send %ld bytes\n", len);

	return true;
}

bool main_loop_run(ping_env_t *env, int fd)
{
	icmp_packet_t packet = icmp_packet_init(env->identity, 0);
	uint16_t sequence_id = 0;

	for (long i = 0; i < env->preload; i++)
	{
		seq_bit_clear_ack_received(env, sequence_id);

		if ( send_echo_request(env, fd, &packet, sequence_id))
			env->sent_pings += 1;
		else
			return false;

		sequence_id++;
		if ( env->count > 0 && sequence_id >= env->count)
			return true;
	}

	while (g_keep_running)
	{
		// struct sockaddr_in addr = get_dest();

		seq_bit_clear_ack_received(env, sequence_id);

		if ( send_echo_request(env, fd, &packet, sequence_id ))
			env->sent_pings += 1;
		else
			return false;

		receiving_loop(env, fd);
		sequence_id++;
		if ( env->count > 0 && sequence_id >= env->count)
			return true;
	}

	return true;
}
