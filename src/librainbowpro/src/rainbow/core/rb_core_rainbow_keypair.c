#include "rb_core_config.h"
#include "rb_core_RAINBOW_PRO_API.h"
#include "rb_core_rainbow_handle.h"
#include "rb_core_rainbow_keypair.h"
#include "rb_core_rainbow_keypair_computation.h"

#include "rb_core_blas_comm.h"
#include "rb_core_blas.h"



#include <stdlib.h>
#include <stdint.h>
#include <string.h>


/////////////////////////////////////////////////////////////////


#include "rb_core_utils_prng.h"


static void rb_generate_S_T_common(unsigned long handle, rb_sk_t* sk, rb_prng_t* prng0)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    rb_prng_gen(prng0, sk->s1, sk->s1_len, HD->P_HASH_LEN);
    rb_prng_gen(prng0, sk->t1, sk->t1_len, HD->P_HASH_LEN);
    rb_prng_gen(prng0, sk->t4, sk->t4_len, HD->P_HASH_LEN);
    rb_prng_gen(prng0, sk->t3, sk->t3_len, HD->P_HASH_LEN);


}
static void rb_generate_l1_F12_common(unsigned long handle, rb_sk_t* sk, rb_prng_t* prng0)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    rb_prng_gen(prng0, sk->l1_F1, sk->l1_F1_len, HD->P_HASH_LEN);
    rb_prng_gen(prng0, sk->l1_F2, sk->l1_F2_len, HD->P_HASH_LEN);

}
static void rb_generate_l2_F12356_common(unsigned long handle, rb_sk_t* sk, rb_prng_t* prng0)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    rb_prng_gen(prng0, sk->l2_F1, sk->l2_F1_len, HD->P_HASH_LEN);
    rb_prng_gen(prng0, sk->l2_F2, sk->l2_F2_len, HD->P_HASH_LEN);
    rb_prng_gen(prng0, sk->l2_F3, sk->l2_F3_len, HD->P_HASH_LEN);
    rb_prng_gen(prng0, sk->l2_F5, sk->l2_F5_len, HD->P_HASH_LEN);
    rb_prng_gen(prng0, sk->l2_F6, sk->l2_F6_len, HD->P_HASH_LEN);

}
static void rb_generate_B1_B2_common(unsigned long handle, rb_sk_t* sk, rb_prng_t* prng0)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    rb_generate_l1_F12_common(handle,sk, prng0);
    rb_generate_l2_F12356_common(handle,sk, prng0);
}
static void rb_generate_S_T(unsigned long handle, unsigned char * s_and_t , rb_prng_t * prng0)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int size;

    size = HD->member_len_of_sk_t[1]; ;
    rb_prng_gen( prng0 , s_and_t , size, HD->P_HASH_LEN);
    s_and_t += size;

    size = HD->member_len_of_sk_t[2];
    rb_prng_gen( prng0 , s_and_t , size, HD->P_HASH_LEN);
    s_and_t += size;

    size = HD->member_len_of_sk_t[3];
    rb_prng_gen( prng0 , s_and_t , size, HD->P_HASH_LEN);
    s_and_t += size;

    size = HD->member_len_of_sk_t[4];
    rb_prng_gen( prng0 , s_and_t , size, HD->P_HASH_LEN);
    s_and_t += size;
}


static unsigned int rb_generate_l1_F12(unsigned long handle, unsigned char * sk, rb_prng_t * prng0)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int n_byte_generated = 0;
    unsigned int size;

    size = HD->member_len_of_sk_t[5]; 
    rb_prng_gen( prng0 , sk , size, HD->P_HASH_LEN);
    sk += size;
    n_byte_generated += size;

    size = HD->member_len_of_sk_t[6]; 
    rb_prng_gen( prng0 , sk , size, HD->P_HASH_LEN);
    sk += size;
    n_byte_generated += size;

    return n_byte_generated;
}


