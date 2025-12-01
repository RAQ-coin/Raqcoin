#ifndef rb__P_MATRIX_OP_H_
#define rb__P_MATRIX_OP_H_


#include "rb_core_config.h"

#ifdef  __cplusplus
extern  "C" {
#endif


static inline unsigned int rb_idx_of_trimat( unsigned int i_row , unsigned int j_col , unsigned int dim )
{
    return (dim + dim - i_row + 1 )*i_row/2 + j_col - i_row;
}
static inline unsigned int rb_idx_of_2trimat( unsigned int i_row , unsigned int j_col , unsigned int n_var )
{
   if( i_row > j_col ) return rb_idx_of_trimat(j_col,i_row,n_var);
   else return rb_idx_of_trimat(i_row,j_col,n_var);
}

void rb_UpperTrianglize(unsigned long handle, unsigned char * btriC , const unsigned char * bA , unsigned int Awidth, unsigned int size_batch );
void rb_batch_trimat_madd_gf16(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch );
void rb_batch_trimat_madd_gf256(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch );

void rb_batch_trimatTr_madd_gf16(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch );

void rb_batch_trimatTr_madd_gf256(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch );

void rb_batch_2trimat_madd_gf16(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch );

void rb_batch_2trimat_madd_gf256(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch );

void rb_batch_matTr_madd_gf16(unsigned long handle, unsigned char * bC ,
        const unsigned char* A_to_tr , unsigned int Aheight, unsigned int size_Acolvec, unsigned int Awidth,
        const unsigned char* bB, unsigned int Bwidth, unsigned int size_batch );

void rb_batch_matTr_madd_gf256(unsigned long handle, unsigned char * bC ,
        const unsigned char* A_to_tr , unsigned int Aheight, unsigned int size_Acolvec, unsigned int Awidth,
        const unsigned char* bB, unsigned int Bwidth, unsigned int size_batch );
void rb_batch_bmatTr_madd_gf16(unsigned long handle, unsigned char *bC , const unsigned char *bA_to_tr, unsigned int Awidth_before_tr,
        const unsigned char *B, unsigned int Bheight, unsigned int size_Bcolvec, unsigned int Bwidth, unsigned int size_batch );
void rb_batch_bmatTr_madd_gf256(unsigned long handle, unsigned char *bC , const unsigned char *bA_to_tr, unsigned int Awidth_before_tr,
        const unsigned char *B, unsigned int Bheight, unsigned int size_Bcolvec, unsigned int Bwidth, unsigned int size_batch );
void rb_batch_mat_madd_gf16(unsigned long handle, unsigned char * bC , const unsigned char* bA , unsigned int Aheight,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch );

void rb_batch_mat_madd_gf256(unsigned long handle, unsigned char * bC , const unsigned char* bA , unsigned int Aheight,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch );
void rb_batch_quad_trimat_eval_gf16(unsigned long handle, unsigned char * y, const unsigned char * trimat, const unsigned char * x, unsigned int dim , unsigned int size_batch );

void rb_batch_quad_trimat_eval_gf256(unsigned long handle, unsigned char * y, const unsigned char * trimat, const unsigned char * x, unsigned int dim , unsigned int size_batch );


void rb_batch_quad_recmat_eval_gf16(unsigned long handle, unsigned char * z, const unsigned char * y, unsigned int dim_y,
        const unsigned char * mat, const unsigned char * x, unsigned int dim_x , unsigned int size_batch );

void rb_batch_quad_recmat_eval_gf256(unsigned long handle, unsigned char * z, const unsigned char * y, unsigned int dim_y,
        const unsigned char * mat, const unsigned char * x, unsigned int dim_x , unsigned int size_batch );







#ifdef  __cplusplus
}
#endif


#endif 

