#ifndef rb__RAINBOW_KEYPAIR_COMP_H_
#define rb__RAINBOW_KEYPAIR_COMP_H_

#include "rb_core_config.h"
#include "rb_core_rainbow_keypair.h"


#ifdef  __cplusplus
extern  "C" {
#endif

#include "rb_core_rng.h" 

void rb_calculate_F_from_Q(unsigned long handle, rb_sk_t * Fs , const rb_sk_t * Qs , const rb_sk_t * Ts);
void rb_calculate_Q_from_F_cyclic(unsigned long handle, rb_cpk_t * Qs, const rb_sk_t * Fs , const rb_sk_t * Ts);


#ifdef  __cplusplus
}
#endif

#endif

