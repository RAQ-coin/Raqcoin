#include "rb_core_config.h"
#include "rb_core_rainbow_handle.h"
#include "rb_core_blas_comm.h"
#include "rb_core_blas.h"
#include "rb_core_blas_matrix_ref.h"

#ifdef RB_SIMD_X86
#include "rb_core_blas_comm_sse.h"
#endif

#include <stdint.h>
#include <string.h>


void rb_gf16mat_prod_ref(unsigned long handle, uint8_t *c, const uint8_t *matA, unsigned int n_A_vec_byte, unsigned int n_A_width, const uint8_t *b) 
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    rb_gf256v_set_zero(handle, c, n_A_vec_byte);
    for (unsigned int i = 0; i < n_A_width; i++) 
    {
        uint8_t bb = HD->gfv_get_ele(b, i);
        HD->gfv_madd(c, matA, bb, n_A_vec_byte);
        matA += n_A_vec_byte;
    }
}

void rb_gf256mat_prod_ref(unsigned long handle, uint8_t *c, const uint8_t *matA, unsigned int n_A_vec_byte, unsigned int n_A_width, const uint8_t *b) 
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    rb_gf256v_set_zero(handle, c, n_A_vec_byte);
    for (unsigned int i = 0; i < n_A_width; i++) {
        HD->gfv_madd(c, matA, b[i], n_A_vec_byte);
        matA += n_A_vec_byte;
    }
}




unsigned int rb_gf16mat_gauss_elim_16x32_ref(unsigned long handle, uint8_t* mat) 
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    const unsigned int h = 16;
    const unsigned int w = 32;
    const unsigned int w_byte = w / 2;

    unsigned int r8 = 1;
    for (unsigned int i = 0; i < h; i++) {
        unsigned int i_start = ((i >> 1) & (~(rb__BLAS_UNIT_LEN_ - 1)));
        uint8_t* ai = mat + i * w_byte;
        for (unsigned int j = i + 1; j < h; j++) {
            uint8_t* aj = mat + j * w_byte;
            HD->gf256v_conditional_add(ai + i_start, !rb_gf16_is_nonzero(HD->gfv_get_ele(ai, i)), aj + i_start, w_byte - i_start);
        }
        uint8_t pivot = HD->gfv_get_ele(ai, i);
        r8 &= rb_gf16_is_nonzero(pivot);
        pivot = rb_gf16_inv(pivot);
        HD->gfv_mul_scalar(ai + i_start, pivot, w_byte - i_start);
        for (unsigned int j = 0; j < h; j++) {
            if (i == j) continue;
            uint8_t* aj = mat + j * w_byte;
            HD->gfv_madd(aj + i_start, ai + i_start, HD->gfv_get_ele(aj, i), w_byte - i_start);
        }
    }
    return r8;
}
unsigned int rb_gf16mat_gauss_elim_8x16_ref(unsigned long handle, uint8_t* mat) 
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    const unsigned int h = 8;
    const unsigned int w = 16;
    const unsigned int w_byte = w / 2;

    unsigned int r8 = 1;
    for (unsigned int i = 0; i < h; i++) {
        unsigned int i_start = ((i >> 1) & (~(rb__BLAS_UNIT_LEN_ - 1)));
        uint8_t* ai = mat + i * w_byte;
        for (unsigned int j = i + 1; j < h; j++) {
            uint8_t* aj = mat + j * w_byte;
            HD->gf256v_conditional_add(ai + i_start, !rb_gf16_is_nonzero(HD->gfv_get_ele(ai, i)), aj + i_start, w_byte - i_start);
        }
        uint8_t pivot = HD->gfv_get_ele(ai, i);
        r8 &= rb_gf16_is_nonzero(pivot);
        pivot = rb_gf16_inv(pivot);
        HD->gfv_mul_scalar(ai + i_start, pivot, w_byte - i_start);
        for (unsigned int j = 0; j < h; j++) {
            if (i == j) continue;
            uint8_t* aj = mat + j * w_byte;
            HD->gfv_madd(aj + i_start, ai + i_start, HD->gfv_get_ele(aj, i), w_byte - i_start);
        }
    }
    return r8;
}



