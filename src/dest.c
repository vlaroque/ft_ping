#include "dest.h"
#include <string.h>      /* memset() */
#include <arpa/inet.h>
#include <stdio.h>       /* stderr printf */

struct sockaddr_in get_dest(void)
{
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    if ( inet_aton("127.0.0.1", &addr.sin_addr) == 0 )
    {
        fprintf(stderr, "failed to convert ip");
    }

    return addr;
}
