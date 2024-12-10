#ifndef INIT_H
#define INIT_H

#include <unistd.h>             /* pid gid */
#include <stdbool.h>            /* bool */
#include <netinet/in.h>         /* INET_ADDRSTRLEN */

typedef struct env_s 
{
	uid_t user_id;
	pid_t process_id;
	bool is_root;
	bool debug;
	bool verbose;
	bool print_help;
	char *target;
    struct sockaddr_in target_sock_addr;
	char ip_addr[INET_ADDRSTRLEN];
} env_t;

bool init(void);
bool parse_args(int ac, char **av);
pid_t get_process_id();
struct sockaddr_in *get_target_sock_addr();


#endif /*INIT_H*/