unsigned int rb_gf16mat_gauss_elim_ref(unsigned long handle, uint8_t *mat, unsigned int h, unsigned int w)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    const unsigned int w_byte = (w+1)>>1;

    unsigned int r8 = 1;
    for (unsigned int i = 0; i < h; i++) {
        //unsigned i_start = ((i>>1)&(~(rb__BLAS_UNIT_LEN_-1)));
        unsigned int i_start = (i>>1);
        uint8_t *ai = mat + i*w_byte;
        for (unsigned int j = i + 1; j < h; j++) {
            uint8_t *aj = mat + j*w_byte;
            HD->gf256v_conditional_add(ai + i_start, !rb_gf16_is_nonzero(HD->gfv_get_ele(ai, i)), aj + i_start, w_byte - i_start );
        }
        uint8_t pivot = HD->gfv_get_ele(ai, i);
        r8 &= rb_gf16_is_nonzero(pivot);
        pivot = rb_gf16_inv(pivot);
        HD->gfv_mul_scalar(ai + i_start, pivot, w_byte - i_start );
        for (unsigned int j = 0; j < h; j++) {
            if (i == j) continue;
            uint8_t *aj = mat + j*w_byte;
            HD->gfv_madd(aj + i_start, ai + i_start, HD->gfv_get_ele(aj, i), w_byte-i_start);
        }
    }
    return r8;
}

unsigned int rb_gf256mat_gauss_elim_ref(unsigned long handle, uint8_t * mat , unsigned int h , unsigned int w )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int r8 = 1;

    for(unsigned int i=0;i<h;i++) {
        uint8_t * ai = mat + w*i;
        //unsigned i_start = i-(i&(rb__BLAS_UNIT_LEN_-1));
        unsigned int i_start = i;

        for(unsigned int j=i+1;j<h;j++) {
            uint8_t * aj = mat + w*j;
            HD->gf256v_conditional_add( ai + i_start , !rb_gf256_is_nonzero(ai[i]) , aj + i_start , w - i_start );
        }
        r8 &= rb_gf256_is_nonzero(ai[i]);
        uint8_t pivot = ai[i];
        pivot = rb_gf256_inv( pivot );
        HD->gfv_mul_scalar( ai + i_start  , pivot , w - i_start );
        for(unsigned int j=0;j<h;j++) {
            if(i==j) continue;
            uint8_t * aj = mat + w*j;
            HD->gfv_madd( aj + i_start , ai+ i_start , aj[i] , w - i_start );
        }
    }

    return r8;
}


unsigned int rb_gf16mat_solve_linear_eq_48x48_ref(unsigned long handle, uint8_t* sol, const uint8_t* inp_mat, const uint8_t* c_terms)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

#define vec_len00 (24+rb__BLAS_UNIT_LEN_)
    uint8_t mat[48 * vec_len00];

    const unsigned int n = 48;
    const unsigned int n_2 = n / 2;
    for (unsigned int i = 0; i < n; i++) 
    {
        uint8_t* mi = mat + i * vec_len00;
        for (unsigned int j = 0; j < n; j++) 
            rb_gf16v_set_ele(mi, j, HD->gfv_get_ele(inp_mat + j * 24, i));

        mi[n_2] = HD->gfv_get_ele(c_terms, i);
    }
    uint8_t r8 = (uint8_t)rb_gf16mat_gauss_elim_ref(handle, mat, n, vec_len00 * 2);
    for (unsigned int i = 0; i < n; i++) 
        rb_gf16v_set_ele(sol, i, mat[i * vec_len00 + n_2]);

    return r8;
}


void rb_gf16mat_submat(unsigned long handle, uint8_t *mat2, unsigned int w2, unsigned int st, const uint8_t *mat, unsigned int w, unsigned int h)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int n_byte_w1 = (w + 1) / 2;
    unsigned int n_byte_w2 = (w2 + 1) / 2;
    unsigned int st_2 = st / 2;
    for (unsigned int i = 0; i < h; i++) {
        for (unsigned int j = 0; j < n_byte_w2; j++) mat2[i * n_byte_w2 + j] = mat[i * n_byte_w1 + st_2 + j];
    }
}

