#ifndef rb__BLAS_MATRIX_REF_H_
#define rb__BLAS_MATRIX_REF_H_

#include <stdint.h>
#include "rb_core_config.h"


#ifdef  __cplusplus
extern  "C" {
#endif


void rb_gf16mat_prod_ref(unsigned long handle, uint8_t *c, const uint8_t *matA, unsigned int n_A_vec_byte, unsigned int n_A_width, const uint8_t *b);
void rb_gf256mat_prod_ref(unsigned long handle, uint8_t *c, const uint8_t *matA, unsigned int n_A_vec_byte, unsigned int n_A_width, const uint8_t *b);
unsigned int rb_gf16mat_inv_8x8_ref  (unsigned long handle, uint8_t* inv_a, const uint8_t* a);
unsigned int rb_gf16mat_solve_linear_eq_48x48_ref(unsigned long handle, uint8_t* sol, const uint8_t* inp_mat, const uint8_t* c_terms);
unsigned int rb_gf256mat_inv_8x8_ref  (unsigned long handle, uint8_t* inv_a, const uint8_t* a);
unsigned int rb_gf256mat_inv_32x32_ref(unsigned long handle, uint8_t* inv_a, const uint8_t* a);
unsigned int rb_gf256mat_inv_36x36_ref(unsigned long handle, uint8_t* inv_a, const uint8_t* a);
unsigned int rb_gf256mat_inv_44x44_ref(unsigned long handle, uint8_t* inv_a, const uint8_t* a);
unsigned int rb_gf256mat_inv_72x72_ref(unsigned long handle, uint8_t* inv_a, const uint8_t* a);
unsigned int rb_gf256mat_inv_96x96_ref(unsigned long handle, uint8_t* inv_a, const uint8_t* a);
unsigned int rb_gf256mat_solve_linear_eq_64x64_ref(unsigned long handle, uint8_t *sol, const uint8_t *inp_mat, const uint8_t *c_terms );
unsigned int rb_gf256mat_solve_linear_eq_96x96_ref(unsigned long handle, uint8_t* sol, const uint8_t* inp_mat, const uint8_t* c_terms );
unsigned int rb_gf16mat_gauss_elim_8x16_ref(unsigned long handle, uint8_t* mat);
unsigned int rb_gf16mat_gauss_elim_16x32_ref(unsigned long handle, uint8_t* mat);



#ifdef  __cplusplus
}
#endif

#endif  // rb__BLAS_MATRIX_REF_H_

