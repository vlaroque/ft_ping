#ifndef FT_PING_H
#define FT_PING_H

#include <unistd.h>             /* pid gid */
#include <netinet/in.h>         /* INET_ADDRSTRLEN */
#include <stdint.h>             /* UINT16_MAX */

#define SEQ_BIT_MAP_SIZE (UINT16_MAX/8+1)

typedef struct ping_env_s
{
	uid_t               user_id;
	pid_t               process_id;
	uint16_t            identity;
	bool                is_root;
	int                 interval_ms;
	bool                verbose;
	bool                print_help;
	bool                ping_support_timing;
	bool                id_check;
	size_t              count;
	size_t              size;                   /* size of icmp packet */
	int                 preload;
	char               *target;
	struct sockaddr_in  target_sock_addr;
	char                ip_addr[INET_ADDRSTRLEN];

	size_t              sent_pings;
	size_t              received_pings;
	size_t              duplicated_pings;

	double              total_time;
	double              min_time;
	double              max_time;
	double              square_time;

	uint8_t             seq_bit_map[SEQ_BIT_MAP_SIZE];
} ping_env_t;


#endif /* FT_PING_H */
