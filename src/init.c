#include "init.h"
#include "debug.h"

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h> /*inet_ntoa*/
#include <argp.h>

static bool dns_resolv(ping_env_t *env)
{
	struct hostent *host = gethostbyname(env->target);

	if (host == NULL)
	{
		DEBUG("Failed to resolve host\n");
		return false;
	}

	const char *ip_dest = inet_ntoa(*(struct in_addr *)host->h_addr_list[0]);

	DEBUG("ip_dest = [%s]", ip_dest);

	if (strlen(ip_dest) < INET_ADDRSTRLEN)
		strncpy(env->ip_addr, ip_dest, INET_ADDRSTRLEN);

	DEBUG("g_env_ip = %s", env->ip_addr);

	if (inet_pton(AF_INET, env->ip_addr, &env->target_sock_addr.sin_addr) != 0)
	{
		env->target_sock_addr.sin_family = AF_INET;
		env->target_sock_addr.sin_port = 0;
		DEBUG("Resolved is valid ip v4");
	}

	return true;
}

bool init(ping_env_t *env)
{
	DEBUG("count=%d", env->count);

	env->process_id = getpid();
	DEBUG("process id = %d", env->process_id);
	env->identity = (env->process_id & 0xFFFF);

	if (inet_pton(AF_INET, env->target, &env->target_sock_addr.sin_addr) != 0)
	{
		env->target_sock_addr.sin_family = AF_INET;
		env->target_sock_addr.sin_port = 0;
		DEBUG("argument is valid ip v4");
		return true;
	}

	DEBUG("argument is not a valid ip address try to resolve it");
	if ( !dns_resolv(env) )
	{
		fprintf(stderr, "ping: %s: Name or service not known", env->target);

		return false;
	}

	return true;
}
