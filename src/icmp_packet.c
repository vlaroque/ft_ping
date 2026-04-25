/*****************************************************************************/
/*                                                                           */
/*                                                🭈🭆🭂███🭞🭜🭘  ███🭞🭜🭘██████    */
/*   icmp_packet.c                             🭈🭆🭂███🭞🭜🭘     🭞🭜🭘   ██████    */
/*                                          🭈🭆🭂███🭞🭜🭘           🭈🭆🭂███🭞🭜🭘    */
/*   By: vlaroque                        🭈🭆🭂███🭞🭜🭘           🭈🭆🭂███🭞🭜🭘       */
/*       vlaroque@student.42.fr          ██████████████████  ██████   🭈🭆🭂    */
/*                                       ██████████████████  ██████🭈🭆🭂███.fr */
/*                                                   ██████   🬚🬢🬞🬋🬏🬚🬢🬦🬞🬋🬃    */
/*                                                   ██████   🬕🬀▐🬂▌🬕🬣▐🬞🬰🬃    */
/*                                                                           */
/*****************************************************************************/

#include <string.h> /* memset */
#include <unistd.h> /* pid_t + getpid() */
#include <sys/types.h> /* pid_t */
#include <arpa/inet.h> /* htons */
#include <errno.h>

#include "icmp_packet.h"
#include "ft_ping.h"
#include "printing.h"

static uint16_t calculate_checksum(uint8_t *buffer, int length) {
	uint32_t result = 0;
	uint16_t *data = (uint16_t *)buffer;

	while (length > 1) {
		result += *data;
		data++;
		length -= 2;
	}

	if (length == 1) {
		result += *(uint8_t *)data;
	}

	while (result >> 16) {
		result = (result & 0xFFFF) + (result >> 16);
	}

	return ~result;
}

icmp_packet_t icmp_packet_init(uint16_t identity, uint16_t seq)
{
	icmp_packet_t res;
	memset(&res, 0, sizeof(icmp_packet_t));

	res.icmp_header.type             = ICMP_ECHO;
	res.icmp_header.code             = 0;
	res.icmp_header.checksum         = 0;
	res.icmp_header.un.echo.id       = htons(identity);
	DEBUG("id of ping : %d", ntohs(res.icmp_header.un.echo.id));
	res.icmp_header.un.echo.sequence = htons(seq);

	return res;
}

void icmp_packet_update(icmp_packet_t *packet, uint16_t sequence, size_t size, bool ping_support_timing)
{
	packet->icmp_header.un.echo.sequence = htons(sequence);
	packet->icmp_header.checksum         = 0;

	if (ping_support_timing)
		clock_gettime(CLOCK_MONOTONIC, &packet->time_stamp);

	packet->icmp_header.checksum         = calculate_checksum((uint8_t *)packet, size);
}

bool open_icmp_socket(ping_env_t *env, int *fd)
{
	int ret_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	env->using_raw_socket = true;

	if (ret_fd < 0 && ( errno == EPERM || errno == EACCES))
	{
		errno = 0;
		ret_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
		
		int on = 1;
		setsockopt(ret_fd, IPPROTO_IP, IP_RECVTTL, &on, sizeof(on));

		env->using_raw_socket = false;
		DEBUG("ICMP SOCKET : no id check");
	}

	if (ret_fd < 0)
	{
		PING_ERR("icmp open socket: %s.", strerror(errno));
		return false;
	}

	int on = 1;
	setsockopt(ret_fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

	if (env->ttl > 0)
	{
		setsockopt(ret_fd, IPPROTO_IP, IP_TTL, &env->ttl, sizeof(env->ttl));
	}

	DEBUG("fd opened %d\n", ret_fd);
	*fd = ret_fd;

	return true;
}
