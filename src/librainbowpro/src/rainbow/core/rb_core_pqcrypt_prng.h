#ifndef rb_PQC_PQCRYPT_PRNG_H
#define rb_PQC_PQCRYPT_PRNG_H

#include "rb_core_config.h"
#include "rb_core_sha256_pqc.h"
/* ---- PRNG Stuff ---- */

#define rb_PQC_FORTUNA_POOLS 32
#define rb_PQC_MUTEX_TYPE(x)

struct rb_fortuna_prng {
    rb_Sha256 pool[rb_PQC_FORTUNA_POOLS];     /* the  pools */

    rb_symmetric_key skey;

    unsigned char K[32],     
                  IV[16];    

    unsigned int pool_idx,   
                  pool0_len, 
                  wd;

    ulong64       reset_cnt; 
    rb_PQC_MUTEX_TYPE(prng_lock)
};

typedef union rb_Prng_state {
    char dummy[1];
    struct rb_fortuna_prng   fortuna;

} rb_prng_state;

int rb_fortuna_start(rb_prng_state *prng);
int rb_fortuna_add_entropy(const unsigned char *in, unsigned int inlen, rb_prng_state *prng);
int rb_fortuna_ready(rb_prng_state *prng);
unsigned int rb_fortuna_read(unsigned char *out, unsigned int outlen, rb_prng_state *prng);
int rb_fortuna_done(rb_prng_state *prng);
int  rb_fortuna_export(unsigned char *out, unsigned int*outlen, rb_prng_state *prng);
int  rb_fortuna_import(const unsigned char *in, unsigned int inlen, rb_prng_state *prng);
int rb_rng_make_prng(int bits, rb_prng_state *prng, void (*callback)(void));



#endif

