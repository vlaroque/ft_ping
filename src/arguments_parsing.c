#include <argp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <time.h>         /* struct timespec */

#include "arguments_parsing.h"
#include "init.h"
#include "printing.h"


static const char doc[]      = "ft_ping - send ICMP ECHO_REQUEST to network hosts";
static const char args_doc[] = "<DESTINATION>";

static struct argp_option options[] = {
	{"count",    'c', "count",   0, "stop after <count> replies",                              0},
	{"preload",  'l', "count",   0, "send <preload> number of packages while waiting replies", 0},
	{"interval", 'i', "seconds", 0, "seconds between sending each packet",                     0},
	{"size",     's', "bytes",   0, "use <size> as number of data bytes to be sent",           0},
	{"ttl",      't', "<TTL>",   0, "define time to live",                                     0},
	{"verbose",  'v',  0,        0, "verbose output",                                          0},
	{0}
};

static error_t args_parsing_fonction(int key, char *arg, struct argp_state *state)
{
	ping_env_t *ping_env = state->input;
	char *end_ptr;

	switch (key)
	{
	case 'v':
		ping_env->verbose = true;
		break;

	case 'c':
		ping_env->count = strtol(arg, &end_ptr, 10);
		break;

	case 'l':
		unsigned long preload = strtoul(arg, &end_ptr, 0);

		if (*end_ptr || preload > INT_MAX)
		{
			argp_error(state, "invalid preload value (%s)", arg);
		}

		ping_env->preload = preload;
		break;

	case 'i':
		double interval = strtod(arg, &end_ptr);
		if ( arg == end_ptr || *end_ptr != '\0' || errno == ERANGE
			 || interval < 0 || interval > (INT_MAX / 1000 ) )
		{
			argp_error(state, "Invalid value for interval: '%s'", arg);
		}
		ping_env->interval_ms = interval * 1000;

		break;

	case 's':
	{
		long size = strtol(arg, &end_ptr, 10);
		if (arg == end_ptr || *end_ptr != '\0' || errno == ERANGE || size < 0 || size > IP_MAXPACKET)
		{
			argp_error(state, "Invalid value for packet size: '%s'. Limit is 0 to %d.", arg, IP_MAXPACKET);
		}

		ping_env->size = size;

		if ( ping_env->size < sizeof(struct timespec))
		{
			DEBUG("NO TIMING supported size of struct timespec %zu", sizeof(struct timespec));
			ping_env->ping_support_timing = false;
		}
		break;
	}

	case ARGP_KEY_ARG:
		if (state->arg_num >= 1)
		{
			/* Trop d'arguments */
			argp_usage(state);
		}

		ping_env->target = arg;
		break;

	case ARGP_KEY_END:
		if (state->arg_num < 1)
		{
			/* Pas assez d'arguments (on attend la destination) */
			argp_usage(state);
		}
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = {options, args_parsing_fonction, args_doc, doc, NULL, NULL, NULL};

bool parse_args(ping_env_t *env, int ac, char **av)
{
	argp_parse(&argp, ac, av, 0, 0, env);
	return true; // todo manage errorss
}
