#ifndef DEBUG_H
#define DEBUG_H

#include "ft_ping.h"

#ifdef ENABLE_DEBUG
    #define DEBUG(fmt, ...) printf("[DEBUG]: " fmt "\n", ##__VA_ARGS__)
#else
    #define DEBUG(fmt, ...)
#endif

#define PING_ERR(fmt, ...) fprintf(stderr, "ft_ping: " fmt "\n", ##__VA_ARGS__)

#endif /* DEBUG_H */
