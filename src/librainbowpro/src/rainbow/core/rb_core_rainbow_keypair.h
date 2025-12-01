#ifndef rb__RAINBOW_KEYPAIR_H_
#define rb__RAINBOW_KEYPAIR_H_


#include "rb_core_rainbow_handle.h"
#include "rb_core_config.h"

#ifdef MATCH_WITH_HAHAHA
#define N_TRIANGLE_TERMS(n_var) (n_var*(n_var+1)/2+n_var+1)
#else
#define N_TRIANGLE_TERMS(n_var) (n_var*(n_var+1)/2)
#endif

#ifdef  __cplusplus
extern  "C" {
#endif


    typedef struct rb_rainbow_publickey
    {
        unsigned char *pk;
    } rb_pk_t;


typedef struct rb_rainbow_secretkey
{
    unsigned char *sk_seed;

    unsigned char *s1;   
    unsigned char *t1;   
    unsigned char *t4;   
    unsigned char *t3;   

    unsigned char *l1_F1;
    unsigned char *l1_F2;

    unsigned char *l2_F1;
    unsigned char *l2_F2;

    unsigned char *l2_F3;
    unsigned char *l2_F5;
    unsigned char *l2_F6;

    unsigned int   sk_seed_len;
    unsigned int   s1_len;   
    unsigned int   t1_len;   
    unsigned int   t4_len;   
    unsigned int   t3_len;   

    unsigned int   l1_F1_len;
    unsigned int   l1_F2_len;

    unsigned int   l2_F1_len;
    unsigned int   l2_F2_len;

    unsigned int   l2_F3_len;
    unsigned int   l2_F5_len;
    unsigned int   l2_F6_len;
} rb_sk_t;


typedef struct rb_rainbow_publickey_cyclic 
{
    unsigned char *pk_seed;                      

    unsigned char *l1_Q3;  
    unsigned char *l1_Q5;  
    unsigned char *l1_Q6;  
    unsigned char *l1_Q9;  

    unsigned char *l2_Q9;  

    unsigned int pk_seed_len;

    unsigned int l1_Q3_len;
    unsigned int l1_Q5_len;
    unsigned int l1_Q6_len;
    unsigned int l1_Q9_len;

    unsigned int l2_Q9_len;
} rb_cpk_t;



typedef struct rb_rainbow_secretkey_cyclic
{
    unsigned char *pk_seed;   
    unsigned char *sk_seed;   

    unsigned int  pk_seed_len;
    unsigned int  sk_seed_len;
} rb_csk_t;



int rb_generate_keypair_cyclic_common(unsigned long handle, rb_cpk_t * pk, rb_sk_t* sk, const unsigned char *pk_seed , const unsigned char *sk_seed);
int rb_generate_keypair_cyclic(unsigned long handle, rb_cpk_t* pk, rb_sk_t* sk, const unsigned char* pk_seed, const unsigned char* sk_seed);
int rb_generate_compact_keypair_cyclic(unsigned long handle, rb_cpk_t * pk, rb_csk_t* sk, const unsigned char *pk_seed , const unsigned char *sk_seed);
int rb_generate_secretkey_cyclic(unsigned long handle, rb_sk_t* sk, const unsigned char *pk_seed , const unsigned char *sk_seed);
void rb_rainbow_publicmap_cpk(unsigned long handle, unsigned char * z, const rb_cpk_t * pk, const unsigned char *w);



#ifdef  __cplusplus
}
#endif

#endif 