unsigned int rb_gf16mat_inv_8x8_ref(unsigned long handle, uint8_t* inv_a, const uint8_t* a)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
#define  H8 8
    uint8_t mat[H8 * H8];
    for (unsigned int i = 0; i < H8; i++)
    {
        uint8_t* ai = mat + i * H8;
        rb_gf256v_set_zero(handle, ai, H8);
        HD->gf256v_add(ai, a + i * H8 / 2, H8 / 2);
        rb_gf16v_set_ele(ai + H8 / 2, i, 1);
    }
    uint8_t r8 = (uint8_t)HD->gf16mat_gauss_elim_8x16_impl(handle, mat);
    rb_gf16mat_submat(handle, inv_a, H8, H8, mat, 2 * H8, H8);
    return r8;
}


void rb_gf256mat_submat(uint8_t * mat2 , unsigned int w2 , unsigned int st , const uint8_t * mat , unsigned int w , unsigned int h )
{
   
    for(unsigned int i=0;i<h;i++) {
        for(unsigned int j=0;j<w2;j++) mat2[i*w2+j] = mat[i*w+st+j];
    }
}


unsigned int rb_gf256mat_inv_32x32_ref(unsigned long handle, uint8_t * inv_a , const uint8_t * a )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    #define N32 32
    uint8_t mat[N32 * N32 *2];

    for(unsigned int i=0;i< N32;i++) {
        uint8_t * ai = mat + i*2* N32;
        rb_gf256v_set_zero(handle, ai , 2* N32);
        HD->gf256v_add( ai , a + i* N32, N32);
        ai[N32 +i] = 1;
    }
    unsigned char r8 = (uint8_t)HD->gf256mat_gauss_elim_impl(handle, mat , N32, 2* N32);
    rb_gf256mat_submat( inv_a , N32, N32, mat , 2* N32, N32);
    rb_gf256v_set_zero(handle, mat, N32 *2* N32);

    return r8;
}


unsigned int rb_gf256mat_solve_linear_eq_64x64_ref(unsigned long handle, uint8_t * sol , const uint8_t * inp_mat , const uint8_t * c_terms )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    #define N64 64
    #define Vec_Len_N64 (N64 + rb__BLAS_UNIT_LEN_)
    uint8_t mat[N64 * Vec_Len_N64];  // no need to clean to zero

    for(unsigned int i=0;i< N64;i++)
    {
        uint8_t * mi = mat + i* Vec_Len_N64;
        for(unsigned int j=0;j< N64;j++)
            mi[j] = inp_mat[j* N64 +i];

        mi[N64] = c_terms[i];
    }
    unsigned int r8 = HD->gf256mat_gauss_elim_impl(handle, mat , N64, Vec_Len_N64);
    for(unsigned int i=0;i< N64;i++) sol[i] = mat[i* Vec_Len_N64 + N64];
    rb_gf256v_set_zero(handle, mat, N64 * Vec_Len_N64); // clean

    return r8;
}

unsigned int rb_gf256mat_solve_linear_eq_96x96_ref(unsigned long handle, uint8_t* sol, const uint8_t* inp_mat, const uint8_t* c_terms)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

#define N96 96
#define Vec_Len_N96 (N96 + rb__BLAS_UNIT_LEN_)
    uint8_t mat[N96 * Vec_Len_N96];  // no need to clean to zero

    for (unsigned int i = 0; i < N96; i++)
    {
        uint8_t* mi = mat + i * Vec_Len_N96;
        for (unsigned int j = 0; j < N96; j++)
            mi[j] =
            inp_mat[j * N96 + i];

        mi[N96] = c_terms[i];
    }
    unsigned int r8 = HD->gf256mat_gauss_elim_impl(handle, mat, N96, Vec_Len_N96);
    for (unsigned int i = 0; i < N96; i++) sol[i] = mat[i * Vec_Len_N96 + N96];
    rb_gf256v_set_zero(handle, mat, N96 * Vec_Len_N96); // clean

    return r8;
}

