#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "rb_core_config.h"
#include "rb_core_rainbow_keypair.h"
#include "rb_core_rainbow.h"
#include "rb_core_blas.h"
#include "rb_core_utils_prng.h"
#include "rb_core_utils_hash.h"
#include "rb_core_api.h"

extern unsigned int rainbow_pro_savebuffer_tofile(unsigned long handle, char* filename, unsigned char* buffer, unsigned int buffer_size, char* dataname, int mode, int format);

int rb_rainbow_sign_common(unsigned long handle, uint8_t* signature, rb_sk_t* sk, uint8_t* _digest)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    uint8_t* mat_l1 = NULL;
    unsigned int mat_l1_len = HD->P_O1 * HD->P_O1_BYTE;
    if (mat_l1_len)
    {
        mat_l1 = (uint8_t*)rb_safe_malloc(mat_l1_len);
        if (mat_l1 == NULL)
            return -1;
    
    }
    uint8_t* mat_l2 = NULL;
    unsigned int mat_l2_len = HD->P_O2 * HD->P_O2_BYTE;
    if (mat_l2_len)
    {
        mat_l2 = (uint8_t*)rb_safe_malloc(mat_l2_len);
        if (mat_l2 == NULL)
            return -1;
        memset(mat_l2, 0, mat_l2_len);
    }

    uint8_t* vinegar = (uint8_t*)rb_safe_malloc(HD->P_V1_BYTE);
   
    rb_prng_t prng_sign;
    memset(&prng_sign, 0, sizeof(rb_prng_t));
    prng_sign.buf = (unsigned char*)rb_safe_malloc(HD->P_HASH_LEN);
 

    uint8_t* prng_preseed = (uint8_t*)rb_safe_malloc(HD->P_LEN_SKSEED + HD->P_HASH_LEN);
 

    memset(prng_preseed, 0, HD->P_LEN_SKSEED + HD->P_HASH_LEN);
    memcpy(prng_preseed, sk->sk_seed, HD->P_LEN_SKSEED);
    memcpy(prng_preseed + HD->P_LEN_SKSEED, _digest, HD->P_HASH_LEN);                     
    uint8_t* prng_seed = (uint8_t*)rb_safe_malloc(HD->P_HASH_LEN);
   
    rb_hash_msg(prng_seed, HD->P_HASH_LEN, prng_preseed, (unsigned int)HD->P_HASH_LEN + (unsigned int)HD->P_LEN_SKSEED, HD->P_HASH_LEN);
    rb_prng_set(&prng_sign, prng_seed, HD->P_HASH_LEN, HD->P_HASH_LEN);                                  
    for (unsigned int i = 0; i < HD->P_LEN_SKSEED + HD->P_HASH_LEN; i++) prng_preseed[i] ^= prng_preseed[i]; 
    for (unsigned int i = 0; i < HD->P_HASH_LEN; i++) prng_seed[i] ^= prng_seed[i];                 



    unsigned int n_attempt = 0;
    unsigned int l1_succ = 0;
    while (!l1_succ)
    {
        if (HD->P_MAX_ATTEMPT_FRMAT <= n_attempt) break;
        if (HD->set_no_salt == 1)
        {
            rb_prng_gen(&prng_sign, vinegar, HD->P_V1_BYTE, HD->P_HASH_LEN);                     
        }
        else
        {
            rb_gf256v_rand(vinegar, HD->P_V1_BYTE);
        }

        if (mat_l1_len && sk->l1_F2_len)
            HD->gfmat_prod(handle, mat_l1, sk->l1_F2, mat_l1_len, HD->P_V1, vinegar);   

        if (mat_l1_len)
            l1_succ = HD->gfmat_inv(handle, mat_l1, mat_l1);    

        n_attempt++;
    }



    uint8_t* r_l1_F1 = NULL;
    unsigned int r_l1_F1_len = HD->P_O1_BYTE;
    if (r_l1_F1_len)
    {
        r_l1_F1 = (uint8_t*)rb_safe_malloc(r_l1_F1_len);
    
    }
    uint8_t* r_l2_F1 = NULL;
    unsigned int r_l2_F1_len = HD->P_O2_BYTE;
    if (r_l2_F1_len)
    {
        r_l2_F1 = (uint8_t*)rb_safe_malloc(r_l2_F1_len);
    
    }
    if (r_l1_F1_len && sk->l1_F1_len)
        HD->batch_quad_trimat_eval(handle, r_l1_F1, sk->l1_F1, vinegar, HD->P_V1, HD->P_O1_BYTE);
    if (r_l2_F1_len && sk->l2_F1_len)
        HD->batch_quad_trimat_eval(handle, r_l2_F1, sk->l2_F1, vinegar, HD->P_V1, HD->P_O2_BYTE);

    uint8_t* mat_l2_F3 = NULL;
    unsigned int mat_l2_F3_len = HD->P_O2 * HD->P_O2_BYTE;
    if (mat_l2_F3_len)
    {
        mat_l2_F3 = (uint8_t*)rb_safe_malloc(mat_l2_F3_len);
       
    }
    uint8_t* mat_l2_F2 = NULL;
    unsigned int mat_l2_F2_len = HD->P_O1 * HD->P_O2_BYTE;
    if (mat_l2_F2_len)
    {
        mat_l2_F2 = (uint8_t*)rb_safe_malloc(mat_l2_F2_len);
 
    }

    if (mat_l2_F3_len && sk->l2_F3_len)
        HD->gfmat_prod(handle, mat_l2_F3, sk->l2_F3, HD->P_O2 * HD->P_O2_BYTE, HD->P_V1, vinegar);
    if (mat_l2_F2_len && sk->l2_F2_len)
        HD->gfmat_prod(handle, mat_l2_F2, sk->l2_F2, HD->P_O1 * HD->P_O2_BYTE, HD->P_V1, vinegar);


    uint8_t* _z = (uint8_t*)rb_safe_malloc(HD->P_PUB_M_BYTE);

    uint8_t* y = (uint8_t*)rb_safe_malloc(HD->P_PUB_N_BYTE);
  
    uint8_t* x_v1 = vinegar;
    uint8_t* x_o1 = (uint8_t*)rb_safe_malloc(HD->P_O1_BYTE);
 
    uint8_t* x_o2 = (uint8_t*)rb_safe_malloc(HD->P_O2_BYTE);
   

    uint8_t* digest_salt = (uint8_t*)rb_safe_malloc(HD->P_HASH_LEN + HD->P_SALT_BYTE);
  

    memcpy(digest_salt, _digest, HD->P_HASH_LEN);
    uint8_t* salt = digest_salt + HD->P_HASH_LEN;


    uint8_t* temp_o = (uint8_t*)rb_safe_malloc(HD->P_MAX_O_BYTE + 32);
    if (temp_o == NULL)
        return -1;
 

    unsigned succ = 0;


    while (!succ)
    {

        if (HD->P_MAX_ATTEMPT_FRMAT <= n_attempt) break;
       

        if (HD->set_no_salt == 1)
        {
            rb_prng_gen(&prng_sign, salt, HD->P_SALT_BYTE, HD->P_HASH_LEN);                      
        }
        else
        {
            rb_gf256v_rand(salt, HD->P_SALT_BYTE);  /// line 8 
        }
        rb_hash_msg(_z, HD->P_PUB_M_BYTE, digest_salt, HD->P_HASH_LEN + HD->P_SALT_BYTE, HD->P_HASH_LEN); 

      
        memcpy(y, _z, HD->P_PUB_M_BYTE);                                   
        HD->gfmat_prod(handle, temp_o, sk->s1, HD->P_O1_BYTE, HD->P_O2, _z + HD->P_O1_BYTE);
        HD->gf256v_add(y, temp_o, HD->P_O1_BYTE);

    
        memcpy(temp_o, r_l1_F1, HD->P_O1_BYTE);
        HD->gf256v_add(temp_o, y, HD->P_O1_BYTE);
        HD->gfmat_prod(handle, x_o1, mat_l1, HD->P_O1_BYTE, HD->P_O1, temp_o);


        rb_gf256v_set_zero(handle, temp_o, HD->P_O2_BYTE);

        if (mat_l2_F2_len && HD->P_O2_BYTE)
            HD->gfmat_prod(handle, temp_o, mat_l2_F2, HD->P_O2_BYTE, HD->P_O1, x_o1);            
        if (mat_l2_len && sk->l2_F5_len && HD->P_O2_BYTE)
            HD->batch_quad_trimat_eval(handle, mat_l2, sk->l2_F5, x_o1, HD->P_O1, HD->P_O2_BYTE);


        HD->gf256v_add(temp_o, mat_l2, HD->P_O2_BYTE);
        HD->gf256v_add(temp_o, r_l2_F1, HD->P_O2_BYTE);                   
        HD->gf256v_add(temp_o, y + HD->P_O1_BYTE, HD->P_O2_BYTE);


        if (mat_l2_len && sk->l2_F6_len && HD->P_O2_BYTE)
            HD->gfmat_prod(handle, mat_l2, sk->l2_F6, HD->P_O2 * HD->P_O2_BYTE, HD->P_O1, x_o1);  
        HD->gf256v_add(mat_l2, mat_l2_F3, HD->P_O2 * HD->P_O2_BYTE);              



        if (mat_l2_len)
            succ = HD->gfmat_solve_linear_eq(handle,x_o2, mat_l2, temp_o);
        else
            succ = 1;



        n_attempt++;
    };

 

    unsigned int w_len = HD->P_PUB_N_BYTE;
    uint8_t* w = (uint8_t*)rb_safe_malloc(w_len);
    

    if (HD->P_V1_BYTE)
        memcpy(w, x_v1, HD->P_V1_BYTE);
    if (HD->P_O1_BYTE)
        memcpy(w + HD->P_V1_BYTE, x_o1, HD->P_O1_BYTE);
    if (HD->P_O2_BYTE)
        memcpy(w + HD->P_V2_BYTE, x_o2, HD->P_O2_BYTE);

    HD->gfmat_prod(handle, y, sk->t1, HD->P_V1_BYTE, HD->P_O1, x_o1);
    HD->gf256v_add(w, y, HD->P_V1_BYTE);

    HD->gfmat_prod(handle, y, sk->t4, HD->P_V1_BYTE, HD->P_O2, x_o2);
    HD->gf256v_add(w, y, HD->P_V1_BYTE);

    HD->gfmat_prod(handle, y, sk->t3, HD->P_O1_BYTE, HD->P_O2, x_o2);
    HD->gf256v_add(w + HD->P_V1_BYTE, y, HD->P_O1_BYTE);

    memset(signature, 0, HD->P_SIGNATURE_BYTE); 
    

    if(mat_l1)
    {
            memset(mat_l1, 0, mat_l1_len);
    }
    if(mat_l2)
    {
            memset(mat_l2, 0, mat_l2_len);
    }

    memset(prng_sign.buf, 0, HD->P_HASH_LEN);
    prng_sign.used = 0;


    if(HD->P_V1_BYTE)
        memset(vinegar, 0, HD->P_V1_BYTE);
    if(HD->P_O1_BYTE)
        memset(r_l1_F1, 0, HD->P_O1_BYTE);
    if(HD->P_O2_BYTE)
        memset(r_l2_F1, 0, HD->P_O2_BYTE);
    if(mat_l2_F3)
        memset(mat_l2_F3, 0, HD->P_O2 * HD->P_O2_BYTE);
    if(mat_l2_F2)
        memset(mat_l2_F2, 0, HD->P_O1 * HD->P_O2_BYTE);
    if(HD->P_PUB_M_BYTE)
        memset(_z, 0, HD->P_PUB_M_BYTE);
    if(HD->P_PUB_N_BYTE)
        memset(y, 0, HD->P_PUB_N_BYTE);
    if(HD->P_O1_BYTE)
        memset(x_o1, 0, HD->P_O1_BYTE);
    if(HD->P_O2_BYTE)
        memset(x_o2, 0, HD->P_O2_BYTE);
    if(HD->P_MAX_O_BYTE + 32)
        memset(temp_o, 0, HD->P_MAX_O_BYTE + 32);


   
 
    if (HD->P_MAX_ATTEMPT_FRMAT <= n_attempt)
        return -1;


    HD->gf256v_add(signature, w, HD->P_PUB_N_BYTE);
    HD->gf256v_add(signature + HD->P_PUB_N_BYTE, salt, HD->P_SALT_BYTE);

    rb_safe_free(mat_l1);
    rb_safe_free(mat_l2);
    rb_safe_free(prng_preseed);
    rb_safe_free(prng_seed);
    rb_safe_free(vinegar);

    rb_safe_free(r_l1_F1);
    rb_safe_free(r_l2_F1);
    rb_safe_free(mat_l2_F3);
    rb_safe_free(mat_l2_F2);

    rb_safe_free(_z);
    rb_safe_free(y);
    rb_safe_free(x_o1);
    rb_safe_free(x_o2);
    rb_safe_free(digest_salt);
    rb_safe_free(temp_o);
    rb_safe_free(w);
    rb_safe_free(prng_sign.buf);

    return 0;
}
int rb_rainbow_sign(unsigned long handle, uint8_t * signature , rb_sk_t * sk , uint8_t * _digest)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    rb_rainbow_sign_common(handle,signature, sk, _digest);
    return 0;
}

