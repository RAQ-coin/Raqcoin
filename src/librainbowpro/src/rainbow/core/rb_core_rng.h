#ifndef rb_rng_h
#define rb_rng_h

#include <stdio.h>
#include "rb_core_rainbow_handle.h"
#include "rb_core_config.h"
#define RNG_SUCCESS      0
#define RNG_BAD_MAXLEN  -1
#define RNG_BAD_OUTBUF  -2
#define RNG_BAD_REQ_LEN -3

void rb_AES256_CTR_DRBG_Update(unsigned char *provided_data, unsigned char *Key,unsigned char *V);
void rb_randombytes_init(unsigned long handle, unsigned char *entropy_input,unsigned char *personalization_string);
int rb_randombytes(unsigned long handle, unsigned char *x, unsigned int xlen);
unsigned int rb_getRngBytes(unsigned char* out, unsigned int outlen, void (*callback)(void));
extern unsigned int (*rb_pqc_rng)(unsigned char* out, unsigned int outlen,void (*callback)(void));

#endif 
