#include <sys/socket.h>		 /* socket(), sendto(), SOCK_RAW, AF_INET*/
#include <stdlib.h>			 /* EXIT_FAILURE EXIT_SUCCES */
#include <stdio.h>			 /* printf() */
#include <netinet/ip_icmp.h> /* IPPROTO_ICMP */
#include <stdbool.h>		 /* bool */
#include <netinet/ip.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>			/* inet_ntoa */


#include "ft_ping.h"
#include "arguments_parsing.h"
#include "init.h"
#include "run.h"

void print_stats(ping_env_t *env)
{
	printf("--- %s ping statistics ---\n", env->target);
	printf("%zu packets transmitted, %zu packets received, ", env->sent_pings, env->received_pings);
	if (env->duplicated_pings != 0)
		printf("+%zu duplicates, ", env->duplicated_pings);
	if (env->sent_pings != 0)
	{
		int percentage = ( ( env->sent_pings - env->received_pings) * 100 ) / env->sent_pings;
		printf("%d%% packet loss", percentage);
	}
	printf("END\n");
}

void print_header(ping_env_t *env)
{
	printf("FT_PING %s (%s): %zu data bytes",
	       env->target, inet_ntoa(env->target_sock_addr.sin_addr), env->payload_size);
	if (env->verbose)
		printf(", id 0x%04x = %u", env->identity, env->identity);
	
	printf("\n");
}

int main(int ac, char **av)
{
	int fd = 0;
	ping_env_t env = {
		.interval_ms = 1000,
		.payload_size = 64
	};

	if (!parse_args(&env, ac, av))
		return EXIT_FAILURE;

	if (!init(&env))
		return EXIT_FAILURE;

	signal(SIGINT, signal_handler);

	if (!open_icmp_socket(&fd))
		return EXIT_FAILURE;

	print_header(&env);

	if (!main_loop_run(&env, fd))
		return EXIT_FAILURE;

	print_stats(&env);

	return EXIT_SUCCESS;
}
