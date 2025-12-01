#ifndef RAQCOIN_PQCRYPTO_SHA256_H
#define RAQCOIN_PQCRYPTO_SHA256_H


struct Sha256 {
    ulong64 length;
    ulong32 state[8], curlen;
    unsigned char buf[64];
};

int sha256Init(Sha256 * md);
int sha256Process(Sha256 * md, const unsigned char *in, unsigned long inlen);
int sha256Done(Sha256 * md, unsigned char *hash);
int pqcSha256(const unsigned char *in, unsigned long inlen, unsigned char *out);


#endif
