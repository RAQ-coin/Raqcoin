#ifndef rb__BLAS_MATRIX_H_
#define rb__BLAS_MATRIX_H_

#include <stdint.h>
#include "rb_core_config.h"


#ifdef  __cplusplus
extern  "C" {
#endif


void rb_gf16mat_prod(unsigned long handle, unsigned char*c, const unsigned char*matA, unsigned int n_A_vec_byte, unsigned int n_A_width, const unsigned char*b);
void rb_gf256mat_prod(unsigned long handle, unsigned char*c, const unsigned char*matA, unsigned int n_A_vec_byte, unsigned int n_A_width, const unsigned char*b);
unsigned int rb_gf256mat_solve_linear_eq_64x64(unsigned long handle, unsigned char*sol, const unsigned char*inp_mat, const unsigned char*c_terms );
unsigned int rb_gf256mat_solve_linear_eq_96x96(unsigned long handle, unsigned char* sol, const unsigned char* inp_mat, const unsigned char* c_terms);
unsigned int rb_gf256mat_inv_8x8(unsigned long handle, unsigned char* inv_a, const unsigned char* a);
unsigned int rb_gf256mat_inv_32x32(unsigned long handle, unsigned char* inv_a, const unsigned char* a);
unsigned int rb_gf256mat_inv_36x36(unsigned long handle, unsigned char*inv_a, const unsigned char*a );
unsigned int rb_gf256mat_inv_44x44(unsigned long handle, unsigned char* inv_a, const unsigned char* a);
unsigned int rb_gf256mat_inv_72x72(unsigned long handle, unsigned char* inv_a, const unsigned char* a);
unsigned int rb_gf256mat_inv_96x96(unsigned long handle, unsigned char* inv_a, const unsigned char* a);
unsigned int rb_gf16mat_solve_linear_eq_48x48(unsigned long handle, unsigned char* sol, const unsigned char* inp_mat, const unsigned char* c_terms);
unsigned int rb_gf16mat_inv_8x8(unsigned long handle, unsigned char* inv_a, const unsigned char* a);


#ifdef  __cplusplus
}
#endif

#endif  // rb__BLAS_MATRIX_H_

