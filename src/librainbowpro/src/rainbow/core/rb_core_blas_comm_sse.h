#ifndef _BLAS_COMM_SSE_H_
#define _BLAS_COMM_SSE_H_
#include "rb_core_config.h"

#ifdef RB_SIMD_X86
#include "stdint.h"


#ifdef  __cplusplus
extern  "C" {
#endif

void gf16mat_prod_add_multab_sse(unsigned long handle, uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * b_multab );
void gf16mat_prod_multab_sse(unsigned long handle, uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * b_multab );
void gf16mat_prod_sse(unsigned long handle, uint8_t * c , const uint8_t * mat_a , unsigned a_h_byte , unsigned a_w , const uint8_t * b );
unsigned gf16mat_gauss_elim_sse(unsigned long handle, uint8_t * mat , unsigned h , unsigned w );

void gf256mat_prod_multab_sse(unsigned long handle, uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * multab );
void gf256mat_prod_sse(unsigned long handle, uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * b );
unsigned gf256mat_gauss_elim_sse(unsigned long handle, uint8_t * mat , unsigned h , unsigned w );
unsigned int rb_gf16mat_gauss_elim_8x16_sse(unsigned long handle, uint8_t* mat);


unsigned int rb_gf256mat_gauss_elim_sse(unsigned long handle, uint8_t* mat, unsigned int h, unsigned int w);
#ifdef  __cplusplus
}
#endif

#endif

#endif //  _BLAS_COMM_SSE_H_