static unsigned int rb_generate_l2_F12356(unsigned long handle, unsigned char * sk, rb_prng_t * prng0)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int n_byte_generated = 0;
   // rb_sk_t * _sk;
    unsigned int size;

    size = HD->member_len_of_sk_t[7];
    rb_prng_gen( prng0 , sk , size, HD->P_HASH_LEN);
    sk += size;
    n_byte_generated += size;

    size = HD->member_len_of_sk_t[8]; 
    rb_prng_gen( prng0 , sk , size, HD->P_HASH_LEN);
    sk += size;
    n_byte_generated += size;

    size = HD->member_len_of_sk_t[9]; 
    rb_prng_gen( prng0 , sk , size, HD->P_HASH_LEN);
    sk += size;
    n_byte_generated += size;

    size = HD->member_len_of_sk_t[10];
    rb_prng_gen( prng0 , sk , size, HD->P_HASH_LEN);
    sk += size;
    n_byte_generated += size;

    size = HD->member_len_of_sk_t[11]; 
    rb_prng_gen( prng0 , sk , size, HD->P_HASH_LEN);
    sk += size;
    n_byte_generated += size;

    return n_byte_generated;
}


static void rb_generate_B1_B2(unsigned long handle, unsigned char * sk , rb_prng_t * prng0)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    sk += rb_generate_l1_F12(handle, sk , prng0 );
    rb_generate_l2_F12356(handle, sk , prng0 );
}



static void rb_calculate_t4(unsigned long handle, unsigned char * t2_to_t4 , const unsigned char *t1 , const unsigned char *t3)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    unsigned char *temp=(unsigned char *)rb_safe_malloc(HD->P_V1_BYTE + 32);

    unsigned char* t4 = t2_to_t4;

    for (unsigned int i = 0; i < HD->P_O2; i++) 
    {
        HD->gfmat_prod(handle, temp, t1, HD->P_V1_BYTE, HD->P_O1, t3);
        HD->gf256v_add(t4, temp, HD->P_V1_BYTE);
        t4 += HD->P_V1_BYTE;
        t3 += HD->P_O1_BYTE;
    }
    rb_safe_free(temp);

}



static void rb_obfuscate_l1_polys(unsigned long handle, unsigned char * l1_polys , const unsigned char * l2_polys , unsigned int n_terms , const unsigned char * s1)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned char *temp=(unsigned char *)rb_safe_malloc(HD->P_O1_BYTE + 32);
   
    while( n_terms-- ) 
    {
        HD->gfmat_prod(handle,  temp , s1 , HD->P_O1_BYTE , HD->P_O2 , l2_polys );
        HD->gf256v_add( l1_polys , temp , HD->P_O1_BYTE );
        l1_polys += HD->P_O1_BYTE;
        l2_polys += HD->P_O2_BYTE;
    }
    rb_safe_free(temp);

}

///////////////////  Classic //////////////////////////////////

static void rb__generate_secretkey(unsigned long handle, rb_sk_t* sk, const unsigned char *sk_seed)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    if(sk->sk_seed_len)
        memcpy( sk->sk_seed, sk_seed, HD->P_LEN_SKSEED );

    rb_prng_t prng0;
    memset(&prng0, 0, sizeof(rb_prng_t));
    prng0.buf = (unsigned char*)rb_safe_malloc(HD->P_HASH_LEN);
   
    rb_prng_set( &prng0 , sk_seed , HD->P_LEN_SKSEED, HD->P_HASH_LEN);

    rb_generate_S_T_common(handle,sk, &prng0);
    rb_generate_B1_B2_common(handle,sk, &prng0);

    
    prng0.used = 0;
    rb_safe_free(prng0.buf);
}




#include "rb_core_rng.h" 




