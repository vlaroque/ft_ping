#ifndef FT_PING_H
#define FT_PING_H

#include <unistd.h>             /* pid gid */
#include <netinet/in.h>         /* INET_ADDRSTRLEN */


typedef struct ping_env_s
{
	uid_t               user_id;
	pid_t               process_id;
	bool                is_root;
	int                 interval_ms;
	bool                verbose;
	bool                print_help;
	int                 count;
	int                 size;
	char               *target;
	struct sockaddr_in  target_sock_addr;
	char                ip_addr[INET_ADDRSTRLEN];
} ping_env_t;


#endif /* FT_PING_H */
