#ifndef rb_api_h
#define rb_api_h

#include "rb_core_config.h"
#include "rb_core_RAINBOW_PRO_API.h"
#include "rb_core_rainbow_handle.h"
#include "rb_core_rainbow_keypair.h"
#include "rb_core_blas_matrix.h"


#ifdef  __cplusplus
extern  "C" {
#endif


int rb_crypto_generate_keypair(unsigned long handle, unsigned char *pk, unsigned char *sk);
int rb_crypto_sign(unsigned long handle, unsigned char* sm, unsigned int* smlen, unsigned char* digest_hash, unsigned int digest_hash_len, unsigned char* sk);

#ifdef  __cplusplus
}
#endif

#endif /* rb_api_h */