int rb_generate_secretkey_cyclic(unsigned long handle, rb_sk_t* sk, const unsigned char* pk_seed, const unsigned char* sk_seed)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    memcpy(sk->sk_seed, sk_seed, HD->P_LEN_SKSEED);

  
    rb_prng_t _prng;
    memset(&_prng, 0, sizeof(rb_prng_t));
    _prng.buf = (unsigned char*)rb_safe_malloc(HD->P_HASH_LEN);
  
    rb_prng_t* prng0 = &_prng;
    rb_prng_set(prng0, sk_seed, HD->P_LEN_SKSEED, HD->P_HASH_LEN);

    rb_generate_S_T_common(handle,sk, prng0);

    rb_calculate_t4(handle,sk->t4, sk->t1, sk->t3);

   

    unsigned char* temp0 = rb_safe_malloc(HD->sizeof_struct_sk_t);
    if (NULL == temp0)
        return -1;
  

    unsigned char* temp = temp0;
    rb_sk_t  _Qs;
    memset(&_Qs, 0, sizeof(rb_sk_t));
    rb_sk_t* Qs = &_Qs;

    Qs->sk_seed = temp;     temp += HD->member_len_of_sk_t[0];          Qs->sk_seed_len = HD->member_len_of_sk_t[0];
    Qs->s1 = temp;          temp += HD->member_len_of_sk_t[1];          Qs->s1_len = HD->member_len_of_sk_t[1];
    Qs->t1 = temp;          temp += HD->member_len_of_sk_t[2];          Qs->t1_len = HD->member_len_of_sk_t[2];
    Qs->t4 = temp;          temp += HD->member_len_of_sk_t[3];          Qs->t4_len = HD->member_len_of_sk_t[3];
    Qs->t3 = temp;          temp += HD->member_len_of_sk_t[4];          Qs->t3_len = HD->member_len_of_sk_t[4];
    Qs->l1_F1 = temp;       temp += HD->member_len_of_sk_t[5];          Qs->l1_F1_len = HD->member_len_of_sk_t[5];
    Qs->l1_F2 = temp;       temp += HD->member_len_of_sk_t[6];          Qs->l1_F2_len = HD->member_len_of_sk_t[6];
    Qs->l2_F1 = temp;       temp += HD->member_len_of_sk_t[7];          Qs->l2_F1_len = HD->member_len_of_sk_t[7];
    Qs->l2_F2 = temp;       temp += HD->member_len_of_sk_t[8];          Qs->l2_F2_len = HD->member_len_of_sk_t[8];
    Qs->l2_F3 = temp;       temp += HD->member_len_of_sk_t[9];          Qs->l2_F3_len = HD->member_len_of_sk_t[9];
    Qs->l2_F5 = temp;       temp += HD->member_len_of_sk_t[10];         Qs->l2_F5_len = HD->member_len_of_sk_t[10];
    Qs->l2_F6 = temp;                                                   Qs->l2_F6_len = HD->member_len_of_sk_t[11];



    rb_prng_t* prng1 = &_prng;
    rb_prng_set(prng1, pk_seed, HD->P_LEN_PKSEED, HD->P_HASH_LEN);

    rb_generate_B1_B2_common(handle,Qs, prng1);

    rb_obfuscate_l1_polys(handle, Qs->l1_F1, Qs->l2_F1, N_TRIANGLE_TERMS(HD->P_V1), sk->s1);
    rb_obfuscate_l1_polys(handle, Qs->l1_F2, Qs->l2_F2, HD->P_V1 * HD->P_O1, sk->s1);

   
    rb_calculate_F_from_Q(handle,sk, Qs, sk);

  
    rb_safe_free(temp0);
    rb_safe_free(_prng.buf);


    return 0;
}


