#ifndef RAQCOIN_PQCRYPT_PRNG_H
#define RAQCOIN_PQCRYPT_PRNG_H


/* ---- PRNG Stuff ---- */

struct fortuna_prng {
    Sha256 pool[PQC_FORTUNA_POOLS];     /* the  pools */

    symmetric_key skey;

    unsigned char K[32],      /* the current key */
                  IV[16];     /* IV for CTR mode */

    unsigned long pool_idx,   /* current pool we will add to */
                  pool0_len,  /* length of 0'th pool */
                  wd;

    ulong64       reset_cnt;  /* number of times we have reset */
    PQC_MUTEX_TYPE(prng_lock)
};

typedef union Prng_state {
    char dummy[1];
    struct fortuna_prng   fortuna;

} prng_state;

int fortuna_start(prng_state *prng);
int fortuna_add_entropy(const unsigned char *in, unsigned long inlen, prng_state *prng);
int fortuna_ready(prng_state *prng);
unsigned long fortuna_read(unsigned char *out, unsigned long outlen, prng_state *prng);
int fortuna_done(prng_state *prng);
int  fortuna_export(unsigned char *out, unsigned long *outlen, prng_state *prng);
int  fortuna_import(const unsigned char *in, unsigned long inlen, prng_state *prng);
//int  fortuna_test(void);

int rng_make_prng(int bits, prng_state *prng, void (*callback)(void));



#endif


/* $Source$ */
/* $Revision$ */
/* $Date$ */

