#ifndef RAQCOIN_RANDOM_H
#define RAQCOIN_RANDOM_H


void getRandBytes(unsigned char* buf, int size);
int getRandInt();
int  getRandHash(unsigned char *hash);
int getRand(int nMax);
unsigned long random_uint32_t();
unsigned long long random_uint64_t();



#endif
