#include "random.h"

#include "pqcrypto.h"


void getRandBytes(unsigned char *buf,int size) {
    int bits = 128;
	prng_state prng;
	if (rng_make_prng(bits, &prng, NULL) != PQCRYPT_OK) {
        printf("rng_make_prng error \n");
	}
	if (fortuna_read(buf,size,&prng) != (unsigned long)size) {
		printf("fortuna_read error \n");
	}

}

int getRandInt() {
	int size = 4, i = 0;
	unsigned char *buf = NULL, mask;
	buf  = (unsigned char *)XMALLOC(size);
	getRandBytes(buf,size);

	mask = static_cast<unsigned char>(0xff << (8 - 128 % 8));
    buf[0] &= ~mask;


    int  res = 0;
	for (i = 0; i < size; i++) {
        res |= (((int)buf[i])<<(i*8));
	}
	XFREE(buf);	
    return res ;
}

int getRandHash(unsigned char* hash) {
    getRandBytes(hash, sizeof(hash));
    return 0;
}

int getRand(int nMax) {
    return (getRandInt()% nMax);
}

unsigned long random_uint32_t() {
	int bits = 128,  size = 4, i = 0;
	prng_state prng;
    if (rng_make_prng(bits, &prng, NULL) != PQCRYPT_OK) {
        printf("rng_make_prng error \n");
	}
	unsigned char *buf, mask;
	buf  = (unsigned char *)XMALLOC(size);
	if (fortuna_read(buf,size,&prng) != (unsigned long)size) {
		printf("fortuna_read error \n");
	}

	mask = 0xff << (8 - bits % 8);
    buf[0] &= ~mask;

    unsigned long  res = 0;

	for (i = 0; i < size; i++) {
        res |= (((unsigned long)buf[i])<<(i*8));
	}
	XFREE(buf);	
    return res;
}


unsigned long long  random_uint64_t() {
	int bits = 128,  size = 8, i = 0;
	prng_state prng;
    if (rng_make_prng(bits, &prng, NULL) != PQCRYPT_OK) {
        printf("rng_make_prng error \n");
	}
	unsigned char *buf, mask;
	buf  = (unsigned char *)XMALLOC(size);
	if (fortuna_read(buf,size,&prng) != (unsigned long)size) {
		printf("fortuna_read error \n");
	}

	mask = 0xff << (8 - bits % 8);
    buf[0] &= ~mask;

    unsigned long long   res = 0;

	for (i = 0; i < size; i++) {
        res |= (((unsigned long long )buf[i])<<(i*8));
	}
	XFREE(buf);	
    return res;
}

