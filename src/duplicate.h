#ifndef DUPLICATE_H
#define DUPLICATE_H

#include <stdint.h>
#include <stdbool.h>

#include "ft_ping.h"

/*
 * my way: bit = 1 if seq received
 *         bit = 0 not received
 */

#define BYTE_IDX(seq) (seq >> 3)   /* divide by 8 */
#define BIT_IDX(seq)  (seq & 0x07) /* modulo 8 */

void seq_bit_ack_received(ping_env_t *env, uint16_t seq);
void seq_bit_clear_ack_received(ping_env_t *env, uint16_t seq);
bool seq_bit_is_duplicate(ping_env_t *env, uint16_t seq);

#endif /* DUPLICATE_H */
