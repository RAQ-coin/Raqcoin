#include "rb_core_random.h"

#include "rb_core_pqcrypto.h"
#include "rb_core_config.h"

void rb_getRandBytes(unsigned char *buf,int size)
{
    int bits = 128;
	rb_prng_state prng;
	if (rb_rng_make_prng(bits, &prng, NULL) != PQCRYPT_OK)
	{
        printf("rb_rng_make_prng error \n");
	}
	if (rb_fortuna_read(buf,size,&prng) != (unsigned int)size)
	{
		printf("fortuna_read error \n");
	}

}

