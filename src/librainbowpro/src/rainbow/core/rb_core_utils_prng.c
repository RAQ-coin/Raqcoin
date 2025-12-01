#include <stdlib.h>
#include <string.h>
#include "rb_core_utils_prng.h"
#include "rb_core_utils_hash.h"

int rb_prng_set(rb_prng_t *ctx, const unsigned char *prng_seed, unsigned int prng_seed_len, unsigned int P_HASH_LEN)
{

    memset(ctx->buf, 0, P_HASH_LEN);
    ctx->used = 0;

    rb_hash_msg(ctx->buf, P_HASH_LEN, (const unsigned char *)prng_seed, prng_seed_len, P_HASH_LEN);

    return 0;
}

int rb_prng_gen(rb_prng_t *ctx, unsigned char *out, unsigned int outlen, unsigned int P_HASH_LEN)
{

   while( outlen ) 
   {
      if(P_HASH_LEN == ctx->used ) 
      { 
          rb_hash_msg(ctx->buf, P_HASH_LEN, ctx->buf, P_HASH_LEN, P_HASH_LEN);
          ctx->used = 0; 
      }
      out[0] = ctx->buf[ctx->used];
      out++;
      ctx->used++;
      outlen--;
   }
   return 0;
}


