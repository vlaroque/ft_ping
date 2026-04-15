#include <argp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "arguments_parsing.h"
#include "init.h"


static const char doc[]      = "ft_ping - send ICMP ECHO_REQUEST to network hosts";
static const char args_doc[] = "<DESTINATION>";

static struct argp_option options[] = {
    {"count",   'c', "count",  0, "stop after <count> replies",                    0},
    {"size",    's', "bytes",  0, "use <size> as number of data bytes to be sent", 0},
    {"verbose", 'v', 0,        0, "verbose output",                                0},
    {"debug",   'x', 0,        0, "debug output",                                  0},
    {0}
};

static error_t args_parsing_fonction(int key, char *arg, struct argp_state *state)
{
	ping_env_t *ping_env = state->input;

	switch (key)
	{
	case 'v':
		ping_env->verbose = 1;
		break;

	case 'c':
		ping_env->count = strtol(arg, NULL, 10);
		break;

	case 's':
		ping_env->size = atoi(arg);
		break;

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
