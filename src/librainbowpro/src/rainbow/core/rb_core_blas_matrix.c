#include "rb_core_rainbow_handle.h"
#include "rb_core_blas_comm.h"
#include "rb_core_blas_matrix.h"
#include "rb_core_blas.h"
#include "rb_core_blas_matrix_ref.h"



void rb_gf16mat_prod(unsigned long handle, unsigned char*c, const unsigned char *matA, unsigned int n_A_vec_byte, unsigned int n_A_width, const unsigned char*b)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    HD->gf16mat_prod_impl(handle, c, matA, n_A_vec_byte, n_A_width, b);
}


void rb_gf256mat_prod(unsigned long handle, unsigned char*c, const unsigned char*matA, unsigned int n_A_vec_byte, unsigned int n_A_width, const unsigned char*b)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    HD->gf256mat_prod_impl(handle, c, matA, n_A_vec_byte, n_A_width, b);
}



unsigned int rb_gf16mat_solve_linear_eq_48x48(unsigned long handle, unsigned char* sol, const unsigned char* inp_mat, const unsigned char* c_terms)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    return HD->gf16mat_solve_linear_eq_48x48_impl(handle, sol, inp_mat, c_terms);
}


unsigned int rb_gf16mat_inv_8x8(unsigned long handle, unsigned char* inv_a, const unsigned char* a)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    return HD->gf16mat_inv_8x8_impl(handle,inv_a, a);
}


unsigned int rb_gf256mat_inv_32x32(unsigned long handle, unsigned char* inv_a , const unsigned char* a )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    return HD->gf256mat_inv_32x32_impl(handle, inv_a , a );
}



unsigned int rb_gf256mat_solve_linear_eq_64x64(unsigned long handle, unsigned char* sol , const unsigned char* inp_mat , const unsigned char* c_terms )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    return HD->gf256mat_solve_linear_eq_64x64_impl(handle, sol , inp_mat , c_terms );
}
unsigned int rb_gf256mat_solve_linear_eq_96x96(unsigned long handle, unsigned char* sol, const unsigned char* inp_mat, const unsigned char* c_terms)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    return HD->gf256mat_solve_linear_eq_96x96_impl(handle, sol, inp_mat, c_terms);
}

unsigned int rb_gf256mat_inv_8x8(unsigned long handle, unsigned char* inv_a, const unsigned char* a)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    return HD->gf256mat_inv_8x8_impl(handle, inv_a, a);
}

unsigned int rb_gf256mat_inv_36x36(unsigned long handle, unsigned char* inv_a , const unsigned char* a )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    return HD->gf256mat_inv_36x36_impl(handle, inv_a , a );
}
unsigned int rb_gf256mat_inv_44x44(unsigned long handle, unsigned char* inv_a, const unsigned char* a)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    return HD->gf256mat_inv_44x44_impl(handle, inv_a, a);
}
unsigned int rb_gf256mat_inv_72x72(unsigned long handle, unsigned char* inv_a, const unsigned char* a)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    return HD->gf256mat_inv_72x72_impl(handle, inv_a, a);
}

unsigned int rb_gf256mat_inv_96x96(unsigned long handle, unsigned char* inv_a, const unsigned char* a)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    return HD->gf256mat_inv_96x96_impl(handle, inv_a, a);
}
