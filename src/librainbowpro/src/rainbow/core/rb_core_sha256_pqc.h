#ifndef rb_PQC_PQCRYPTO_SHA256_H
#define rb_PQC_PQCRYPTO_SHA256_H

#include "rb_core_config.h"
typedef struct  {
    ulong64 length;
    ulong32 state[8], curlen;
    unsigned char buf[64];
}rb_Sha256;

int rb_sha256Init(rb_Sha256 * md);
int rb_sha256Process(rb_Sha256 * md, const unsigned char *in, unsigned int inlen);
int rb_sha256Done(rb_Sha256 * md, unsigned char *hash);
int rb_pqcSha256(const unsigned char *in, unsigned int inlen, unsigned char *out);


#endif
