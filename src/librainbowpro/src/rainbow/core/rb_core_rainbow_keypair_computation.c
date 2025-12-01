
#include "rb_core_config.h"
#include "rb_core_RAINBOW_PRO_API.h"
#include "rb_core_rainbow_keypair.h"
#include "rb_core_rainbow_keypair_computation.h"

#include "rb_core_blas_comm.h"
#include "rb_core_blas.h"



#include "rb_core_parallel_matrix_op.h"


#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#if defined(_BLAS_SSE_) || defined(_BLAS_AVX2_)
#include "rainbow_keypair_computation_simd.h"

#define calculate_F_from_Q_impl        calculate_F_from_Q_simd
#define calculate_Q_from_F_cyclic_impl calculate_Q_from_F_cyclic_simd

#else
#define calculate_F_from_Q_impl         rb_calculate_F_from_Q_ref        
#define calculate_Q_from_F_cyclic_impl  rb_calculate_Q_from_F_cyclic_ref 



/////////////////////////////////////////////////////
static void rb_calculate_F_from_Q_ref_common(unsigned long handle, rb_sk_t* Fs, const rb_sk_t* Qs, const rb_sk_t* Ts)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
 
    if (Fs->l1_F1_len && Qs->l1_F1_len)
        memcpy(Fs->l1_F1, Qs->l1_F1, Qs->l1_F1_len);

  
    if (Fs->l1_F2_len && Qs->l1_F2_len)
        memcpy(Fs->l1_F2, Qs->l1_F2, Qs->l1_F2_len);

    if (Fs->l1_F2_len && Qs->l1_F1_len && Ts->t1_len)
        HD->batch_2trimat_madd(handle, Fs->l1_F2, Qs->l1_F1, Ts->t1, HD->P_V1, HD->P_V1_BYTE, HD->P_O1, HD->P_O1_BYTE);

  
    if (Fs->l2_F1_len && Qs->l2_F1_len)
        memcpy(Fs->l2_F1, Qs->l2_F1, Qs->l2_F1_len);     


   
    if (Fs->l2_F2_len && Qs->l2_F2_len)
        memcpy(Fs->l2_F2, Qs->l2_F2, Qs->l2_F2_len);

    if (Fs->l2_F2_len && Qs->l2_F1_len && Ts->t1_len)
        HD->batch_trimat_madd(handle, Fs->l2_F2, Qs->l2_F1, Ts->t1, HD->P_V1, HD->P_V1_BYTE, HD->P_O1, HD->P_O2_BYTE);   

    unsigned int tempQ_len = 0;
    if(HD->P_O2==0)
        tempQ_len = HD->P_O1 * HD->P_O1 * HD->P_O1_BYTE;
    else
        tempQ_len = HD->P_O1 * HD->P_O1 * HD->P_O2_BYTE;

    unsigned char* tempQ = (unsigned char*)rb_safe_malloc(tempQ_len);
 

    if (tempQ_len && Ts->t1_len && Fs->l2_F2_len)
        HD->batch_matTr_madd(handle, tempQ, Ts->t1, HD->P_V1, HD->P_V1_BYTE, HD->P_O1, Fs->l2_F2, HD->P_O1, HD->P_O2_BYTE);   

    if (Fs->l2_F5_len && Qs->l2_F5_len)
        memcpy(Fs->l2_F5, Qs->l2_F5, Qs->l2_F5_len);                      // F5

    if (Fs->l2_F5_len && tempQ_len)
        rb_UpperTrianglize(handle,Fs->l2_F5, tempQ, HD->P_O1, HD->P_O2_BYTE);                               

    if (tempQ_len)
        memset(tempQ, 0, tempQ_len);

    if (Fs->l2_F2_len && Qs->l2_F1_len && Ts->t1_len)
        HD->batch_trimatTr_madd(handle, Fs->l2_F2, Qs->l2_F1, Ts->t1, HD->P_V1, HD->P_V1_BYTE, HD->P_O1, HD->P_O2_BYTE);  

  
    if (Fs->l2_F3_len && Qs->l2_F3_len)
        memcpy(Fs->l2_F3, Qs->l2_F3, Qs->l2_F3_len);

    if (Fs->l2_F3_len && Qs->l2_F1_len && Ts->t4_len)
        HD->batch_2trimat_madd(handle, Fs->l2_F3, Qs->l2_F1, Ts->t4, HD->P_V1, HD->P_V1_BYTE, HD->P_O2, HD->P_O2_BYTE);   

    if (Fs->l2_F3_len && Qs->l2_F2_len && Ts->t3_len)
        HD->batch_mat_madd(handle, Fs->l2_F3, Qs->l2_F2, HD->P_V1, Ts->t3, HD->P_O1, HD->P_O1_BYTE, HD->P_O2, HD->P_O2_BYTE);  

    

    if (Fs->l2_F6_len && Qs->l2_F6_len)
        memcpy(Fs->l2_F6, Qs->l2_F6, Qs->l2_F6_len);

    if (Fs->l2_F6_len && Ts->t1_len && Fs->l2_F3_len)
        HD->batch_matTr_madd(handle, Fs->l2_F6, Ts->t1, HD->P_V1, HD->P_V1_BYTE, HD->P_O1, Fs->l2_F3, HD->P_O2, HD->P_O2_BYTE);  

    if (Fs->l2_F6_len && Qs->l2_F5_len && Ts->t3_len)
        HD->batch_2trimat_madd(handle, Fs->l2_F6, Qs->l2_F5, Ts->t3, HD->P_O1, HD->P_O1_BYTE, HD->P_O2, HD->P_O2_BYTE);     

    if (Fs->l2_F6_len && Qs->l2_F2_len && Ts->t4_len)
        HD->batch_bmatTr_madd(handle, Fs->l2_F6, Qs->l2_F2, HD->P_O1, Ts->t4, HD->P_V1, HD->P_V1_BYTE, HD->P_O2, HD->P_O2_BYTE);

    rb_safe_free(tempQ);
}
static void rb_calculate_F_from_Q_ref(unsigned long handle, rb_sk_t * Fs , const rb_sk_t * Qs , const rb_sk_t * Ts)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    rb_calculate_F_from_Q_ref_common(handle,Fs, Qs, Ts);
}


