#ifndef _BLAS_U64_H_
#define _BLAS_U64_H_

#include <stdint.h>
#include <stdio.h>

#include "gf16.h"

#include "blas_config.h"
#include "assert.h"


#ifdef  __cplusplus
extern  "C" {
#endif



//////////////////////////////////////////
/// u64 library
/////////////////////////////////////////


static inline
void _gf16v_mul_scalar_u64( uint8_t * a, uint8_t b , unsigned _num_byte ) {
	unsigned i=0;
	for(;i+8<=_num_byte;i+=8) { uint64_t *a64 = (uint64_t*)&a[i]; *a64 = gf16_mul_u64(a64[0],b); }
	if( i >= _num_byte ) return;

	uint64_t temp;
	uint8_t * ptr_p  = (uint8_t *)&temp;
	for(unsigned j=0;j<_num_byte-i;j++) ptr_p[j] = a[i+j];
	temp = gf16_mul_u64( temp , b );
	for(unsigned j=0;j<_num_byte-i;j++) a[i+j] = ptr_p[j];
}

static inline
void _gf16v_madd_u64( uint8_t * accu_c, const uint8_t * a , uint8_t b, unsigned _num_byte ) {
	unsigned i=0;
	for(;i+8<=_num_byte;i+=8) {
		uint64_t *a64 = (uint64_t*)&a[i];
		uint64_t *c64 = (uint64_t*)&accu_c[i];
		c64[0] ^= gf16_mul_u64(a64[0],b);
	}
	if( i >= _num_byte ) return;

	uint64_t temp;
	uint8_t * ptr_p  = (uint8_t *)&temp;
	for(unsigned j=0;j<_num_byte-i;j++) ptr_p[j] = a[i+j];
	temp = gf16_mul_u64( temp , b );
	for(unsigned j=0;j<_num_byte-i;j++) accu_c[i+j] ^= ptr_p[j];
}



static inline
void _gf256v_add_u64( uint8_t * accu_b, const uint8_t * a , unsigned _num_byte ) {
	while( _num_byte >= 8 ) {
		uint64_t * a64 = (uint64_t *) a;
		uint64_t * b64 = (uint64_t *) accu_b;
		b64[0] ^= a64[0];
		a += 8;
		accu_b += 8;
		_num_byte -= 8;
	}
	for(unsigned i=0; i<_num_byte; i++) accu_b[i] ^= a[i];
}

static inline
void _gf256v_mul_scalar_u64( uint8_t *a, uint8_t b, unsigned _num_byte ) {
#if 0
	unsigned i=0;
	for(;i+8<=_num_byte;i+=8) { uint64_t *a64 = (uint64_t*)&a[i]; *a64 = gf256_mul_u64(a64[0],b); }
	for(;i<_num_byte;i++) a[i] = gf256_mul( a[i] , b );
#else
	unsigned i=0;
	for(;i+8<=_num_byte;i+=8) { uint64_t *a64 = (uint64_t*)&a[i]; *a64 = gf256_mul_u64(a64[0],b); }
	if( i >= _num_byte ) return;

	uint64_t temp;
	uint8_t * ptr_p  = (uint8_t *)&temp;
	for(unsigned j=0;j<_num_byte-i;j++) ptr_p[j] = a[i+j];
	temp = gf256_mul_u64( temp , b );
	for(unsigned j=0;j<_num_byte-i;j++) a[i+j] = ptr_p[j];
#endif
}

static inline
void _gf256v_madd_u64( uint8_t * accu_c, const uint8_t * a , uint8_t b, unsigned _num_byte ) {
	unsigned i=0;
	for(;i+8<=_num_byte;i+=8) {
		uint64_t *a64 = (uint64_t*)&a[i];
		uint64_t *c64 = (uint64_t*)&accu_c[i];
		c64[0] ^= gf256_mul_u64(a64[0],b);
	}
	if( i >= _num_byte ) return;

	uint64_t temp;
	uint8_t * ptr_p  = (uint8_t *)&temp;
	for(unsigned j=0;j<_num_byte-i;j++) ptr_p[j] = a[i+j];
	temp = gf256_mul_u64( temp , b );
	for(unsigned j=0;j<_num_byte-i;j++) accu_c[i+j] ^= ptr_p[j];
}

static inline
void _gf256v_m0x10_add_u64( uint8_t * accu_c, const uint8_t * a , unsigned _num_byte ) {
	while( _num_byte >= 8 ) {
		uint64_t * a64 = (uint64_t *) a;
		uint64_t * c64 = (uint64_t *) accu_c;
		c64[0] ^= gf256_mul_0x10_u64( a64[0] );
		a += 8;
		accu_c += 8;
		_num_byte -= 8;
	}
	if( 0 == _num_byte ) return;

	uint64_t temp;
	uint8_t * ptr_p  = (uint8_t *)&temp;
	for(unsigned i=0;i<_num_byte;i++) ptr_p[i] = a[i];
	temp = gf256_mul_0x10_u64( temp );
	for(unsigned i=0;i<_num_byte;i++) accu_c[i] ^= ptr_p[i];
}




#ifdef  __cplusplus
}
#endif



#endif