static unsigned int rb__rainbow_verify(unsigned long handle, uint8_t * digest_hash ,  uint8_t * salt ,  unsigned char * digest_ck)
{

    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned char *correct=(unsigned char *)rb_safe_malloc(HD->P_PUB_M_BYTE);
    if (correct == NULL)
        return 1;
  
    unsigned char *digest_salt=(unsigned char *)rb_safe_malloc(HD->P_HASH_LEN + HD->P_SALT_BYTE);
    if (digest_salt == NULL)
        return 1;
   

    memcpy(digest_salt, digest_hash, HD->P_HASH_LEN);
    memcpy(digest_salt + HD->P_HASH_LEN, salt, HD->P_SALT_BYTE);
    rb_hash_msg(correct, HD->P_PUB_M_BYTE, digest_salt, (unsigned int)HD->P_HASH_LEN + HD->P_SALT_BYTE, HD->P_HASH_LEN);  // H( digest || salt )


    // check consistancy.
    unsigned char cc = 0;
    for (unsigned int i = 0; i < HD->P_PUB_M_BYTE; i++) 
    {
        cc |= (digest_ck[i] ^ correct[i]);
    }
    if (HD->set_display_debuginfo == 1)
    {
        rainbow_pro_savebuffer_tofile((unsigned long)HD, NULL, digest_ck, HD->P_PUB_M_BYTE, " Verify_A", 1, 0);
        rainbow_pro_savebuffer_tofile((unsigned long)HD, NULL, correct, HD->P_PUB_M_BYTE, " Verify_B", 1, 0);
    }

    rb_safe_free(correct);
    rb_safe_free(digest_salt);

    return (0==cc)? 0: 1;
}


