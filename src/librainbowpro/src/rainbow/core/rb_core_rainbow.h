#ifndef rb__RAINBOW_H_
#define rb__RAINBOW_H_

#include "rb_core_config.h"
#include "rb_core_rainbow_keypair.h"
#include "rb_core_api.h"
#include <stdint.h>

#ifdef  __cplusplus
extern  "C" {
#endif


int rb_rainbow_sign(unsigned long handle, uint8_t * signature , rb_sk_t * sk , uint8_t * digest);
int rb_rainbow_sign_common(unsigned long handle, uint8_t* signature, rb_sk_t* sk, uint8_t* digest);
unsigned int rb_rainbow_sign_cyclic(unsigned long handle, uint8_t * signature , rb_csk_t * sk , uint8_t * digest);
unsigned int rb_rainbow_verify_cyclic(unsigned long handle, uint8_t * digest_hash ,  uint8_t * signature ,  rb_cpk_t * pk);

 
#ifdef  __cplusplus
}
#endif


#endif // rb__RAINBOW_H_
