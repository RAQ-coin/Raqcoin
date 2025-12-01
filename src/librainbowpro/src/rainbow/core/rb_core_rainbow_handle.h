#ifndef rb__rainbow_handle_h
#define rb__rainbow_handle_h



#ifdef  __cplusplus
extern  "C" {
#endif


//#define MATCH_WITH_HAHAHA 

typedef enum _rb_PQC_SIGN_TYPE
{
    _PQC_RAINBOWPRO = 1,
    _PQC_MIX
} rb_PQC_SIGN_TYPE;


typedef struct {
    unsigned char   Key[32];
    unsigned char   V[16];
    int             reseed_counter;
} rb_AES256_CTR_DRBG_struct;


typedef enum rb__RAINBOW_TYPE_ID
{
    _RAINBOW_COMPRESSED_TYPE=3,

} rb_RAINBOW_TYPE_ID;

typedef enum rb__RAINBOW_PAR_ID
{

    _UOV_RAINBOW256_112_44_0_PAR= 2,
    _UOV_RAINBOW256_184_72_0_PAR =3,
    _UOV_RAINBOW256_244_96_0_PAR =4,
    _RAINBOW16_72_8_48_PAR=6,      
    _RAINBOW256_104_8_64_PAR=8,    
    _RAINBOW256_148_8_96_PAR=10,    

} rb_RAINBOW_PAR_ID;



typedef enum rb__USE_GF16_OR_GF256
{
    _USE_GF16_PAR = 1,
    _USE_GF256_PAR,

} rb_USE_GF16_OR_GF256;





typedef struct _RB_CORE_HANDLE
{
    unsigned int    set_type;
    unsigned int    set_subtype; 
    unsigned int    set_rainbow_type_id;            
    unsigned int    set_rainbow_par_id;             
    unsigned int    set_use_gf16_gf256_par_id;  

    unsigned int    P_GFSIZE;
    unsigned int    P_V1;
    unsigned int    P_V2;
    unsigned int    P_O1;
    unsigned int    P_O2;
    unsigned int    P_HASH_LEN;

    unsigned int    P_PUB_N; 
    unsigned int    P_PUB_M; 

    unsigned int    P_V1_BYTE;
    unsigned int    P_V2_BYTE;
    unsigned int    P_O1_BYTE;
    unsigned int    P_O2_BYTE;
    unsigned int    P_PUB_N_BYTE;
    unsigned int    P_PUB_M_BYTE;
    unsigned int    P_MAX_O; 
    unsigned int    P_MAX_O_BYTE;

    unsigned int    P_CRYPTO_SECRETKEYBYTES;
    unsigned int    P_CRYPTO_PUBLICKEYBYTES;

    

    unsigned int    P_SALT_BYTE; 
    unsigned int    P_LEN_SKSEED; 
    unsigned int    P_LEN_PKSEED; 
    unsigned int    P_SIGNATURE_BYTE;
    unsigned int    P_CRYPTO_BYTES;

    unsigned int    P_TMPVEC_LEN;

    unsigned int    P_MAX_N;
    unsigned int    P_MAX_ATTEMPT_FRMAT;

    unsigned int    sizeof_struct_pk_t;
    unsigned int    member_len_of_pk_t[1];
    unsigned int    sizeof_struct_sk_t;
    unsigned int    member_len_of_sk_t[12];
    unsigned int    sizeof_struct_cpk_t;
    unsigned int    member_len_of_cpk_t[6];
    unsigned int    sizeof_struct_csk_t;
    unsigned int    member_len_of_csk_t[2];
    unsigned int    sizeof_struct_ext_cpk_t;
    unsigned int    member_len_of_ext_cpk_t[12];

    unsigned int (*gfmat_inv)(unsigned long handle,unsigned char*, const unsigned char*);
    unsigned int (*gfmat_solve_linear_eq)(unsigned long handle, unsigned char*, const unsigned char*, const unsigned char*);

    unsigned char (*gfv_get_ele)(const unsigned char*, unsigned int);
    void (*gfv_mul_scalar)(unsigned char*, unsigned char, unsigned int);
    void (*gfv_madd)(unsigned char*, const unsigned char*, unsigned char, unsigned int);
    void (*gf256v_add)(unsigned char*, const unsigned char*, unsigned int);
    void (*gf256v_conditional_add)(unsigned char*, unsigned char, const unsigned char*, unsigned int);
    
                       
    void (*gfmat_prod)(unsigned long handle, unsigned char* c, const unsigned char* , unsigned int , unsigned int , const unsigned char* );

    void (*batch_trimat_madd)(unsigned long handle, unsigned char*, const unsigned char*,
        const unsigned char*, unsigned int, unsigned int, unsigned int, unsigned int);
    void (*batch_trimatTr_madd)(unsigned long handle, unsigned char*, const unsigned char*,
        const unsigned char*, unsigned int, unsigned int, unsigned int, unsigned int);
    void (*batch_2trimat_madd)(unsigned long handle, unsigned char*, const unsigned char*,
        const unsigned char*, unsigned int, unsigned int, unsigned int, unsigned int);
    void (*batch_matTr_madd)(unsigned long handle, unsigned char*, const unsigned char*, unsigned int, unsigned int, unsigned int,
        const unsigned char*, unsigned int, unsigned int);
    void (*batch_bmatTr_madd)(unsigned long handle, unsigned char*, const unsigned char*, unsigned int,
        const unsigned char*, unsigned int, unsigned int, unsigned int, unsigned int);
    void (*batch_mat_madd)(unsigned long handle, unsigned char*, const unsigned char*, unsigned int,
        const unsigned char*, unsigned int, unsigned int, unsigned int, unsigned int);

    void (*batch_quad_trimat_eval)(unsigned long handle, unsigned char*, const unsigned char*, const unsigned char*, unsigned int, unsigned int);
    void (*batch_quad_recmat_eval)(unsigned long handle, unsigned char*, const unsigned char*, unsigned int, const unsigned char*,
        const unsigned char*, unsigned int, unsigned int);


    unsigned int(*gf16mat_gauss_elim_8x16_impl)(unsigned long handle, unsigned char* mat);


    unsigned int(*gf256mat_gauss_elim_impl)(unsigned long handle, unsigned char* mat, unsigned int h, unsigned int w);

    void (*gf16mat_prod_impl)(unsigned long handle, unsigned char* c, const unsigned char* matA, unsigned int n_A_vec_byte, unsigned int n_A_width, const unsigned char* b);
    void (*gf256mat_prod_impl)(unsigned long handle, unsigned char* c, const unsigned char* matA, unsigned int n_A_vec_byte, unsigned int n_A_width, const unsigned char* b);

    unsigned int (*gf16mat_inv_8x8_impl)(unsigned long handle, unsigned char* inv_a, const unsigned char* a);
   
    unsigned int (*gf16mat_solve_linear_eq_48x48_impl)(unsigned long handle, unsigned char* sol, const unsigned char* inp_mat, const unsigned char* c_terms);

    unsigned int (*gf256mat_inv_8x8_impl)(unsigned long handle, unsigned char* inv_a, const unsigned char* a);
    unsigned int (*gf256mat_inv_32x32_impl)(unsigned long handle, unsigned char* inv_a, const unsigned char* a);
    unsigned int (*gf256mat_inv_36x36_impl)(unsigned long handle, unsigned char* inv_a, const unsigned char* a);
    unsigned int (*gf256mat_inv_44x44_impl)(unsigned long handle, unsigned char* inv_a, const unsigned char* a);
    unsigned int (*gf256mat_inv_72x72_impl)(unsigned long handle, unsigned char* inv_a, const unsigned char* a);
    unsigned int (*gf256mat_inv_96x96_impl)(unsigned long handle, unsigned char* inv_a, const unsigned char* a);

    unsigned int (*gf256mat_solve_linear_eq_48x48_impl)(unsigned long handle, unsigned char* sol, const unsigned char* inp_mat, const unsigned char* c_terms);
    unsigned int (*gf256mat_solve_linear_eq_64x64_impl)(unsigned long handle, unsigned char* sol, const unsigned char* inp_mat, const unsigned char* c_terms);
    unsigned int (*gf256mat_solve_linear_eq_96x96_impl)(unsigned long handle, unsigned char* sol, const unsigned char* inp_mat, const unsigned char* c_terms);

    unsigned char* _sk;
    unsigned char* _pk;
    unsigned char* signature;
    unsigned char* msg;
    unsigned int mlen;
    unsigned int mlen_len;
    unsigned char* msg_hash;
    unsigned char* _hash_buffer;

    unsigned char hash_of_sk[32];
    unsigned char hash_of_pk[32];
    unsigned char hash_of_sig[32];

    char STR_CRYPTO_ALGNAME[100];

    unsigned int set_display_debuginfo;
    unsigned int set_no_salt;

    unsigned char rnd_seed[48];
    unsigned char personalization_string[48];

    rb_AES256_CTR_DRBG_struct  DRBG_ctx;

    unsigned long dili_handle;
    unsigned long falo_handle;
    unsigned long sphi_handle;
    unsigned long secp_handle;
}RB_CORE_HANDLE;

int rb_crypto_generate_keypair(unsigned long handle,unsigned char *pk, unsigned char *sk);

int rb_crypto_sign(unsigned long handle, unsigned char *sm, unsigned int *smlen,
             unsigned char *digest_hash, unsigned int digest_hash_len,
             unsigned char *sk);



#ifdef  __cplusplus
}
#endif

#endif /* rb__rainbow_handle_h */
