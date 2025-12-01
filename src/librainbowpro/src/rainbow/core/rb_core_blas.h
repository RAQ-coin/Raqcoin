#ifndef rb__BLAS_H_
#define rb__BLAS_H_

#include "rb_core_config.h"
#include "rb_core_blas_u32.h"

#ifdef RB_SIMD_X86
#include "rb_core_blas_avx2.h"
#include "rb_core_blas_sse.h"
#endif

#ifdef  __cplusplus
extern  "C" {
#endif




#ifndef rb__BLAS_UNIT_LEN_
#define rb__BLAS_UNIT_LEN_ 4
#endif



#include "rb_core_blas_comm.h"
#include "rb_core_blas_matrix.h"
#include "rb_core_random.h"

static inline void rb_gf256v_rand(uint8_t* a, unsigned int _num_byte)
{
	rb_getRandBytes(a, _num_byte);
}


#ifdef  __cplusplus
}
#endif


#endif // rb__BLAS_H_