static void rb_calculate_Q_from_F_cyclic_ref_common(unsigned long handle, rb_cpk_t* Qs, const rb_sk_t* Fs, const rb_sk_t* Ts)
{

    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
 
    const unsigned char* t2 = Ts->t4;
    unsigned int t2_len = Ts->t4_len;

   
    unsigned int buffer_F2_len = 0;
    if(HD->P_O2==0)
        buffer_F2_len = HD->P_O1_BYTE * HD->P_V1 * HD->P_O1;
    else
        buffer_F2_len = HD->P_O2_BYTE * HD->P_V1 * HD->P_O2;
    unsigned char* buffer_F2 = (unsigned char*)rb_safe_malloc(buffer_F2_len);
  
    
    if (buffer_F2_len)
        memcpy(buffer_F2, Fs->l1_F2, Fs->l1_F2_len);
    if (buffer_F2_len && Fs->l1_F1_len && Ts->t1_len)
        HD->batch_trimat_madd(handle, buffer_F2, Fs->l1_F1, Ts->t1, HD->P_V1, HD->P_V1_BYTE, HD->P_O1, HD->P_O1_BYTE);   


    unsigned int buffer_F3_len = 0;
    if (HD->P_O2 == 0)
        buffer_F3_len = HD->P_O1_BYTE * HD->P_V1 * HD->P_O1;
    else
        buffer_F3_len = HD->P_O2_BYTE * HD->P_V1 * HD->P_O2;


    unsigned char* buffer_F3 = (unsigned char*)rb_safe_malloc(buffer_F3_len);
   

    if (buffer_F3_len && Ts->t1_len && buffer_F2_len)
        HD->batch_matTr_madd(handle, buffer_F3, Ts->t1, HD->P_V1, HD->P_V1_BYTE, HD->P_O1, buffer_F2, HD->P_O1, HD->P_O1_BYTE); 
    if (Qs->l1_Q5_len)
        memset(Qs->l1_Q5, 0, HD->P_O1_BYTE * N_TRIANGLE_TERMS(HD->P_O1));
    if (Qs->l1_Q5_len && buffer_F3_len)
        rb_UpperTrianglize(handle,Qs->l1_Q5, buffer_F3, HD->P_O1, HD->P_O1_BYTE);                 


    if (Qs->l1_Q3_len)
        memset(Qs->l1_Q3, 0, Qs->l1_Q3_len);
    if (Qs->l1_Q6_len)
        memset(Qs->l1_Q6, 0, Qs->l1_Q6_len);
    if (Qs->l1_Q9_len)
        memset(Qs->l1_Q9, 0, Qs->l1_Q9_len);

    if (Qs->l1_Q3_len && Fs->l1_F1_len && t2_len)
        HD->batch_trimat_madd(handle, Qs->l1_Q3, Fs->l1_F1, t2, HD->P_V1, HD->P_V1_BYTE, HD->P_O2, HD->P_O1_BYTE);      
    if (Qs->l1_Q3_len && Fs->l1_F2_len && Ts->t3_len)
        HD->batch_mat_madd(handle, Qs->l1_Q3, Fs->l1_F2, HD->P_V1, Ts->t3, HD->P_O1, HD->P_O1_BYTE, HD->P_O2, HD->P_O1_BYTE);  

    if (buffer_F3_len)
        memset(buffer_F3, 0, buffer_F3_len);
    if (buffer_F3_len && t2_len && Qs->l1_Q3_len)
        HD->batch_matTr_madd(handle, buffer_F3, t2, HD->P_V1, HD->P_V1_BYTE, HD->P_O2, Qs->l1_Q3, HD->P_O2, HD->P_O1_BYTE);    
    if (Qs->l1_Q9_len && buffer_F3_len)
        rb_UpperTrianglize(handle,Qs->l1_Q9, buffer_F3, HD->P_O2, HD->P_O1_BYTE);                            

    if (Qs->l1_Q3_len && Fs->l1_F1_len && t2_len)
        HD->batch_trimatTr_madd(handle, Qs->l1_Q3, Fs->l1_F1, t2, HD->P_V1, HD->P_V1_BYTE, HD->P_O2, HD->P_O1_BYTE);      

    if (Qs->l1_Q6_len && Fs->l1_F2_len && t2_len)
        HD->batch_bmatTr_madd(handle, Qs->l1_Q6, Fs->l1_F2, HD->P_O1, t2, HD->P_V1, HD->P_V1_BYTE, HD->P_O2, HD->P_O1_BYTE);      

    if (Qs->l1_Q6_len && Ts->t1_len && Qs->l1_Q3_len)
        HD->batch_matTr_madd(handle, Qs->l1_Q6, Ts->t1, HD->P_V1, HD->P_V1_BYTE, HD->P_O1, Qs->l1_Q3, HD->P_O2, HD->P_O1_BYTE);  

    if (buffer_F3_len < HD->P_O2_BYTE * HD->P_V1 * HD->P_O2)
    {
        printf("error: incorrect buffer size./n");
    }
    memcpy(buffer_F3, Fs->l2_F3, Fs->l2_F3_len);

    if (buffer_F3_len && Fs->l2_F1_len && t2_len)
        HD->batch_trimat_madd(handle, buffer_F3, Fs->l2_F1, t2, HD->P_V1, HD->P_V1_BYTE, HD->P_O2, HD->P_O2_BYTE);      
    if (buffer_F3_len && Fs->l2_F2_len && Ts->t3_len)
        HD->batch_mat_madd(handle, buffer_F3, Fs->l2_F2, HD->P_V1, Ts->t3, HD->P_O1, HD->P_O1_BYTE, HD->P_O2, HD->P_O2_BYTE); 

    if (buffer_F2_len < HD->P_O2_BYTE * HD->P_V1 * HD->P_O2)
    {
        printf("error: incorrect buffer size./n");
    }

    memset(buffer_F2, 0, buffer_F2_len);
    if (buffer_F2_len && t2_len && buffer_F3_len)
        HD->batch_matTr_madd(handle, buffer_F2, t2, HD->P_V1, HD->P_V1_BYTE, HD->P_O2, buffer_F3, HD->P_O2, HD->P_O2_BYTE); 

    if (buffer_F3_len < HD->P_O2_BYTE * HD->P_O1 * HD->P_O2)
    {
        printf("error: incorrect buffer size./n");
    }

    memcpy(buffer_F3, Fs->l2_F6, Fs->l2_F6_len);
    if (buffer_F3_len && Fs->l2_F5_len && Ts->t3_len)
        HD->batch_trimat_madd(handle, buffer_F3, Fs->l2_F5, Ts->t3, HD->P_O1, HD->P_O1_BYTE, HD->P_O2, HD->P_O2_BYTE);    
    if (buffer_F2_len && Ts->t3_len && buffer_F3_len)
        HD->batch_matTr_madd(handle, buffer_F2, Ts->t3, HD->P_O1, HD->P_O1_BYTE, HD->P_O2, buffer_F3, HD->P_O2, HD->P_O2_BYTE); 
    memset(Qs->l2_Q9, 0, Qs->l2_Q9_len);

    if (Qs->l2_Q9_len && buffer_F2_len)
        rb_UpperTrianglize(handle,Qs->l2_Q9, buffer_F2, HD->P_O2, HD->P_O2_BYTE);                           

    if (buffer_F2_len)
        memset(buffer_F2, 0, buffer_F2_len);
    if (buffer_F3_len)
        memset(buffer_F3, 0, buffer_F3_len);

    rb_safe_free(buffer_F2);
    rb_safe_free(buffer_F3);
}

static void rb_calculate_Q_from_F_cyclic_ref(unsigned long handle, rb_cpk_t * Qs, const rb_sk_t * Fs , const rb_sk_t * Ts)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    rb_calculate_Q_from_F_cyclic_ref_common(handle,Qs, Fs, Ts);
}



#endif   





void rb_calculate_F_from_Q(unsigned long handle, rb_sk_t * Fs , const rb_sk_t * Qs , const rb_sk_t * Ts)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    calculate_F_from_Q_impl(handle, Fs , Qs , Ts);
}




void rb_calculate_Q_from_F_cyclic(unsigned long handle, rb_cpk_t * Qs, const rb_sk_t * Fs , const rb_sk_t * Ts)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    calculate_Q_from_F_cyclic_impl(handle, Qs , Fs , Ts);
}