int rb_generate_keypair_cyclic_common(unsigned long handle, rb_cpk_t* pk, rb_sk_t* sk, const unsigned char* pk_seed, const unsigned char* sk_seed)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    if(pk->pk_seed_len)
        memcpy(pk->pk_seed, pk_seed, HD->P_LEN_PKSEED);
    if(sk->sk_seed_len)
        memcpy(sk->sk_seed, sk_seed, HD->P_LEN_SKSEED);

  
    rb_prng_t prng;
    memset(&prng, 0, sizeof(rb_prng_t));
    prng.buf = (unsigned char*)rb_safe_malloc(HD->P_HASH_LEN);
   
    rb_prng_t* prng0 = &prng;

   
    rb_prng_set(prng0, sk_seed, HD->P_LEN_SKSEED, HD->P_HASH_LEN);

    rb_generate_S_T_common(handle,sk, prng0);


    unsigned char* t2 = NULL;
    unsigned int t2_len = HD->member_len_of_sk_t[3];

    if (t2_len)
    {
        t2 = (unsigned char*)rb_safe_malloc(HD->member_len_of_sk_t[3]);
        
    }
    
    if(t2_len)
        memcpy(t2, sk->t4, HD->P_V1_BYTE * HD->P_O2);    

    rb_calculate_t4(handle,sk->t4, sk->t1, sk->t3);   

  

    unsigned char* temp0 = rb_safe_malloc(HD->sizeof_struct_sk_t);
    if (NULL == temp0) 
        return -1;
  

    unsigned char* temp = temp0;
    rb_sk_t  _Qs;
    memset(&_Qs, 0, sizeof(rb_sk_t));
    rb_sk_t* Qs = &_Qs;

    Qs->sk_seed = temp;          temp += HD->member_len_of_sk_t[0];          Qs->sk_seed_len    = HD->member_len_of_sk_t[0];
    Qs->s1      = temp;          temp += HD->member_len_of_sk_t[1];          Qs->s1_len         = HD->member_len_of_sk_t[1];
    Qs->t1      = temp;          temp += HD->member_len_of_sk_t[2];          Qs->t1_len         = HD->member_len_of_sk_t[2];
    Qs->t4      = temp;          temp += HD->member_len_of_sk_t[3];          Qs->t4_len         = HD->member_len_of_sk_t[3];
    Qs->t3      = temp;          temp += HD->member_len_of_sk_t[4];          Qs->t3_len         = HD->member_len_of_sk_t[4];
    Qs->l1_F1   = temp;          temp += HD->member_len_of_sk_t[5];          Qs->l1_F1_len      = HD->member_len_of_sk_t[5];
    Qs->l1_F2   = temp;          temp += HD->member_len_of_sk_t[6];          Qs->l1_F2_len      = HD->member_len_of_sk_t[6];
    Qs->l2_F1   = temp;          temp += HD->member_len_of_sk_t[7];          Qs->l2_F1_len      = HD->member_len_of_sk_t[7];
    Qs->l2_F2   = temp;          temp += HD->member_len_of_sk_t[8];          Qs->l2_F2_len      = HD->member_len_of_sk_t[8];
    Qs->l2_F3   = temp;          temp += HD->member_len_of_sk_t[9];          Qs->l2_F3_len      = HD->member_len_of_sk_t[9];
    Qs->l2_F5   = temp;          temp += HD->member_len_of_sk_t[10];         Qs->l2_F5_len      = HD->member_len_of_sk_t[10];
    Qs->l2_F6   = temp;                                                      Qs->l2_F6_len      = HD->member_len_of_sk_t[11];


    rb_prng_t* prng1 = &prng;
    rb_prng_set(prng1, pk_seed, HD->P_LEN_PKSEED, HD->P_HASH_LEN);


    rb_generate_B1_B2_common(handle,Qs, prng1);


    rb_obfuscate_l1_polys(handle, Qs->l1_F1, Qs->l2_F1, N_TRIANGLE_TERMS(HD->P_V1), sk->s1);
    rb_obfuscate_l1_polys(handle, Qs->l1_F2, Qs->l2_F2, HD->P_V1 * HD->P_O1, sk->s1);
  

    rb_calculate_F_from_Q(handle,sk, Qs, sk);        

    unsigned char* t4 = NULL;
    unsigned int t4_len = HD->member_len_of_sk_t[3];
    if (t4_len)
    {
        t4 = (unsigned char*)rb_safe_malloc(HD->member_len_of_sk_t[3]);
     
    }
    
    if(t4_len)
        memcpy(t4, sk->t4, HD->P_V1_BYTE * HD->P_O2);       
    if (sk->t4_len)
    {
        if(sk->t4 && t2)
            memcpy(sk->t4, t2, HD->P_V1_BYTE * HD->P_O2);  
    }

    rb_calculate_Q_from_F_cyclic(handle,pk, sk, sk);    

    if (sk->t4_len)
    {
        if(sk->t4 && t4)
            memcpy(sk->t4, t4, HD->P_V1_BYTE * HD->P_O2);  
    }


    rb_obfuscate_l1_polys(handle, pk->l1_Q3, Qs->l2_F3, HD->P_V1 * HD->P_O2, sk->s1);
    rb_obfuscate_l1_polys(handle, pk->l1_Q5, Qs->l2_F5, N_TRIANGLE_TERMS(HD->P_O1), sk->s1);
    rb_obfuscate_l1_polys(handle, pk->l1_Q6, Qs->l2_F6, HD->P_O1 * HD->P_O2, sk->s1);
    rb_obfuscate_l1_polys(handle, pk->l1_Q9, pk->l2_Q9, N_TRIANGLE_TERMS(HD->P_O2), sk->s1);

    // clean
    memset(prng.buf, 0, HD->P_HASH_LEN);
    prng.used = 0;

    rb_safe_free(t2);
    rb_safe_free(t4);
    rb_safe_free(temp0);
    rb_safe_free(prng.buf);


    return 0;
}


