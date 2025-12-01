#ifndef RAINBOW_h
#define RAINBOW_h


#include "rainbow_config.h"
#include "rainbow_16.h"


//  Set these three values apropriately for your algorithm
#define CRYPTO_SECRETKEYBYTES _SEC_KEY_LEN
#define CRYPTO_PUBLICKEYBYTES _PUB_KEY_LEN
#define CRYPTO_BYTES _SIGNATURE_BYTE

// Change the algorithm name
#define CRYPTO_ALGNAME _S_NAME

int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

int
crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
            const unsigned char *sk);

int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk);


#endif /* api_h */
