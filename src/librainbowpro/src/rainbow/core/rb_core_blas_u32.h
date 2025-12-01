#ifndef rb__BLAS_U32_H_
#define rb__BLAS_U32_H_
#include "rb_core_config.h"
#include "rb_core_gf16.h"

#include <string.h>
#include <stdint.h>

static inline void rb__gf256v_add_u64(uint8_t* accu_b, const uint8_t* a, unsigned _num_byte) 
{
    while (_num_byte >= 8) {
        uint64_t* a64 = (uint64_t*)a;
        uint64_t* b64 = (uint64_t*)accu_b;
        b64[0] ^= a64[0];
        a += 8;
        accu_b += 8;
        _num_byte -= 8;
    }
    for (unsigned i = 0; i < _num_byte; i++) accu_b[i] ^= a[i];
}
/////////////////////////////////////////////////////////
//   uint64_t  somewhat bitslice  library
/////////////////////////////////////////////////////////

static uint64_t _0x33 = 0x3333333333333333ULL; /// 3: 0011
static uint64_t _0x55 = 0x5555555555555555ULL; /// 5: 0101
static uint64_t _0xaa = 0xaaaaaaaaaaaaaaaaULL; /// a: 1010
//static uint64_t _0xcc = 0xccccccccccccccccULL; /// c: 1100
static uint64_t _0x0f = 0x0f0f0f0f0f0f0f0fULL; /// f: 1111
static uint64_t _0xf0 = 0xf0f0f0f0f0f0f0f0ULL; /// f: 1111
static inline uint64_t rb_gf4_mul_2_u64(uint64_t a)
{
    uint64_t bit0 = a & _0x55;
    uint64_t bit1 = a & _0xaa;
    return (bit0 << 1) ^ bit1 ^ (bit1 >> 1);
}
static inline uint64_t rb_gf4_mul_u64(uint64_t a, unsigned char b)
{
    uint64_t r = a * ((uint64_t)(b & 1));
    return r ^ (rb_gf4_mul_2_u64(a) * ((uint64_t)(b >> 1)));
}
static inline uint64_t rb_gf16_mul_u64(uint64_t a, unsigned char b)
{
    uint64_t a0 = a & _0x33;
    uint64_t a1 = (a >> 2) & _0x33;
    unsigned char b0 = b & 3;
    unsigned char b1 = (b >> 2);

    uint64_t a0b0 = rb_gf4_mul_u64(a0, b0);
    uint64_t a1b1 = rb_gf4_mul_u64(a1, b1);
    uint64_t a0b1_a1b0 = rb_gf4_mul_u64(a0 ^ a1, b0 ^ b1) ^ a0b0 ^ a1b1;
    uint64_t a1b1_x2 = rb_gf4_mul_2_u64(a1b1);
    return ((a0b1_a1b0 ^ a1b1) << 2) ^ a0b0 ^ a1b1_x2;
}

static inline void rb__gf16v_madd_u64(uint8_t* accu_c, const uint8_t* a, uint8_t b, unsigned _num_byte) 
{
    unsigned i = 0;
    for (; i + 8 <= _num_byte; i += 8) {
        uint64_t* a64 = (uint64_t*)&a[i];
        uint64_t* c64 = (uint64_t*)&accu_c[i];
        c64[0] ^= rb_gf16_mul_u64(a64[0], b);
    }
    if (i >= _num_byte) return;

    uint64_t temp;
    uint8_t* ptr_p = (uint8_t*)&temp;
    for (unsigned j = 0; j < _num_byte - i; j++) ptr_p[j] = a[i + j];
    temp = rb_gf16_mul_u64(temp, b);
    for (unsigned j = 0; j < _num_byte - i; j++) accu_c[i + j] ^= ptr_p[j];
}

static inline void rb__gf256v_add_u32(uint8_t *accu_b, const uint8_t *a, unsigned int _num_byte)
{
    if((accu_b == NULL) || (a == NULL) || (_num_byte == 0))
        return;

    unsigned int n_u32 = _num_byte >> 2;
    for (unsigned int i = 0; i < n_u32; i++) 
    {
      uint32_t bx;
      uint32_t ax;
      memcpy(&bx,accu_b+4*i,4);
      memcpy(&ax,a+4*i,4);
      bx ^= ax;
      memcpy(accu_b+4*i,&bx,4);
    }

    unsigned int rem = _num_byte & 3;
    if( !rem ) return;
    a += (n_u32 << 2);
    accu_b += (n_u32 << 2);
    for (unsigned int i = 0; i < rem; i++) 
        accu_b[i] ^= a[i];
}