unsigned int rb_rainbow_sign_cyclic(unsigned long handle, uint8_t *signature , rb_csk_t * csk , uint8_t * digest)
{

    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned char* temp0 = rb_safe_malloc(HD->sizeof_struct_sk_t);
	if (NULL == temp0) return 1;
  
	
    unsigned char* temp = temp0;
    rb_sk_t  _sk;
    rb_sk_t* sk = &_sk;

    sk->sk_seed     = temp;   temp += HD->member_len_of_sk_t[0];             sk->sk_seed_len    = HD->member_len_of_sk_t[0];
    sk->s1          = temp;   temp += HD->member_len_of_sk_t[1];             sk->s1_len         = HD->member_len_of_sk_t[1];
    sk->t1          = temp;   temp += HD->member_len_of_sk_t[2];             sk->t1_len         = HD->member_len_of_sk_t[2];
    sk->t4          = temp;   temp += HD->member_len_of_sk_t[3];             sk->t4_len         = HD->member_len_of_sk_t[3];
    sk->t3          = temp;   temp += HD->member_len_of_sk_t[4];             sk->t3_len         = HD->member_len_of_sk_t[4];
    sk->l1_F1       = temp;   temp += HD->member_len_of_sk_t[5];             sk->l1_F1_len      = HD->member_len_of_sk_t[5];
    sk->l1_F2       = temp;   temp += HD->member_len_of_sk_t[6];             sk->l1_F2_len      = HD->member_len_of_sk_t[6];
    sk->l2_F1       = temp;   temp += HD->member_len_of_sk_t[7];             sk->l2_F1_len      = HD->member_len_of_sk_t[7];
    sk->l2_F2       = temp;   temp += HD->member_len_of_sk_t[8];             sk->l2_F2_len      = HD->member_len_of_sk_t[8];
    sk->l2_F3       = temp;   temp += HD->member_len_of_sk_t[9];             sk->l2_F3_len      = HD->member_len_of_sk_t[9];
    sk->l2_F5       = temp;   temp += HD->member_len_of_sk_t[10];            sk->l2_F5_len      = HD->member_len_of_sk_t[10];
    sk->l2_F6       = temp;                                                  sk->l2_F6_len      = HD->member_len_of_sk_t[11];

    rb_generate_secretkey_cyclic(handle, sk, csk->pk_seed , csk->sk_seed);  

    int r = rb_rainbow_sign(handle, signature , sk , digest);


    rb_safe_free(temp0);
    return r;
}
unsigned int rb_rainbow_verify_cyclic(unsigned long handle, uint8_t * digest_hash ,  uint8_t * signature ,  rb_cpk_t * _pk)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int ret = 0;
    unsigned char *digest_ck=(unsigned char *)rb_safe_malloc(HD->P_PUB_M_BYTE);
  
    if (HD->set_display_debuginfo == 1)
    {
        rainbow_pro_savebuffer_tofile((unsigned long)HD, NULL, digest_hash, HD->P_HASH_LEN, " Verify_HASH", 1, 0);
        rainbow_pro_savebuffer_tofile((unsigned long)HD, NULL, signature, HD->P_CRYPTO_BYTES, " Verify_SIGN", 1, 0);
    }
    rb_rainbow_publicmap_cpk(handle,digest_ck, _pk, signature);
    ret = rb__rainbow_verify(handle,digest_hash, signature + HD->P_PUB_N_BYTE, digest_ck);
    rb_safe_free(digest_ck);
    return ret;

}


