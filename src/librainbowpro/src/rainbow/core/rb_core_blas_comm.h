#ifndef rb__BLAS_COMM_H_
#define rb__BLAS_COMM_H_

#include <stdint.h>
#include "rb_core_config.h"


static inline uint8_t rb_gf16v_get_ele(const uint8_t* a, unsigned int i)
{
    uint8_t r = a[i >> 1];
    r = (i & 1) ? (r >> 4) : (r & 0xf);
    return r;
}
static inline uint8_t rb_gf16v_set_ele(uint8_t* a, unsigned int i, uint8_t v) {
    unsigned char m = (i & 1) ? 0xf : 0xf0;
    a[i >> 1] &= m;
    m = (i & 1) ? v << 4 : v & 0xf;
    a[i >> 1] |= m;
    return v;
}
static inline uint8_t rb_gf256v_get_ele(const uint8_t *a, unsigned int i) 
{ 
    return a[i]; 
}

void rb_gf256v_set_zero(unsigned long handle, uint8_t* b, unsigned int _num_byte);
unsigned int rb_gf16mat_gauss_elim_8x16_ref(unsigned long handle, uint8_t* mat);
unsigned int rb_gf16mat_gauss_elim_16x32_ref(unsigned long handle, uint8_t* mat);
unsigned int rb_gf256mat_gauss_elim_ref(unsigned long handle, uint8_t* mat, unsigned int h, unsigned int w);


#endif  

