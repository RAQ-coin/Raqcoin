
#include "rb_core_config.h"
#include "rb_core_rainbow_handle.h"
#include "rb_core_blas_comm.h"
#include "rb_core_blas.h"

#include "rb_core_parallel_matrix_op.h"




void rb_UpperTrianglize(unsigned long handle, unsigned char * btriC , const unsigned char * bA , unsigned Awidth, unsigned size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned char * runningC = btriC;
    unsigned int Aheight = Awidth;
    for(unsigned int i=0;i<Aheight;i++) 
    {
        for(unsigned int j=0;j<i;j++) 
        {
            unsigned int idx = rb_idx_of_trimat(j,i,Aheight);
            HD->gf256v_add( btriC + idx*size_batch , bA + size_batch*(i*Awidth+j) , size_batch );
        }
        HD->gf256v_add( runningC , bA + size_batch*(i*Awidth+i) , size_batch*(Aheight-i) );
        runningC += size_batch*(Aheight-i);
    }
}




void rb_batch_trimat_madd_gf16(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int Awidth = Bheight;
    unsigned int Aheight = Awidth;
    for(unsigned int i=0;i<Aheight;i++) {
        for(unsigned int j=0; j<Bwidth;j++) {
            for(unsigned int k=0;k<Bheight;k++) {
                if(k<i) continue;
                HD->gfv_madd( bC , & btriA[ (k-i)*size_batch ] , HD->gfv_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
        btriA += (Aheight-i)*size_batch;
    }
}

void rb_batch_trimat_madd_gf256(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int Awidth = Bheight;
    unsigned int Aheight = Awidth;
    for(unsigned int i=0;i<Aheight;i++) {
        for(unsigned int j=0;j<Bwidth;j++) {
            for(unsigned int k=0;k<Bheight;k++) {
                if(k<i) continue;
                HD->gfv_madd( bC , & btriA[ (k-i)*size_batch ] , HD->gfv_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
        btriA += (Aheight-i)*size_batch;
    }
}





void rb_batch_trimatTr_madd_gf16(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int Aheight = Bheight;
    for(unsigned int i=0;i<Aheight;i++) {
        for(unsigned int j=0;j<Bwidth;j++) {
            for(unsigned int k=0;k<Bheight;k++) {
                if(i<k) continue;
                HD->gfv_madd( bC , & btriA[ size_batch*(rb_idx_of_trimat(k,i,Aheight)) ] , HD->gfv_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}

void rb_batch_trimatTr_madd_gf256(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int Aheight = Bheight;
    for(unsigned int i=0;i<Aheight;i++) {
        for(unsigned int j=0;j<Bwidth;j++) {
            for(unsigned int k=0;k<Bheight;k++) {
                if(i<k) continue;
                HD->gfv_madd( bC , & btriA[ size_batch*(rb_idx_of_trimat(k,i,Aheight)) ] , HD->gfv_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}




void rb_batch_2trimat_madd_gf16(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int Aheight = Bheight;
    for(unsigned int i=0;i<Aheight;i++) {
        for(unsigned int j=0;j<Bwidth;j++) {
            for(unsigned int k=0;k<Bheight;k++) {
                if(i==k) continue;
                HD->gfv_madd( bC , & btriA[ size_batch*(rb_idx_of_2trimat(i,k,Aheight)) ] , HD->gfv_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}

void rb_batch_2trimat_madd_gf256(unsigned long handle, unsigned char * bC , const unsigned char* btriA ,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int  size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int Aheight = Bheight;
    for(unsigned int i=0;i<Aheight;i++) {
        for(unsigned int j=0;j<Bwidth;j++) {
            for(unsigned int k=0;k<Bheight;k++) {
                if(i==k) continue;
                HD->gfv_madd( bC , & btriA[ size_batch*(rb_idx_of_2trimat(i,k,Aheight)) ] , HD->gfv_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}




void rb_batch_matTr_madd_gf16(unsigned long handle, unsigned char * bC , const unsigned char* A_to_tr , unsigned int Aheight, unsigned int size_Acolvec, unsigned int Awidth,
        const unsigned char* bB, unsigned int Bwidth, unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int Atr_height = Awidth;
    unsigned int Atr_width  = Aheight;
    for(unsigned int i=0;i<Atr_height;i++) {
        for(unsigned int j=0;j<Atr_width;j++) {
            HD->gfv_madd( bC , & bB[ j*Bwidth*size_batch ] , HD->gfv_get_ele( &A_to_tr[size_Acolvec*i] , j ) , size_batch*Bwidth );
        }
        bC += size_batch*Bwidth;
    }
}

void rb_batch_matTr_madd_gf256(unsigned long handle, unsigned char * bC , const unsigned char* A_to_tr , unsigned int Aheight, unsigned int size_Acolvec, unsigned int  Awidth,
        const unsigned char* bB, unsigned int Bwidth, unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int Atr_height = Awidth;
    unsigned int Atr_width  = Aheight;
    for(unsigned int i=0;i<Atr_height;i++) {
        for(unsigned int j=0;j<Atr_width;j++) {
            HD->gfv_madd( bC , & bB[ j*Bwidth*size_batch ] , HD->gfv_get_ele( &A_to_tr[size_Acolvec*i] , j ) , size_batch*Bwidth );
        }
        bC += size_batch*Bwidth;
    }
}




void rb_batch_bmatTr_madd_gf16(unsigned long handle, unsigned char *bC , const unsigned char *bA_to_tr, unsigned int Awidth_before_tr,
        const unsigned char *B, unsigned int Bheight, unsigned int size_Bcolvec, unsigned int Bwidth, unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    const unsigned char *bA = bA_to_tr;
    unsigned int Aheight = Awidth_before_tr;
    for(unsigned int i=0;i<Aheight;i++) {
        for(unsigned int j=0;j<Bwidth;j++) {
            for(unsigned int k=0;k<Bheight;k++) {
                HD->gfv_madd( bC , & bA[ size_batch*(i+k*Aheight) ] , HD->gfv_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}

void rb_batch_bmatTr_madd_gf256(unsigned long handle, unsigned char *bC , const unsigned char *bA_to_tr, unsigned int Awidth_before_tr,
        const unsigned char *B, unsigned int Bheight, unsigned int  size_Bcolvec, unsigned int Bwidth, unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    const unsigned char *bA = bA_to_tr;
    unsigned int Aheight = Awidth_before_tr;
    for(unsigned int i=0;i<Aheight;i++) {
        for(unsigned int j=0;j<Bwidth;j++) {
            for(unsigned int k=0;k<Bheight;k++) {
                HD->gfv_madd( bC , & bA[ size_batch*(i+k*Aheight) ] , HD->gfv_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
    }
}





void rb_batch_mat_madd_gf16(unsigned long handle, unsigned char * bC , const unsigned char* bA , unsigned int Aheight,
        const unsigned char* B , unsigned int Bheight, unsigned int size_Bcolvec , unsigned int Bwidth, unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int Awidth = Bheight;
    for(unsigned int i=0;i<Aheight;i++) {
        for(unsigned int j=0;j<Bwidth;j++) {
            for(unsigned int k=0;k<Bheight;k++) {
                HD->gfv_madd( bC , & bA[ k*size_batch ] , HD->gfv_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
        bA += (Awidth)*size_batch;
    }
}

void rb_batch_mat_madd_gf256(unsigned long handle, unsigned char * bC , const unsigned char* bA , unsigned int Aheight,
        const unsigned char* B , unsigned int Bheight, unsigned int  size_Bcolvec , unsigned int Bwidth, unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    unsigned int Awidth = Bheight;
    for(unsigned int i=0;i<Aheight;i++) {
        for(unsigned int j=0;j<Bwidth;j++) {
            for(unsigned int k=0;k<Bheight;k++) {
                HD->gfv_madd( bC , & bA[ k*size_batch ] , HD->gfv_get_ele( &B[j*size_Bcolvec] , k ) , size_batch );
            }
            bC += size_batch;
        }
        bA += (Awidth)*size_batch;
    }
}




void rb_batch_quad_trimat_eval_gf16(unsigned long handle, unsigned char * y, const unsigned char * trimat, const unsigned char * x, unsigned int dim , unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    unsigned char tmp[256];

    unsigned char _x[256];
    for(unsigned int i=0;i<dim;i++) _x[i] = HD->gfv_get_ele( x , i );

    rb_gf256v_set_zero(handle, y , size_batch );
    for(unsigned int i=0;i<dim;i++) {
        rb_gf256v_set_zero(handle, tmp , size_batch );
        for(unsigned int j=i;j<dim;j++) {
            HD->gfv_madd( tmp , trimat , _x[j] , size_batch );
           trimat += size_batch;
        }
        HD->gfv_madd( y , tmp , _x[i] , size_batch );
    }
}

void rb_batch_quad_trimat_eval_gf256(unsigned long handle, unsigned char * y, const unsigned char * trimat, const unsigned char * x, unsigned int dim , unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    unsigned char tmp[256];

    unsigned char _x[256];
    for(unsigned int i=0;i<dim;i++) _x[i] = HD->gfv_get_ele( x , i );

    rb_gf256v_set_zero(handle, y , size_batch );
    for(unsigned int i=0;i<dim;i++) {
        rb_gf256v_set_zero(handle, tmp , size_batch );
        for(unsigned int j=i;j<dim;j++) {
            HD->gfv_madd( tmp , trimat , _x[j] , size_batch );
           trimat += size_batch;
        }
        HD->gfv_madd( y , tmp , _x[i] , size_batch );
    }
}







void rb_batch_quad_recmat_eval_gf16(unsigned long handle, unsigned char * z, const unsigned char * y, unsigned int dim_y, const unsigned char * mat,
        const unsigned char * x, unsigned int dim_x , unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    unsigned char tmp[128];

    unsigned char _x[128];
    for(unsigned int i=0;i<dim_x;i++) _x[i] = HD->gfv_get_ele( x , i );
    unsigned char _y[128];
    for(unsigned int i=0;i<dim_y;i++) _y[i] = HD->gfv_get_ele( y , i );

    rb_gf256v_set_zero(handle, z , size_batch );
    for(unsigned int i=0;i<dim_y;i++) {
        rb_gf256v_set_zero(handle, tmp , size_batch );
        for(unsigned int j=0;j<dim_x;j++) {
            HD->gfv_madd( tmp , mat , _x[j] , size_batch );
           mat += size_batch;
        }
        HD->gfv_madd( z , tmp , _y[i] , size_batch );
    }
}


void rb_batch_quad_recmat_eval_gf256(unsigned long handle, unsigned char * z, const unsigned char * y, unsigned int dim_y, const unsigned char * mat,
        const unsigned char * x, unsigned int dim_x , unsigned int size_batch )
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;

    unsigned char tmp[128];

     unsigned char _x[128];
    for(unsigned int i=0;i<dim_x;i++) _x[i] = (unsigned char)HD->gfv_get_ele( x , i );
     unsigned char _y[128];
    for(unsigned int i=0;i<dim_y;i++) _y[i] = (unsigned char)HD->gfv_get_ele( y , i );

    rb_gf256v_set_zero(handle, z , size_batch );
    for(unsigned int i=0;i<dim_y;i++) {
        rb_gf256v_set_zero(handle, tmp , size_batch );
        for(unsigned int j=0;j<dim_x;j++) {
            HD->gfv_madd( tmp , mat , _x[j] , size_batch );
           mat += size_batch;
        }
        HD->gfv_madd( z , tmp , _y[i] , size_batch );
    }
}





