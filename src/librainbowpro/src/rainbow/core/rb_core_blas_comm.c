#include "rb_core_rainbow_handle.h"
#include "rb_core_blas_comm.h"
#include "rb_core_blas.h"

#include <stdint.h>


void rb_gf256v_set_zero(unsigned long handle,uint8_t *b, unsigned int _num_byte)
{
    RB_CORE_HANDLE* HD = (RB_CORE_HANDLE*)handle;
    if(_num_byte)
        HD->gf256v_add(b, b, _num_byte);
}