unsigned int rb_gf256mat_inv_8x8_ref(unsigned long handle, uint8_t* inv_a, const uint8_t* a)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
#define H8 8
    uint8_t mat[H8 * H8 * 2];

    for (unsigned int i = 0; i < H8; i++) {
        uint8_t* ai = mat + i * 2 * H8;
        rb_gf256v_set_zero(handle, ai, 2 * H8);
        HD->gf256v_add(ai, a + i * H8, H8);
        ai[H8 + i] = 1;
    }
    unsigned char r8 = (uint8_t)HD->gf256mat_gauss_elim_impl(handle, mat, H8, 2 * H8);
    rb_gf256mat_submat(inv_a, H8, H8, mat, 2 * H8, H8);
    rb_gf256v_set_zero(handle, mat, H8 * 2 * H8);

    return r8;
}

unsigned int rb_gf256mat_inv_36x36_ref(unsigned long handle, uint8_t * inv_a , const uint8_t * a )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    #define H36 36
    uint8_t mat[H36 * H36 *2];

    for(unsigned int i=0;i< H36;i++) {
        uint8_t * ai = mat + i*2* H36;
        rb_gf256v_set_zero(handle, ai , 2* H36);
        HD->gf256v_add( ai , a + i* H36, H36);
        ai[H36 +i] = 1;
    }
    unsigned char r8 = (uint8_t)HD->gf256mat_gauss_elim_impl(handle, mat , H36, 2* H36);
    rb_gf256mat_submat(inv_a , H36, H36, mat , 2* H36, H36);
    rb_gf256v_set_zero(handle, mat, H36 *2* H36);

    return r8;
}
unsigned int rb_gf256mat_inv_44x44_ref(unsigned long handle, uint8_t* inv_a, const uint8_t* a)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
#define H44 44
    uint8_t mat[H44 * H44 * 2];

    for (unsigned int i = 0; i < H44; i++) {
        uint8_t* ai = mat + i * 2 * H44;
        rb_gf256v_set_zero(handle, ai, 2 * H44);
        HD->gf256v_add(ai, a + i * H44, H44);
        ai[H44 + i] = 1;
    }
    unsigned char r8 = (uint8_t)HD->gf256mat_gauss_elim_impl(handle, mat, H44, 2 * H44);
    rb_gf256mat_submat(inv_a, H44, H44, mat, 2 * H44, H44);
    rb_gf256v_set_zero(handle, mat, H44 * 2 * H44);

    return r8;
}
unsigned int rb_gf256mat_inv_72x72_ref(unsigned long handle, uint8_t* inv_a, const uint8_t* a)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
#define H72 72
    uint8_t mat[H72 * H72 * 2];

    for (unsigned int i = 0; i < H72; i++) {
        uint8_t* ai = mat + i * 2 * H72;
        rb_gf256v_set_zero(handle, ai, 2 * H72);
        HD->gf256v_add(ai, a + i * H72, H72);
        ai[H72 + i] = 1;
    }
    unsigned char r8 = (uint8_t)HD->gf256mat_gauss_elim_impl(handle, mat, H72, 2 * H72);
    rb_gf256mat_submat( inv_a, H72, H72, mat, 2 * H72, H72);
    rb_gf256v_set_zero(handle, mat, H72 * 2 * H72);

    return r8;
}

unsigned int rb_gf256mat_inv_96x96_ref(unsigned long handle, uint8_t* inv_a, const uint8_t* a)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
#define H96 96
    uint8_t mat[H96 * H96 * 2];

    for (unsigned int i = 0; i < H96; i++) {
        uint8_t* ai = mat + i * 2 * H96;
        rb_gf256v_set_zero(handle, ai, 2 * H96);
        HD->gf256v_add(ai, a + i * H96, H96);
        ai[H96 + i] = 1;
    }
    unsigned char r8 = (uint8_t)HD->gf256mat_gauss_elim_impl(handle,mat, H96, 2 * H96);
    rb_gf256mat_submat(inv_a, H96, H96, mat, 2 * H96, H96);
    rb_gf256v_set_zero(handle, mat, H96 * 2 * H96);

    return r8;
}