static inline void rb__gf256v_conditional_add_u32(uint8_t *accu_b, uint8_t condition, const uint8_t *a, unsigned int _num_byte) {
    uint32_t pr_u32 = ((uint32_t) 0) - ((uint32_t) condition);
    uint8_t pr_u8 = pr_u32 & 0xff;

    unsigned int n_u32 = _num_byte >> 2;
    for (unsigned int i = 0; i < n_u32; i++) {
      uint32_t bx;
      uint32_t ax;
      memcpy(&bx,accu_b+4*i,4);
      memcpy(&ax,a+4*i,4);
      bx ^= ax&pr_u32;
      memcpy(accu_b+4*i,&bx,4);
    }

    unsigned int rem = _num_byte & 3;
    if( !rem ) return;
    a += (n_u32 << 2);
    accu_b += (n_u32 << 2);
    for (unsigned int i = 0; i < rem; i++) accu_b[i] ^= (a[i] & pr_u8);
}

///////////////////////////////////////////////////

static inline void rb__gf16v_mul_scalar_u32(uint8_t *a, uint8_t gf16_b, unsigned int _num_byte)
{

    typedef union tmp_32 {
        uint8_t u8[4];
        uint32_t u32;
    }tmp_32_U;

    tmp_32_U t = { 0 };

    unsigned int n_u32 = _num_byte >> 2;
    for (unsigned int i = 0; i < n_u32; i++) {
      uint32_t ax;
      memcpy(&ax,a+4*i,4);
      ax = rb_gf16v_mul_u32(ax, gf16_b);
      memcpy(a+4*i,&ax,4);
    }

    unsigned int  rem = _num_byte & 3;
    if( !rem ) return;
   
    a += (n_u32 << 2);
    for (unsigned int i = 0; i < rem; i++) t.u8[i] = a[i];
    t.u32 = rb_gf16v_mul_u32(t.u32, gf16_b);
    for (unsigned int i = 0; i < rem; i++) a[i] = t.u8[i];
}

static inline void rb__gf256v_mul_scalar_u32(uint8_t *a, uint8_t b, unsigned int _num_byte)
{

    typedef union tmp_32 {
        uint8_t u8[4];
        uint32_t u32;
    }tmp_32_U;

    tmp_32_U t = { 0 };

    unsigned int n_u32 = _num_byte >> 2;
    for (unsigned int i = 0; i < n_u32; i++) {
      uint32_t ax;
      memcpy(&ax,a+4*i,4);
      ax = rb_gf256v_mul_u32(ax, b);
      memcpy(a+4*i,&ax,4);
    }

    unsigned int rem = _num_byte & 3;
    if( !rem ) return;

    a += (n_u32 << 2);
    for (unsigned int i = 0; i < rem; i++) t.u8[i] = a[i];
    t.u32 = rb_gf256v_mul_u32(t.u32, b);
    for (unsigned int i = 0; i < rem; i++) a[i] = t.u8[i];
}

/////////////////////////////////////

static inline void rb__gf16v_madd_u32(uint8_t *accu_c, const uint8_t *a, uint8_t gf16_b, unsigned int _num_byte)
{
    typedef union tmp_32 {
        uint8_t u8[4];
        uint32_t u32;
    }tmp_32_U;

    tmp_32_U t = { 0 };

    unsigned int n_u32 = _num_byte >> 2;

    for (unsigned int i = 0; i < n_u32; i++) {
      uint32_t ax;
      uint32_t cx;
      memcpy(&ax,a+4*i,4);
      memcpy(&cx,accu_c+4*i,4);
      cx ^= rb_gf16v_mul_u32(ax, gf16_b);
      memcpy(accu_c+4*i,&cx,4);
    }

    unsigned int rem = _num_byte & 3;
    if( !rem ) return;

    accu_c += (n_u32 << 2);
    a += (n_u32 << 2);
    for (unsigned int i = 0; i < rem; i++) t.u8[i] = a[i];
    t.u32 = rb_gf16v_mul_u32(t.u32, gf16_b);
    for (unsigned int i = 0; i < rem; i++) accu_c[i] ^= t.u8[i];
}

static inline void rb__gf256v_madd_u32(uint8_t *accu_c, const uint8_t *a, uint8_t gf256_b, unsigned int _num_byte)
{
    typedef union tmp_32 {
        uint8_t u8[4];
        uint32_t u32;
    }tmp_32_U;

    tmp_32_U t = { 0 };

    unsigned int n_u32 = _num_byte >> 2;
    for (unsigned int i = 0; i < n_u32; i++) {
      uint32_t ax;
      uint32_t cx;
      memcpy(&ax,a+4*i,4);
      memcpy(&cx,accu_c+4*i,4);
      cx ^= rb_gf256v_mul_u32(ax, gf256_b);
      memcpy(accu_c+4*i,&cx,4);
    }

    unsigned int rem = _num_byte & 3;
    if( !rem ) return;
   
    accu_c += (n_u32 << 2);
    a += (n_u32 << 2);
    for (unsigned int i = 0; i < rem; i++) t.u8[i] = a[i];
    t.u32 = rb_gf256v_mul_u32(t.u32, gf256_b);
    for (unsigned int i = 0; i < rem; i++) accu_c[i] ^= t.u8[i];
}


#endif // rb__BLAS_U32_H_

