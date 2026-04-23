#include <sys/socket.h>		 /* socket(), sendto(), SOCK_RAW, AF_INET*/
#include <stdlib.h>			 /* EXIT_FAILURE EXIT_SUCCES */
#include <stdio.h>			 /* printf() */
#include <netinet/ip_icmp.h> /* IPPROTO_ICMP */
#include <stdbool.h>		 /* bool */
#include <netinet/ip.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h> /* inet_ntoa */

#include "ft_ping.h"
#include "arguments_parsing.h"
#include "init.h"
#include "run.h"
#include "utils.h"

#define DEFAULT_SIZE (64 - sizeof(struct icmphdr))

void print_stats(ping_env_t *env)
{
	printf("--- %s ping statistics ---\n", env->target);
	printf("%zu packets transmitted, %zu packets received, ", env->sent_pings, env->received_pings);

	if ( env->duplicated_pings != 0 )
		printf("+%zu duplicates, ", env->duplicated_pings );

	if ( env->sent_pings != 0)
	{
		int percentage = ((env->sent_pings - env->received_pings) * 100) / env->sent_pings;
		printf("%d%% packet loss", percentage);

		if ( env->ping_support_timing && (env->received_pings > 0) )
		{
			double total_received = env->received_pings + env->duplicated_pings;
			double average        = env->total_time / total_received;
			double variation      = env->square_time / total_received - average * average;

			printf("\nround-trip min/avg/max/stddev = ");
			printf("%.3f/", env->min_time);
			printf("%.3f/", average);
			printf("%.3f/", env->max_time);
			printf("%.3f ms", ping_sqrt(variation));
		}
	}

	printf("\n");
}

void print_header(ping_env_t *env)
{
	printf("FT_PING %s (%s): %zu data bytes",
		   env->target, inet_ntoa(env->target_sock_addr.sin_addr), env->size);
	if (env->verbose)
		printf(", id 0x%04x = %u", env->identity, env->identity);

	printf("\n");
}

int main(int ac, char **av)
{
	int fd = 0;
	ping_env_t env = {
		.interval_ms = 1000,
		.size = DEFAULT_SIZE,
		.ping_support_timing = true,
		.min_time = 99999999999.0,
		.preload = 0,
		.count = -1,
		.ttl = 0,
	};

	if (!parse_args(&env, ac, av))
		return EXIT_FAILURE;

	if (!init(&env))
		return EXIT_FAILURE;

	signal(SIGINT, signal_handler);

	if (!open_icmp_socket(&env, &fd))
		return EXIT_FAILURE;

	print_header(&env);

	if (!main_loop_run(&env, fd))
		return EXIT_FAILURE;

	print_stats(&env);

	return EXIT_SUCCESS;
}
