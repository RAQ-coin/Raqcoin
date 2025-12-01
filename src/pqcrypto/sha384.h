#ifndef RAQCOIN_PQCRYPTO_SHA384_H
#define RAQCOIN_PQCRYPTO_SHA384_H


struct Sha384 {
    ulong64  length, state[8];
    unsigned long curlen;
    unsigned char buf[128];
};


int sha384Init(Sha384 * md);
int sha384Process(Sha384 * md, const unsigned char *in, unsigned long inlen);
int sha384Done(Sha384 * md, unsigned char *hash);
int pqcSha384(const unsigned char *in, unsigned long inlen, unsigned char *hash);



#endif

