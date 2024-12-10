#include "init.h"

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h> /*inet_ntoa*/

env_t g_env;

static bool dns_resolv()
{
    struct hostent *host = gethostbyname(g_env.target);

    if (host == NULL)
    {
        printf("Failed to resolve host\n");
        return false;
    }

    const char *ip_dest = inet_ntoa(*(struct in_addr *)host->h_addr_list[0]);
    printf("ip_dest = [%s]\n", ip_dest);

    if ( strlen(ip_dest) < INET_ADDRSTRLEN )
        strcpy(g_env.ip_addr, ip_dest);

    printf("g_env_ip = %s\n", g_env.ip_addr);

    if ( inet_pton(AF_INET, g_env.ip_addr, &g_env.target_sock_addr.sin_addr) != 0 )
    {
        g_env.target_sock_addr.sin_family = AF_INET;
        g_env.target_sock_addr.sin_port = 0;
        printf("Resolved is valid ip v4\n");
    }

    return true;
}

static void print_help()
{
    printf("TODO HELP\n");
}

bool init(void)
{
	g_env.debug = true;

    if (g_env.print_help)
    {
        print_help();
        return false;
    }

    if ( inet_pton(AF_INET, g_env.target, &g_env.target_sock_addr.sin_addr) != 0 )
    {
        g_env.target_sock_addr.sin_family = AF_INET;
        g_env.target_sock_addr.sin_port = 0;
        printf("argument is valid ip v4\n");
    }
    else
    {
        printf("argument is not a valid ip address\n");
        dns_resolv();
    }

	return true;
}

bool parse_args(int ac, char **av)
{
    bool target_found = false;
    g_env.verbose = false;
    g_env.print_help = false;

    if (ac <= 1)
        return false;

    int i = 1;
    while ( i < ac )
    {
        if ( !(av[i][0] == '-') )
        {
            if (target_found)
                return false;

            g_env.target = av[i];
            target_found = true;
        }
        else
        {
            int j = 1;
            while ( av[i][j] != '\0' )
            {
                switch (av[i][j])
                {
                case 'v':
                    g_env.verbose = true;
                    break;
                case '?':
                    g_env.print_help = true;
                    break;
                
                default:
                    break;
                }
            j++;
            }
        }
        i++;
    }
    return true;
}

char *dest_ip_str()
{
    return g_env.target;
}

pid_t get_process_id()
{
    return g_env.process_id;
}

struct sockaddr_in *get_target_sock_addr()
{
    return &g_env.target_sock_addr;
}
