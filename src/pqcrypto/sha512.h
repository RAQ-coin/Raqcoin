#ifndef RAQCOIN_PQCRYPTO_SHA512_H
#define RAQCOIN_PQCRYPTO_SHA512_H


struct Sha512 {
    ulong64  length, state[8];
    unsigned long curlen;
    unsigned char buf[128];
};


int sha512Init(Sha512 * md);
int sha512Process(Sha512 * md, const unsigned char *in, unsigned long inlen);
int sha512Done(Sha512 * md, unsigned char *hash);
int pqcSha512(const unsigned char *in, unsigned long inlen, unsigned char *hash);



#endif

