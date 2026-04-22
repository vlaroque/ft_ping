#ifndef PRINTING_H
#define PRINTING_H

#include <stdio.h>

#include "ft_ping.h"

#ifdef ENABLE_DEBUG
    #define DEBUG(fmt, ...) printf("[DEBUG]: " fmt "\n", ##__VA_ARGS__)
#else
    #define DEBUG(fmt, ...)
#endif

#define PING_ERR(fmt, ...) fprintf(stderr, "ft_ping: " fmt "\n", ##__VA_ARGS__)

#endif /* PRINTING_H */