int rb_generate_keypair_cyclic(unsigned long handle, rb_cpk_t * pk, rb_sk_t* sk, const unsigned char *pk_seed , const unsigned char *sk_seed)
{

    rb_generate_keypair_cyclic_common(handle,pk, sk, pk_seed, sk_seed);

    return 0;
}



int rb_generate_compact_keypair_cyclic(unsigned long handle, rb_cpk_t * pk, rb_csk_t* rsk, const unsigned char *pk_seed , const unsigned char *sk_seed)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    memcpy( rsk->pk_seed , pk_seed , HD->P_LEN_PKSEED );
    memcpy( rsk->sk_seed , sk_seed , HD->P_LEN_SKSEED );


    unsigned char* temp0 = rb_safe_malloc(HD->sizeof_struct_sk_t);
	if (NULL == temp0) return -1;
   

    unsigned char* temp = temp0;
    rb_sk_t  _sk;
    memset(&_sk, 0, sizeof(rb_sk_t));
    rb_sk_t* sk = &_sk;

    sk->sk_seed = temp;   temp += HD->member_len_of_sk_t[0];        sk->sk_seed_len = HD->member_len_of_sk_t[0];
    sk->s1 = temp;   temp += HD->member_len_of_sk_t[1];             sk->s1_len = HD->member_len_of_sk_t[1];
    sk->t1 = temp;   temp += HD->member_len_of_sk_t[2];             sk->t1_len = HD->member_len_of_sk_t[2];
    sk->t4 = temp;   temp += HD->member_len_of_sk_t[3];             sk->t4_len = HD->member_len_of_sk_t[3];
    sk->t3 = temp;   temp += HD->member_len_of_sk_t[4];             sk->t3_len = HD->member_len_of_sk_t[4];
    sk->l1_F1 = temp;   temp += HD->member_len_of_sk_t[5];          sk->l1_F1_len = HD->member_len_of_sk_t[5];
    sk->l1_F2 = temp;   temp += HD->member_len_of_sk_t[6];          sk->l1_F2_len = HD->member_len_of_sk_t[6];
    sk->l2_F1 = temp;   temp += HD->member_len_of_sk_t[7];          sk->l2_F1_len = HD->member_len_of_sk_t[7];
    sk->l2_F2 = temp;   temp += HD->member_len_of_sk_t[8];          sk->l2_F2_len = HD->member_len_of_sk_t[8];
    sk->l2_F3 = temp;   temp += HD->member_len_of_sk_t[9];          sk->l2_F3_len = HD->member_len_of_sk_t[9];
    sk->l2_F5 = temp;   temp += HD->member_len_of_sk_t[10];         sk->l2_F5_len = HD->member_len_of_sk_t[10];
    sk->l2_F6 = temp;                                               sk->l2_F6_len = HD->member_len_of_sk_t[11];



    int r = rb_generate_keypair_cyclic(handle, pk , sk , pk_seed , sk_seed);

    rb_safe_free(temp0);
    return r;
}




