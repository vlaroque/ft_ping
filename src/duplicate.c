#include "duplicate.h"

void seq_bit_ack_received(ping_env_t *env, uint16_t seq)
{
	env->seq_bit_map[BYTE_IDX(seq)] |= (1 << BIT_IDX(seq));
}

bool seq_bit_is_duplicate(ping_env_t *env, uint16_t seq)
{
	return((env->seq_bit_map[BYTE_IDX(seq)] & (1 << BIT_IDX(seq))) != 0);
}

void seq_bit_clear_ack_received(ping_env_t *env, uint16_t seq)
{
	if ( !BIT_IDX(seq) )
	{
		uint16_t opposite_seq = seq ^ 0x8000;

		env->seq_bit_map[BYTE_IDX(opposite_seq)] = 0;
	}
}
