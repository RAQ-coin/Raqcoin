#ifndef _BLAS_SSE_H_
#define _BLAS_SSE_H_

#include "rb_core_config.h"

#ifdef RB_SIMD_X86

#include "emmintrin.h"
#include "tmmintrin.h"
#include "rb_core_gf16.h"
#include "rb_core_gf16_sse.h"
#include "assert.h"


#ifdef  __cplusplus
extern  "C" {
#endif



static inline
__m128i _load_xmm(const uint8_t* a, unsigned _num_byte) {
	ALIGN(32) uint8_t temp[32];
	assert(16 >= _num_byte);
	assert(0 < _num_byte);
	for (unsigned i = 0; i < _num_byte; i++) temp[i] = a[i];
	return _mm_load_si128((__m128i*)temp);
}
static inline
void loadu_xmm( __m128i *xmm_a, const uint8_t *a, unsigned _num_byte ) {
	unsigned n_16 = (_num_byte>>4);
	unsigned n_16_rem = _num_byte&0xf;
	while( n_16-- ) {
		xmm_a[0] = _mm_loadu_si128( (__m128i*)(a) );
		xmm_a++;
		a += 16;
	}
	if( n_16_rem ) xmm_a[0] = _load_xmm( a , n_16_rem );
}

static inline
void _store_xmm(uint8_t* a, unsigned _num_byte, __m128i data) {
	ALIGN(32) uint8_t temp[32];
	assert(16 >= _num_byte);
	assert(0 < _num_byte);
	_mm_store_si128((__m128i*)temp, data);
	for (unsigned i = 0; i < _num_byte; i++) a[i] = temp[i];
}


static inline
void storeu_xmm( uint8_t *a , unsigned _num_byte , __m128i *xmm_a ) {
	unsigned n_16 = (_num_byte>>4);
	unsigned n_16_rem = _num_byte&0xf;
	while( n_16-- ) {
		_mm_storeu_si128( (__m128i*)a , xmm_a[0] );
		xmm_a++;
		a += 16;
	}
	if( n_16_rem ) _store_xmm( a , n_16_rem , xmm_a[0] );
}





static inline
void linearmap_8x8_accu_sse(uint8_t* accu_c, const uint8_t* a, __m128i ml, __m128i mh, __m128i mask, unsigned _num_byte) {
	unsigned n_16 = _num_byte >> 4;
	for (unsigned i = n_16; i > 0; i--) {
		__m128i inp = _mm_loadu_si128((__m128i*)(a));
		__m128i out = _mm_loadu_si128((__m128i*)(accu_c));
		__m128i r0 = linear_transform_8x8_128b(ml, mh, inp, mask);
		//r0 ^= out;
		r0 = _mm_xor_si128(r0, out);
		_mm_storeu_si128((__m128i*)(accu_c), r0);
		a += 16;
		accu_c += 16;
	}
	unsigned rem = _num_byte & 15;
	if (rem) {
		__m128i inp = _load_xmm(a, rem);
		__m128i out = _load_xmm(accu_c, rem);
		__m128i r0 = linear_transform_8x8_128b(ml, mh, inp, mask);
		//r0 ^= out;
		r0 = _mm_xor_si128(r0, out);
		_store_xmm(accu_c, rem, r0);
	}
}



static inline
void linearmap_8x8_accu_ymm(uint8_t* accu_c, const uint8_t* a, __m256i ml, __m256i mh, __m256i mask, unsigned _num_byte) {
	unsigned n_32 = _num_byte >> 5;
	for (unsigned i = 0; i < n_32; i++) {
		__m256i inp = _mm256_loadu_si256((__m256i*)(a + i * 32));
		__m256i out = _mm256_loadu_si256((__m256i*)(accu_c + i * 32));
		__m256i r0 = linear_transform_8x8_256b(ml, mh, inp, mask);
		//r0 ^= out;
		r0 = _mm256_xor_si256(r0, out);
		_mm256_storeu_si256((__m256i*)(accu_c + i * 32), r0);
	}
	unsigned rem = _num_byte & 31;
	if (rem) linearmap_8x8_accu_sse(accu_c + n_32 * 32, a + n_32 * 32, _mm256_castsi256_si128(ml), _mm256_castsi256_si128(mh), _mm256_castsi256_si128(mask), rem);
}
static inline
void gf256v_madd_avx2(uint8_t* accu_c, const uint8_t* a, uint8_t _b, unsigned _num_byte) {
	unsigned b = _b;
	__m256i m_tab = _mm256_load_si256((__m256i*) (__gf256_mul + 32 * b));
	__m256i ml = _mm256_permute2x128_si256(m_tab, m_tab, 0);
	__m256i mh = _mm256_permute2x128_si256(m_tab, m_tab, 0x11);
	__m256i mask = _mm256_load_si256((__m256i*) __mask_low);

	linearmap_8x8_accu_ymm(accu_c, a, ml, mh, mask, _num_byte);
}

static inline
void gf16v_madd_avx2(uint8_t* accu_c, const uint8_t* a, uint8_t gf16_b, unsigned _num_byte) {
	unsigned b = gf16_b & 0xf;
	__m256i m_tab = _mm256_load_si256((__m256i*) (__gf16_mul + 32 * b));
	__m256i ml = _mm256_permute2x128_si256(m_tab, m_tab, 0);
	__m256i mh = _mm256_permute2x128_si256(m_tab, m_tab, 0x11);
	__m256i mask = _mm256_load_si256((__m256i*) __mask_low);

	linearmap_8x8_accu_ymm(accu_c, a, ml, mh, mask, _num_byte);
}







static inline
void gf16v_madd_multab_avx2(uint8_t* accu_c, const uint8_t* a, const uint8_t* multab, unsigned _num_byte) {
	__m128i ml_16 = _mm_load_si128((__m128i*) multab);
	__m128i mh_16 = _mm_slli_epi16(ml_16, 4);
	__m256i ml = _mm256_inserti128_si256(_mm256_castsi128_si256(ml_16), ml_16, 1);
	__m256i mh = _mm256_inserti128_si256(_mm256_castsi128_si256(mh_16), mh_16, 1);
	__m256i mask = _mm256_load_si256((__m256i*) __mask_low);

	linearmap_8x8_accu_ymm(accu_c, a, ml, mh, mask, _num_byte);
}

//////////////////////   basic functions  ///////////////////////////////////////////////





static inline
void gf256v_add_sse(uint8_t* accu_b, const uint8_t* a, unsigned _num_byte) {
	//uint8_t temp[32] __attribute__((aligned(32)));
	unsigned n_xmm = (_num_byte) >> 4;
	for (unsigned i = n_xmm; i > 0; i--) {
		__m128i inp = _mm_loadu_si128((__m128i*) (a));
		__m128i out = _mm_loadu_si128((__m128i*) (accu_b));
		//out ^= inp;
		out = _mm_xor_si128(out, inp);
		_mm_storeu_si128((__m128i*) (accu_b), out);
		a += 16;
		accu_b += 16;
	}
	if (0 == (_num_byte & 0xf)) return;
	for (unsigned j = 0; j < (_num_byte & 0xf); j++) {
		accu_b[j] ^= a[j];
	}
}


static inline
void gf256v_add_avx2(uint8_t* accu_b, const uint8_t* a, unsigned _num_byte) {
	//uint8_t temp[32] __attribute__((aligned(32)));
	unsigned n_ymm = (_num_byte) >> 5;
	unsigned i = 0;
	for (; i < n_ymm; i++) {
		__m256i inp = _mm256_loadu_si256((__m256i*) (a + i * 32));
		__m256i out = _mm256_loadu_si256((__m256i*) (accu_b + i * 32));
		//out ^= inp;
		out = _mm256_xor_si256(out, inp);
		_mm256_storeu_si256((__m256i*) (accu_b + i * 32), out);
	}
	if (0 != (_num_byte & 0x1f)) {
		n_ymm <<= 5;
		gf256v_add_sse(accu_b + n_ymm, a + n_ymm, _num_byte & 0x1f);
	}
}


///////////////////////////////



extern const unsigned char __mask_low[];
extern const unsigned char * __gf16_mul;
extern const unsigned char __gf256_mul[];

static inline
void linearmap_8x8_sse(uint8_t* a, __m128i ml, __m128i mh, __m128i mask, unsigned _num_byte) {
	unsigned n_16 = _num_byte >> 4;
	for (unsigned i = n_16; i > 0; i--) {
		__m128i inp = _mm_loadu_si128((__m128i*)(a));
		__m128i r0 = linear_transform_8x8_128b(ml, mh, inp, mask);
		_mm_storeu_si128((__m128i*)(a), r0);
		a += 16;
	}

	unsigned rem = _num_byte & 15;
	if (rem) {
		__m128i inp = _load_xmm(a, rem);
		__m128i r0 = linear_transform_8x8_128b(ml, mh, inp, mask);
		_store_xmm(a, rem, r0);
	}
}

static inline
void linearmap_8x8_ymm(uint8_t* a, __m256i ml, __m256i mh, __m256i mask, unsigned _num_byte) {
	unsigned n_32 = _num_byte >> 5;
	for (unsigned i = 0; i < n_32; i++) {
		__m256i inp = _mm256_loadu_si256((__m256i*)(a + i * 32));
		__m256i r0 = linear_transform_8x8_256b(ml, mh, inp, mask);
		_mm256_storeu_si256((__m256i*)(a + i * 32), r0);
	}
	unsigned rem = _num_byte & 31;
	if (rem) linearmap_8x8_sse(a + n_32 * 32, _mm256_castsi256_si128(ml), _mm256_castsi256_si128(mh), _mm256_castsi256_si128(mask), rem);
}
static inline
void gf16v_mul_scalar_avx2(uint8_t* a, uint8_t gf16_b, unsigned _num_byte) 
{
	unsigned b = gf16_b & 0xf;
	__m256i m_tab = _mm256_load_si256((__m256i*) (__gf16_mul + 32 * b));
	__m256i ml = _mm256_permute2x128_si256(m_tab, m_tab, 0);
	__m256i mh = _mm256_permute2x128_si256(m_tab, m_tab, 0x11);
	__m256i mask = _mm256_load_si256((__m256i*) __mask_low);

	linearmap_8x8_ymm(a, ml, mh, mask, _num_byte);
}
static inline
void gf256v_mul_scalar_avx2(uint8_t* a, uint8_t _b, unsigned _num_byte) {
	unsigned b = _b;
	__m256i m_tab = _mm256_load_si256((__m256i*) (__gf256_mul + 32 * b));
	__m256i ml = _mm256_permute2x128_si256(m_tab, m_tab, 0);
	__m256i mh = _mm256_permute2x128_si256(m_tab, m_tab, 0x11);
	__m256i mask = _mm256_load_si256((__m256i*) __mask_low);

	linearmap_8x8_ymm(a, ml, mh, mask, _num_byte);
}


static inline
void gf16v_mul_scalar_sse( uint8_t * a, uint8_t gf16_b , unsigned _num_byte ) {
	unsigned b = gf16_b&0xf;
	__m128i ml = _mm_load_si128( (__m128i*) (__gf16_mul + 32*b) );
	__m128i mh = _mm_load_si128( (__m128i*) (__gf16_mul + 32*b + 16) );
	__m128i mask = _mm_set1_epi8(0xf);

	linearmap_8x8_sse( a, ml , mh , mask , _num_byte );
}



static inline
void gf16v_madd_multab_sse( uint8_t * accu_c, const uint8_t * a , const uint8_t * multab , unsigned _num_byte ) {
	__m128i ml = _mm_load_si128( (__m128i*) multab );
	__m128i mh = _mm_slli_epi16( ml , 4 );
	__m128i mask = _mm_set1_epi8(0xf);

	linearmap_8x8_accu_sse( accu_c , a , ml , mh , mask , _num_byte );
}


static inline
void gf16v_madd_sse( uint8_t * accu_c, const uint8_t * a , uint8_t gf16_b, unsigned _num_byte ) {
	unsigned b = gf16_b&0xf;
	__m128i ml = _mm_load_si128( (__m128i*) (__gf16_mul + 32*b) );
	__m128i mh = _mm_load_si128( (__m128i*) (__gf16_mul + 32*b + 16) );
	__m128i mask = _mm_set1_epi8(0xf);

	linearmap_8x8_accu_sse( accu_c , a , ml , mh , mask , _num_byte );
}


static inline
void gf256v_mul_scalar_sse( uint8_t * a, uint8_t _b , unsigned _num_byte ) {
	unsigned b = _b;
	__m128i ml = _mm_load_si128( (__m128i*) (__gf256_mul + 32*b) );
	__m128i mh = _mm_load_si128( (__m128i*) (__gf256_mul + 32*b + 16) );
	__m128i mask = _mm_set1_epi8(0xf);

	linearmap_8x8_sse( a, ml , mh , mask , _num_byte );
}


static inline
void gf256v_madd_multab_sse( uint8_t * accu_c, const uint8_t * a , const uint8_t * multab , unsigned _num_byte ) {
	__m128i ml = _mm_load_si128( (__m128i*) multab );
	__m128i mh = _mm_load_si128( (__m128i*) (multab+16) );
	__m128i mask = _mm_set1_epi8(0xf);

	linearmap_8x8_accu_sse( accu_c , a , ml , mh , mask , _num_byte );
}

static inline
void gf256v_madd_sse( uint8_t * accu_c, const uint8_t * a , uint8_t _b, unsigned _num_byte ) {
	unsigned b = _b;
	__m128i ml = _mm_load_si128( (__m128i*) (__gf256_mul + 32*b) );
	__m128i mh = _mm_load_si128( (__m128i*) (__gf256_mul + 32*b + 16) );
	__m128i mask = _mm_set1_epi8(0xf);

	linearmap_8x8_accu_sse( accu_c , a , ml , mh , mask , _num_byte );
}




static inline
void transpose_16x16_sse( uint8_t * r , const uint8_t * a ) {
//	for(unsigned j=0;j<16;j++)
//		for(unsigned k=0;k<16;k++) r[j*16+k] = a[k*16+j];

	__m128i a0 = _mm_load_si128( (__m128i*) a );
	__m128i a1 = _mm_load_si128( (__m128i*) (a+16) );
	__m128i b0 = _mm_unpacklo_epi8( a0 , a1 );
	__m128i b1 = _mm_unpackhi_epi8( a0 , a1 );

	__m128i a2 = _mm_load_si128( (__m128i*) (a+16*2) );
	__m128i a3 = _mm_load_si128( (__m128i*) (a+16*3) );
	__m128i b2 = _mm_unpacklo_epi8( a2 , a3 );
	__m128i b3 = _mm_unpackhi_epi8( a2 , a3 );

	__m128i c0 = _mm_unpacklo_epi16( b0 , b2 );
	__m128i c1 = _mm_unpacklo_epi16( b1 , b3 );
	__m128i c2 = _mm_unpackhi_epi16( b0 , b2 );
	__m128i c3 = _mm_unpackhi_epi16( b1 , b3 );

	__m128i a4 = _mm_load_si128( (__m128i*) (a+16*4) );
	__m128i a5 = _mm_load_si128( (__m128i*) (a+16*5) );
	__m128i b4 = _mm_unpacklo_epi8( a4 , a5 );
	__m128i b5 = _mm_unpackhi_epi8( a4 , a5 );

	__m128i a6 = _mm_load_si128( (__m128i*) (a+16*6) );
	__m128i a7 = _mm_load_si128( (__m128i*) (a+16*7) );
	__m128i b6 = _mm_unpacklo_epi8( a6 , a7 );
	__m128i b7 = _mm_unpackhi_epi8( a6 , a7 );

	__m128i c4 = _mm_unpacklo_epi16( b4 , b6 );
	__m128i c5 = _mm_unpacklo_epi16( b5 , b7 );
	__m128i c6 = _mm_unpackhi_epi16( b4 , b6 );
	__m128i c7 = _mm_unpackhi_epi16( b5 , b7 );

	__m128i d0 = _mm_unpacklo_epi32( c0 , c4 );
	__m128i d1 = _mm_unpacklo_epi32( c1 , c5 );
	__m128i d2 = _mm_unpacklo_epi32( c2 , c6 );
	__m128i d3 = _mm_unpacklo_epi32( c3 , c7 );
	__m128i d4 = _mm_unpackhi_epi32( c0 , c4 );
	__m128i d5 = _mm_unpackhi_epi32( c1 , c5 );
	__m128i d6 = _mm_unpackhi_epi32( c2 , c6 );
	__m128i d7 = _mm_unpackhi_epi32( c3 , c7 );
/////
	__m128i a8 = _mm_load_si128( (__m128i*) (a+16*8) );
	__m128i a9 = _mm_load_si128( (__m128i*) (a+16*9) );
	__m128i b8 = _mm_unpacklo_epi8( a8 , a9 );
	__m128i b9 = _mm_unpackhi_epi8( a8 , a9 );

	__m128i a10 = _mm_load_si128( (__m128i*) (a+16*10) );
	__m128i a11 = _mm_load_si128( (__m128i*) (a+16*11) );
	__m128i b10 = _mm_unpacklo_epi8( a10 , a11 );
	__m128i b11 = _mm_unpackhi_epi8( a10 , a11 );

	__m128i c8 = _mm_unpacklo_epi16( b8 , b10 );
	__m128i c9 = _mm_unpacklo_epi16( b9 , b11 );
	__m128i c10 = _mm_unpackhi_epi16( b8 , b10 );
	__m128i c11 = _mm_unpackhi_epi16( b9 , b11 );

	__m128i a12 = _mm_load_si128( (__m128i*) (a+16*12) );
	__m128i a13 = _mm_load_si128( (__m128i*) (a+16*13) );
	__m128i b12 = _mm_unpacklo_epi8( a12 , a13 );
	__m128i b13 = _mm_unpackhi_epi8( a12 , a13 );

	__m128i a14 = _mm_load_si128( (__m128i*) (a+16*14) );
	__m128i a15 = _mm_load_si128( (__m128i*) (a+16*15) );
	__m128i b14 = _mm_unpacklo_epi8( a14 , a15 );
	__m128i b15 = _mm_unpackhi_epi8( a14 , a15 );

	__m128i c12 = _mm_unpacklo_epi16( b12 , b14 );
	__m128i c13 = _mm_unpacklo_epi16( b13 , b15 );
	__m128i c14 = _mm_unpackhi_epi16( b12 , b14 );
	__m128i c15 = _mm_unpackhi_epi16( b13 , b15 );

	__m128i d8 = _mm_unpacklo_epi32( c8 , c12 );
	__m128i d9 = _mm_unpacklo_epi32( c9 , c13 );
	__m128i d10 = _mm_unpacklo_epi32( c10 , c14 );
	__m128i d11 = _mm_unpacklo_epi32( c11 , c15 );
	__m128i d12 = _mm_unpackhi_epi32( c8 , c12 );
	__m128i d13 = _mm_unpackhi_epi32( c9 , c13 );
	__m128i d14 = _mm_unpackhi_epi32( c10 , c14 );
	__m128i d15 = _mm_unpackhi_epi32( c11 , c15 );
//////
	__m128i e0 = _mm_unpacklo_epi64( d0 , d8 );
	__m128i e8 = _mm_unpackhi_epi64( d0 , d8 );
	_mm_store_si128( (__m128i*)( r + 16*0 ) , e0 );
	_mm_store_si128( (__m128i*)( r + 16*1 ) , e8 );

	__m128i e1 = _mm_unpacklo_epi64( d1 , d9 );
	__m128i e9 = _mm_unpackhi_epi64( d1 , d9 );
	_mm_store_si128( (__m128i*)( r + 16*8 ) , e1 );
	_mm_store_si128( (__m128i*)( r + 16*9 ) , e9 );

	__m128i e2 = _mm_unpacklo_epi64( d2 , d10 );
	__m128i e10 = _mm_unpackhi_epi64( d2 , d10 );
	_mm_store_si128( (__m128i*)( r + 16*4 ) , e2 );
	_mm_store_si128( (__m128i*)( r + 16*5 ) , e10 );

	__m128i e3 = _mm_unpacklo_epi64( d3 , d11 );
	__m128i e11 = _mm_unpackhi_epi64( d3 , d11 );
	_mm_store_si128( (__m128i*)( r + 16*0xc ) , e3 );
	_mm_store_si128( (__m128i*)( r + 16*0xd ) , e11 );

	__m128i e4 = _mm_unpacklo_epi64( d4 , d12 );
	__m128i e12 = _mm_unpackhi_epi64( d4 , d12 );
	_mm_store_si128( (__m128i*)( r + 16*2 ) , e4 );
	_mm_store_si128( (__m128i*)( r + 16*3 ) , e12 );

	__m128i e5 = _mm_unpacklo_epi64( d5 , d13 );
	__m128i e13 = _mm_unpackhi_epi64( d5 , d13 );
	_mm_store_si128( (__m128i*)( r + 16*0xa ) , e5 );
	_mm_store_si128( (__m128i*)( r + 16*0xb ) , e13 );

	__m128i e6 = _mm_unpacklo_epi64( d6 , d14 );
	__m128i e14 = _mm_unpackhi_epi64( d6 , d14 );
	_mm_store_si128( (__m128i*)( r + 16*6 ) , e6 );
	_mm_store_si128( (__m128i*)( r + 16*7 ) , e14 );

	__m128i e7 = _mm_unpacklo_epi64( d7 , d15 );
	__m128i e15 = _mm_unpackhi_epi64( d7 , d15 );
	_mm_store_si128( (__m128i*)( r + 16*0xe ) , e7 );
	_mm_store_si128( (__m128i*)( r + 16*15 ) , e15 );
}





/////////////////  GF( 16 ) /////////////////////////////////////




static inline
void gf16v_generate_multab_16_sse( uint8_t * _multab_byte , const uint8_t * _x0 )
{
	ALIGN(32) uint8_t multab[16*16];
	__m128i cc = _mm_load_si128( (__m128i*) (_x0) );
	for(unsigned j=0;j<16;j++) {
		__m128i mt = _mm_load_si128( (__m128i*) (__gf16_mulx2 + 32*j) );
		_mm_store_si128( (__m128i*)(multab + j*16) , _mm_shuffle_epi8( mt, cc ) );
	}
	transpose_16x16_sse( _multab_byte , multab );
}

static inline
void gf16v_split_16to32_sse( __m128i * x_align , __m128i a )
{
	__m128i mask_f = _mm_set1_epi8(0xf);
#ifdef LINUX_VERSION  
	__m128i al = a&mask_f;
	__m128i ah = _mm_srli_epi16( a,4 )&mask_f;
#else
	__m128i al = _mm_and_si128(a , mask_f);
	__m128i ah = _mm_and_si128(_mm_srli_epi16(a, 4) , mask_f);
#endif

	__m128i a0 = _mm_unpacklo_epi8( al , ah );
	__m128i a1 = _mm_unpackhi_epi8( al , ah );

	_mm_store_si128( x_align , a0 );
	_mm_store_si128( x_align + 1 , a1 );
}

static inline
void gf16v_generate_multab_sse( uint8_t * _multabs , const uint8_t * x , unsigned n )
{
	ALIGN(32) uint8_t _x[32];

	unsigned n_32 = n>>5;
	unsigned n_16 = n>>4;
	unsigned n_16_rem = n&0xf;

	for(unsigned i=0;i<n_32;i++) {
		//for(unsigned j=0;j<16;j++) _x[j] = x[i*16+j];
		//__m128i x32 = _mm_load_si128( (__m128i*) _x );
		__m128i x32 = _mm_loadu_si128( (__m128i*) (x+i*16) );
		gf16v_split_16to32_sse( (__m128i*)_x , x32 );

		gf16v_generate_multab_16_sse( _multabs +  i*2*16*16 , _x );
		gf16v_generate_multab_16_sse( _multabs +  i*2*16*16 + 16*16 , _x + 16 );
	}
	if( n_16&1 ) {  /// n_16 is odd
		unsigned idx = n_16-1;

		for(unsigned j=0;j<8;j++) _x[j] = x[ idx*8 + j];

		__m128i x32 = _mm_load_si128( (__m128i*) _x );
		gf16v_split_16to32_sse( (__m128i*)_x , x32 );
		gf16v_generate_multab_16_sse( _multabs +  idx*16*16 , _x );
	}

	ALIGN(32) uint8_t multab[16*16];
	if( n_16_rem ) {
		unsigned rem_byte = (n_16_rem + 1)/2;
		for(unsigned j=0;j<rem_byte;j++) _x[j] = x[n_16*8 + j];

		__m128i x32 = _mm_load_si128( (__m128i*) _x );
		gf16v_split_16to32_sse( (__m128i*)_x , x32 );
		gf16v_generate_multab_16_sse( multab , _x );

		for(unsigned j=0;j<n_16_rem;j++) {
			__m128i temp = _mm_load_si128( (__m128i*) ( multab + 16*j) );
			_mm_store_si128( (__m128i *) (_multabs + n_16*16*16 + 16*j) , temp );
		}
	}
}



static inline
void gf16v_split_sse( uint8_t * x_align32 , const uint8_t * _x , unsigned n_gf16 )
{
	//assert( n <= 512 ); /// for spliting gf256v
	uint8_t * x = x_align32;
	unsigned n_byte = (n_gf16+1)/2;
	unsigned n_16 = n_byte>>4;
	unsigned n_16_rem = n_byte&0xf;
	__m128i mask_f = _mm_set1_epi8(0xf);
	for(unsigned i=0;i<n_16;i++) {
		__m128i inp = _mm_loadu_si128( (__m128i*)_x ); _x += 16;
#ifdef LINUX_VERSION  
		__m128i il = inp&mask_f;
		__m128i ih = _mm_srli_epi16(inp,4)&mask_f;
#else
		__m128i il = _mm_and_si128(inp , mask_f);
		__m128i ih = _mm_and_si128(_mm_srli_epi16(inp, 4) , mask_f);
#endif
		_mm_store_si128( (__m128i*)( x+ 32*i ) , _mm_unpacklo_epi8(il,ih) );
		_mm_store_si128( (__m128i*)( x+ 32*i + 16 ) , _mm_unpackhi_epi8(il,ih) );
	}
	if( n_16_rem ) {
		unsigned i = n_16;
		__m128i inp = _load_xmm( _x , n_16_rem );
#ifdef LINUX_VERSION  
		__m128i il = inp&mask_f;
		__m128i ih = _mm_srli_epi16(inp,4)&mask_f;
#else
		__m128i il = _mm_and_si128(inp , mask_f);
		__m128i ih = _mm_and_si128(_mm_srli_epi16(inp, 4) , mask_f);
#endif
		_mm_store_si128( (__m128i*)( x+ 32*i ) , _mm_unpacklo_epi8(il,ih) );
		_mm_store_si128( (__m128i*)( x+ 32*i + 16 ) , _mm_unpackhi_epi8(il,ih) );
	}
}


static inline
uint8_t gf16v_dot_sse( const uint8_t * a , const uint8_t * b , unsigned n_byte )
{
	ALIGN(32) uint8_t v1[32];
	ALIGN(32) uint8_t v2[32];
	ALIGN(32) uint8_t v3[32];

	unsigned n_xmm = n_byte>>4;
	unsigned n_rem = n_byte&15;
	__m128i r = _mm_setzero_si128();
	for(unsigned i=0;i<n_xmm;i++) {
		__m128i inp1 = _mm_loadu_si128(  (__m128i*)(a+i*16) );
		__m128i inp2 = _mm_loadu_si128(  (__m128i*)(b+i*16) );
		gf16v_split_16to32_sse( (__m128i *)v1 , inp1 );
		gf16v_split_16to32_sse( (__m128i *)v2 , inp2 );
#ifdef LINUX_VERSION  
		r ^= tbl_gf16_mul( _mm_load_si128( (__m128i*)(v1) ) , _mm_load_si128( (__m128i*)(v2) ) );
		r ^= tbl_gf16_mul( _mm_load_si128( (__m128i*)(v1+16) ) , _mm_load_si128( (__m128i*)(v2+16) ) );
#else
		r = _mm_xor_si128(r, tbl_gf16_mul(_mm_load_si128((__m128i*)(v1)), _mm_load_si128((__m128i*)(v2))));
		r = _mm_xor_si128(r, tbl_gf16_mul(_mm_load_si128((__m128i*)(v1 + 16)), _mm_load_si128((__m128i*)(v2 + 16))));
#endif
	}
	if( n_rem ) {
		_mm_store_si128( (__m128i*)(v3) , _mm_setzero_si128() );
		for(unsigned i=0;i<n_rem;i++) v3[i] = a[n_xmm*16+i];
		__m128i inp1 = _mm_load_si128(  (__m128i*)(v3) );
		for(unsigned i=0;i<n_rem;i++) v3[i] = b[n_xmm*16+i];
		__m128i inp2 = _mm_load_si128(  (__m128i*)(v3) );
		gf16v_split_16to32_sse( (__m128i *)v1 , inp1 );
		gf16v_split_16to32_sse( (__m128i *)v2 , inp2 );
#ifdef LINUX_VERSION  
		r ^= tbl_gf16_mul( _mm_load_si128( (__m128i*)(v1) ) , _mm_load_si128( (__m128i*)(v2) ) );
		r ^= tbl_gf16_mul( _mm_load_si128( (__m128i*)(v1+16) ) , _mm_load_si128( (__m128i*)(v2+16) ) );
#else
		r = _mm_xor_si128(r, tbl_gf16_mul(_mm_load_si128((__m128i*)(v1)), _mm_load_si128((__m128i*)(v2))));
		r = _mm_xor_si128(r, tbl_gf16_mul(_mm_load_si128((__m128i*)(v1 + 16)), _mm_load_si128((__m128i*)(v2 + 16))));
#endif
	}
#ifdef LINUX_VERSION  
	r ^= _mm_srli_si128(r,8);
	r ^= _mm_srli_si128(r,4);
	r ^= _mm_srli_si128(r,2);
	r ^= _mm_srli_si128(r,1);
	r ^= _mm_srli_epi16(r,4);
#else
	r = _mm_xor_si128(r, _mm_srli_si128(r, 8));
	r = _mm_xor_si128(r, _mm_srli_si128(r, 4));
	r = _mm_xor_si128(r, _mm_srli_si128(r, 2));
	r = _mm_xor_si128(r, _mm_srli_si128(r, 1));
	r = _mm_xor_si128(r, _mm_srli_epi16(r, 4));
#endif
	return _mm_extract_epi16(r,0)&0xf;
}




static inline
void gf256v_generate_multab_sse( uint8_t * _multabs , const uint8_t * _x , unsigned n )
{
	gf16v_generate_multab_sse( _multabs , _x , 2*n );

	__m128i mul_8 = _mm_load_si128( (__m128i*)(__gf16_mulx2 + 32*8) );
	for(unsigned i=0;i<n;i++) {
		__m128i ml = _mm_load_si128( (__m128i*) (_multabs+32*i) );
		__m128i mh = _mm_load_si128( (__m128i*) (_multabs+32*i+16) );
#ifdef LINUX_VERSION  
		__m128i ml256 = _mm_slli_epi16( mh,4) | ml;
		__m128i mh256 = _mm_slli_epi16(ml^mh,4)|_mm_shuffle_epi8(mul_8,mh);
#else
		__m128i ml256 = _mm_or_si128(_mm_slli_epi16(mh, 4) , ml);
		__m128i mh256 = _mm_or_si128(_mm_slli_epi16(_mm_xor_si128(ml , mh), 4) , _mm_shuffle_epi8(mul_8, mh));
#endif
		_mm_store_si128( (__m128i*) (_multabs+32*i) , ml256 );
		_mm_store_si128( (__m128i*) (_multabs+32*i+16) , mh256 );
	}
}

void gf16mat_prod_multab_sse(unsigned long handle, uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * multab );
void gf16mat_prod_sse(unsigned long handle, uint8_t * c , const uint8_t * mat_a , unsigned a_h_byte , unsigned a_w , const uint8_t * b );
void gf256mat_prod_multab_sse(unsigned long handle, uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * multab );
void gf256mat_prod_sse(unsigned long handle, uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * b );
unsigned gf16mat_gauss_elim_sse(unsigned long handle, uint8_t * mat , unsigned h , unsigned w );
unsigned gf256mat_gauss_elim_sse(unsigned long handle, uint8_t * mat , unsigned h , unsigned w );




#ifdef  __cplusplus
}
#endif

#endif

#endif // _BLAS_SSE_H_

