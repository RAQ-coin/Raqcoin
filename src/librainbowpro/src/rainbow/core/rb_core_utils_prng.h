#ifndef _UTILS_PRNG_H_
#define _UTILS_PRNG_H_

#include "rb_core_config.h"
#ifdef  __cplusplus
extern  "C" {
#endif




typedef struct rb_prng_context
{ 
    unsigned char* buf;
    unsigned int used;
} rb_prng_t;


int rb_prng_set(rb_prng_t *ctx, const unsigned char* prng_seed, unsigned int prng_seedlen, unsigned int P_HASH_LEN);

int rb_prng_gen(rb_prng_t *ctx, unsigned char *out, unsigned int outlen, unsigned int P_HASH_LEN);




#ifdef  __cplusplus
}
#endif



#endif // _UTILS_PRNG_H_


