#include "rb_core_api.h"
#include "rb_core_rainbow_keypair.h"
#include "rb_core_utils_prng.h"
#include "rb_core_blas.h"
#include "rb_core_parallel_matrix_op.h"
#include "rb_core_config.h"



void rb_accu_eval_quad(unsigned long handle, unsigned char * accu_res , unsigned char * trimat , unsigned char * x_in_byte , unsigned int num_gfele_x , unsigned int vec_len)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int cnt = 0;
    unsigned char* _x = x_in_byte;
    unsigned char *_xixj = (unsigned char *)rb_safe_malloc(HD->P_MAX_N);
  
    unsigned int n = num_gfele_x;



    for (unsigned int i = 0; i < n; i++) 
    {
#if defined( _BLAS_AVX2_ )
        unsigned int i_start = i - (i & 31);
#elif defined( _BLAS_SSE_ )
        unsigned int i_start = i - (i & 15);
#elif defined( _BLAS_UINT64_ )
        unsigned int i_start = i - (i & 7);
#else
        unsigned int i_start = i - (i & 3);
#endif
        for (unsigned int j = i; j < n; j++)
        {
            _xixj[j] = _x[j];
        }
        HD->gfv_mul_scalar(_xixj + i_start, _x[i], n - i_start);


        for (unsigned int j = i; j < n; j++) 
        {
            unsigned int idx = _xixj[j];
            if (idx) 
                HD->gf256v_add(accu_res + HD->P_TMPVEC_LEN * idx, trimat, vec_len);
          
           // if (idx)
            {
                trimat += vec_len;
                cnt++;
            }
        }
    }
    rb_safe_free(_xixj);


}
static void rb_accu_eval_quad_rect( unsigned long handle, unsigned char * accu_res , const unsigned char * y , unsigned int num_y , const unsigned char * mat , const unsigned char * x , unsigned int num_x , unsigned int vec_len)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned char *_xy =(unsigned char *)rb_safe_malloc(HD->P_MAX_N);
   

    for (unsigned int i = 0; i < num_y; i++) {
        for (unsigned int j = 0; j < num_x; j++) _xy[j] = x[j];
        HD->gfv_mul_scalar(_xy, y[i], num_x);
        for (unsigned int j = 0; j < num_x; j++) {
            unsigned int idx = _xy[j];
            if (idx) HD->gf256v_add(accu_res + HD->P_TMPVEC_LEN * idx, mat, vec_len);
            mat += vec_len;
        }
    }
    rb_safe_free(_xy);

}
static void rb_madd_reduce(unsigned long handle, unsigned char* y, const unsigned char* tmp_res, unsigned int vec_len)
{

    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned char *tmp = (unsigned char *)rb_safe_malloc(HD->P_TMPVEC_LEN);
  
    int accu_bit = 1;

    rb_gf256v_set_zero(handle, y, vec_len);
    //// x0



    // x1
    accu_bit = 1 << 0; // 4
    rb_gf256v_set_zero(handle, tmp, vec_len);
    for (unsigned int i = accu_bit; i < HD->P_GFSIZE; i += accu_bit * 2) {
        for (int j = 0; j < accu_bit; j++) HD->gf256v_add(tmp, tmp_res + HD->P_TMPVEC_LEN * (i + j), vec_len);
    }
    HD->gfv_madd(y, tmp, (uint8_t)accu_bit, vec_len);

    // x2
    accu_bit = 1 << 1; // 2
    rb_gf256v_set_zero(handle, tmp, vec_len);
    for (unsigned int i = accu_bit; i < HD->P_GFSIZE; i += accu_bit * 2) {
        for (int j = 0; j < accu_bit; j++) HD->gf256v_add(tmp, tmp_res + HD->P_TMPVEC_LEN * (i + j), vec_len);
    }
    HD->gfv_madd(y, tmp, (uint8_t)accu_bit, vec_len);

    // x4
    accu_bit = 1 << 2; // 4
    rb_gf256v_set_zero(handle, tmp, vec_len);
    for (unsigned int i = accu_bit; i < HD->P_GFSIZE; i += accu_bit * 2) {
        for (int j = 0; j < accu_bit; j++) HD->gf256v_add(tmp, tmp_res + HD->P_TMPVEC_LEN * (i + j), vec_len);
    }
    HD->gfv_madd(y, tmp, (uint8_t)accu_bit, vec_len);

    // x8
    accu_bit = 1 << 3; // 8
    rb_gf256v_set_zero(handle, tmp, vec_len);
    for (unsigned int i = accu_bit; i < HD->P_GFSIZE; i += accu_bit * 2) {
        for (int j = 0; j < accu_bit; j++) HD->gf256v_add(tmp, tmp_res + HD->P_TMPVEC_LEN * (i + j), vec_len);
    }
    HD->gfv_madd(y, tmp, (uint8_t)accu_bit, vec_len);

if (256 == HD->P_GFSIZE)
{
        accu_bit = 1 << 4; // 16
        rb_gf256v_set_zero(handle, tmp, vec_len);
    for (unsigned int i = accu_bit; i < HD->P_GFSIZE; i += accu_bit * 2) {
        for (int j = 0; j < accu_bit; j++) HD->gf256v_add(tmp, tmp_res + HD->P_TMPVEC_LEN * (i + j), vec_len);
    }
    HD->gfv_madd(y, tmp, (uint8_t)accu_bit, vec_len);

    accu_bit = 1 << 5; // 32
    rb_gf256v_set_zero(handle, tmp, vec_len);
    for (unsigned int i = accu_bit; i < HD->P_GFSIZE; i += accu_bit * 2) {
        for (int j = 0; j < accu_bit; j++) HD->gf256v_add(tmp, tmp_res + HD->P_TMPVEC_LEN * (i + j), vec_len);
    }
    HD->gfv_madd(y, tmp, (uint8_t)accu_bit, vec_len);

    accu_bit = 1 << 6; // 64
    rb_gf256v_set_zero(handle, tmp, vec_len);
    for (unsigned int i = accu_bit; i < HD->P_GFSIZE; i += accu_bit * 2) {
        for (int j = 0; j < accu_bit; j++) HD->gf256v_add(tmp, tmp_res + HD->P_TMPVEC_LEN * (i + j), vec_len);
    }
    HD->gfv_madd(y, tmp, (uint8_t)accu_bit, vec_len);

    accu_bit = 1 << 7; // 128
    rb_gf256v_set_zero(handle, tmp, vec_len);
    for (unsigned int i = accu_bit; i < HD->P_GFSIZE; i += accu_bit * 2) {
        for (int j = 0; j < accu_bit; j++) HD->gf256v_add(tmp, tmp_res + HD->P_TMPVEC_LEN * (i + j), vec_len);
    }
    HD->gfv_madd(y, tmp, (uint8_t)accu_bit, vec_len);

}
rb_safe_free(tmp);

}
void rb_rainbow_publicmap_cpk(unsigned long handle, unsigned char * z, const rb_cpk_t * pk, const unsigned char *w)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    rb_prng_t prng0;
    memset(&prng0, 0, sizeof(rb_prng_t));
    prng0.buf = (unsigned char*)rb_safe_malloc(HD->P_HASH_LEN);
  

    rb_prng_set(&prng0, pk->pk_seed, HD->P_LEN_PKSEED, HD->P_HASH_LEN);


    unsigned int BUF_SIZE_1 = 0;
    if (HD->P_O2 == 0)
    {
        BUF_SIZE_1 = (HD->P_V1 + 1) > HD->P_O1 * 2    ?    HD->P_O1_BYTE * (N_TRIANGLE_TERMS(HD->P_V1)) : HD->P_O1_BYTE * HD->P_V1 * HD->P_O1;
        
    }
    else
    {
        BUF_SIZE_1 = (HD->P_V1 + 1) > HD->P_O2 * 2    ?    HD->P_O2_BYTE * (N_TRIANGLE_TERMS(HD->P_V1)) : HD->P_O2_BYTE * HD->P_V1 * HD->P_O2;
        if ((HD->P_O2 < HD->P_O1) || (128 < (HD->P_O1_BYTE + HD->P_O2_BYTE)))
            printf("error: buffer size.\n");
    }



    unsigned char* buffer = (unsigned char*)rb_safe_malloc(BUF_SIZE_1);
    


    unsigned char* tmp = (unsigned char*)rb_safe_malloc(HD->P_TMPVEC_LEN * HD->P_GFSIZE);
   
    unsigned char *_x = (unsigned char *)rb_safe_malloc(HD->P_MAX_N);
   


    unsigned char* _v1 = _x;
    unsigned char* _o1 = _v1 + HD->P_V1;
    unsigned char* _o2 = _o1 + HD->P_O1;
    for (unsigned int i = 0; i < HD->P_PUB_N; i++) 
        _x[i] = HD->gfv_get_ele(w, i);

    rb_prng_gen(&prng0, buffer, HD->member_len_of_sk_t[5], HD->P_HASH_LEN);
    rb_accu_eval_quad(handle,tmp, buffer, _v1, HD->P_V1, HD->P_O1_BYTE);
    rb_prng_gen(&prng0, buffer, HD->member_len_of_sk_t[6], HD->P_HASH_LEN);
    rb_accu_eval_quad_rect(handle, tmp, _v1, HD->P_V1, buffer, _o1, HD->P_O1, HD->P_O1_BYTE);
    rb_accu_eval_quad_rect(handle, tmp, _v1, HD->P_V1, pk->l1_Q3, _o2, HD->P_O2, HD->P_O1_BYTE);
    rb_accu_eval_quad(handle,tmp, pk->l1_Q5, _o1, HD->P_O1, HD->P_O1_BYTE);
    rb_accu_eval_quad_rect(handle, tmp, _o1, HD->P_O1, pk->l1_Q6, _o2, HD->P_O2, HD->P_O1_BYTE);
    rb_accu_eval_quad(handle,tmp, pk->l1_Q9, _o2, HD->P_O2, HD->P_O1_BYTE);

    
    unsigned char* tmp2 = (unsigned char*)rb_safe_malloc(HD->P_TMPVEC_LEN * HD->P_GFSIZE);
 
    rb_prng_gen(&prng0, buffer, HD->member_len_of_sk_t[7], HD->P_HASH_LEN);
    rb_accu_eval_quad(handle,tmp2, buffer, _v1, HD->P_V1, HD->P_O2_BYTE);

    rb_prng_gen(&prng0, buffer, HD->member_len_of_sk_t[8], HD->P_HASH_LEN);
    rb_accu_eval_quad_rect(handle, tmp2, _v1, HD->P_V1, buffer, _o1, HD->P_O1, HD->P_O2_BYTE);

    rb_prng_gen(&prng0, buffer, HD->member_len_of_sk_t[9], HD->P_HASH_LEN);
    rb_accu_eval_quad_rect(handle, tmp2, _v1, HD->P_V1, buffer, _o2, HD->P_O2, HD->P_O2_BYTE);

    rb_prng_gen(&prng0, buffer, HD->member_len_of_sk_t[10], HD->P_HASH_LEN);
    rb_accu_eval_quad(handle,tmp2, buffer, _o1, HD->P_O1, HD->P_O2_BYTE);

    rb_prng_gen(&prng0, buffer, HD->member_len_of_sk_t[11], HD->P_HASH_LEN);
    rb_accu_eval_quad_rect(handle, tmp2, _o1, HD->P_O1, buffer, _o2, HD->P_O2, HD->P_O2_BYTE);

    rb_accu_eval_quad(handle,tmp2, pk->l2_Q9, _o2, HD->P_O2, HD->P_O2_BYTE);

    for (unsigned int i = 0; i < HD->P_GFSIZE; i++)
        HD->gf256v_add(tmp + i * HD->P_TMPVEC_LEN + HD->P_O1_BYTE, tmp2 + i * HD->P_TMPVEC_LEN, HD->P_O2_BYTE);

    rb_madd_reduce(handle,z, tmp, HD->P_PUB_M_BYTE);

    rb_safe_free(buffer);
    rb_safe_free(tmp);
    rb_safe_free(_x);
    rb_safe_free(tmp2);
    rb_safe_free(prng0.buf);

}





