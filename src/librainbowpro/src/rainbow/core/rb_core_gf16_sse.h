#ifndef _GF16_SSE_H_
#define _GF16_SSE_H_

#include "rb_core_config.h"

#ifdef RB_SIMD_X86

#include "rb_core_gf16_tabs.h"


// SSE2
#include <emmintrin.h>

// SSSE3
#include <tmmintrin.h>


//////////////  GF(16)  /////////////////////////////




static inline __m128i tbl_gf16_squ( __m128i a )
{
	__m128i tab_l = _mm_load_si128((__m128i const *) __gf16_squ );
	return _mm_shuffle_epi8(tab_l,a);
}


static inline __m128i tbl_gf16_squ_x8( __m128i a )
{
	__m128i tab_l = _mm_load_si128((__m128i const *) __gf16_squ_x8 );
	return _mm_shuffle_epi8(tab_l,a);
}


static inline __m128i tbl_gf16_inv( __m128i a )
{
	__m128i tab_l = _mm_load_si128((__m128i const *) __gf16_inv );
	return _mm_shuffle_epi8(tab_l,a);
}

static inline __m128i tbl_gf16_log( __m128i a )
{
	__m128i tab_l = _mm_load_si128((__m128i const *) __gf16_log );
	return _mm_shuffle_epi8(tab_l,a);
}

static inline __m128i tbl_gf16_exp( __m128i a )
{
	__m128i tab_l = _mm_load_si128((__m128i const *) __gf16_exp );
	return _mm_shuffle_epi8(tab_l,a);
}

static inline __m128i tbl_gf16_mul_const( unsigned char a , __m128i b )
{
	__m128i tab_l = _mm_load_si128((__m128i const *) (__gf256_mul+  ((unsigned)a)*32 ));
	return _mm_shuffle_epi8(tab_l,b);
}



static inline __m128i tbl_gf16_mul_0x8( __m128i b )
{
	__m128i tab_l = _mm_load_si128((__m128i const *) (__gf256_mul+  8*32 ));
	return _mm_shuffle_epi8(tab_l,b);
}




static inline __m128i tbl_gf16_mul( __m128i a , __m128i b )
{
	__m128i mask_f = _mm_load_si128((__m128i const *) __mask_low);
	__m128i log_16 = _mm_load_si128((__m128i const *) __gf16_log);
	__m128i exp_16 = _mm_load_si128((__m128i const *) __gf16_exp);

	__m128i la = _mm_shuffle_epi8(log_16,a);
	__m128i lb = _mm_shuffle_epi8(log_16,b);
	__m128i la_lb = _mm_add_epi8(la,lb);
#ifdef LINUX_VERSION  
	__m128i r0 = _mm_shuffle_epi8(exp_16, _mm_sub_epi8(la_lb, mask_f&_mm_cmpgt_epi8(la_lb,mask_f) ) );
#else
	__m128i r0 = _mm_shuffle_epi8(exp_16, _mm_sub_epi8(la_lb, _mm_and_si128(mask_f , _mm_cmpgt_epi8(la_lb, mask_f))));
#endif
	return r0;
}



static inline __m128i tbl_gf16_mul_log( __m128i a , __m128i logb , __m128i mask_f )
{
	__m128i la = tbl_gf16_log( a );
	__m128i la_lb = _mm_add_epi8(la,logb);
#ifdef LINUX_VERSION
	return tbl_gf16_exp( _mm_sub_epi8(la_lb, mask_f&_mm_cmpgt_epi8(la_lb,mask_f) ) );
#else
	return tbl_gf16_exp(_mm_sub_epi8(la_lb, _mm_and_si128(mask_f , _mm_cmpgt_epi8(la_lb, mask_f))));
#endif
}

static inline __m128i tbl_gf16_mul_log_log( __m128i loga , __m128i logb , __m128i mask_f )
{
	__m128i la_lb = _mm_add_epi8(loga,logb);
#ifdef LINUX_VERSION
	return tbl_gf16_exp( _mm_sub_epi8(la_lb, mask_f&_mm_cmpgt_epi8(la_lb,mask_f) ) );
#else
	return tbl_gf16_exp(_mm_sub_epi8(la_lb, _mm_and_si128(mask_f , _mm_cmpgt_epi8(la_lb, mask_f))));
#endif
}





/////////////////////////////  GF(256) ////////////////////////////////////////




static inline __m128i tbl_gf256_mul_const( unsigned char a , __m128i b )
{
	__m128i mask_f = _mm_load_si128((__m128i const *) __mask_low);
	__m128i tab_l = _mm_load_si128((__m128i const *) (__gf256_mul+  ((unsigned)a)*32 ));
	__m128i tab_h = _mm_load_si128((__m128i const *) (__gf256_mul+  ((unsigned)a)*32 + 16 ));

	return linear_transform_8x8_128b( tab_l , tab_h , b , mask_f );
}


/// use log table
static inline __m128i tbl_gf256_mul( __m128i a , __m128i b )
{
	__m128i mask_f = _mm_load_si128((__m128i const *) __mask_low);
	__m128i log_16 = _mm_load_si128((__m128i const *) __gf16_log);
	__m128i exp_16 = _mm_load_si128((__m128i const *) __gf16_exp);
#ifdef LINUX_VERSION
	__m128i a0 = a&mask_f;
	__m128i a1 = _mm_srli_epi16(a,4)&mask_f;
	__m128i b0 = b&mask_f;
	__m128i b1 = _mm_srli_epi16(b,4)&mask_f;
#else
	__m128i a0 = _mm_and_si128(a , mask_f);
	__m128i a1 = _mm_and_si128(_mm_srli_epi16(a, 4) , mask_f);
	__m128i b0 = _mm_and_si128(b , mask_f);
	__m128i b1 = _mm_and_si128(_mm_srli_epi16(b, 4) , mask_f);

#endif

	__m128i la0 = _mm_shuffle_epi8(log_16,a0);
	__m128i la1 = _mm_shuffle_epi8(log_16,a1);
	__m128i lb0 = _mm_shuffle_epi8(log_16,b0);
	__m128i lb1 = _mm_shuffle_epi8(log_16,b1);

	__m128i la0b0 = _mm_add_epi8(la0,lb0);
	__m128i la1b0 = _mm_add_epi8(la1,lb0);
	__m128i la0b1 = _mm_add_epi8(la0,lb1);
	__m128i la1b1 = _mm_add_epi8(la1,lb1);
#ifdef LINUX_VERSION
	__m128i r0 = _mm_shuffle_epi8(exp_16, _mm_sub_epi8(la0b0, mask_f&_mm_cmpgt_epi8(la0b0,mask_f) ) );
	__m128i r1 = _mm_shuffle_epi8(exp_16, _mm_sub_epi8(la1b0, mask_f&_mm_cmpgt_epi8(la1b0,mask_f) ) )
			^_mm_shuffle_epi8(exp_16, _mm_sub_epi8(la0b1, mask_f&_mm_cmpgt_epi8(la0b1,mask_f) ) );
	__m128i r2 = _mm_shuffle_epi8(exp_16, _mm_sub_epi8(la1b1, mask_f&_mm_cmpgt_epi8(la1b1,mask_f) ) );

	return _mm_slli_epi16(r1^r2,4)^r0^tbl_gf16_mul_0x8(r2);
#else
	__m128i r0 = _mm_shuffle_epi8(exp_16, _mm_sub_epi8(la0b0, _mm_and_si128(mask_f , _mm_cmpgt_epi8(la0b0, mask_f))));
	__m128i r1 = _mm_xor_si128(_mm_shuffle_epi8(exp_16, _mm_sub_epi8(la1b0, _mm_and_si128(mask_f , _mm_cmpgt_epi8(la1b0, mask_f))))
		, _mm_shuffle_epi8(exp_16, _mm_sub_epi8(la0b1, _mm_and_si128(mask_f , _mm_cmpgt_epi8(la0b1, mask_f)))));
	__m128i r2 = _mm_shuffle_epi8(exp_16, _mm_sub_epi8(la1b1, _mm_and_si128(mask_f , _mm_cmpgt_epi8(la1b1, mask_f))));

	return _mm_xor_si128(_mm_xor_si128(_mm_slli_epi16(_mm_xor_si128(r1 , r2), 4) , r0) , tbl_gf16_mul_0x8(r2));

#endif
}


#if 0

static inline __m128i tbl_gf16_squ_sl4( __m128i a )
{
	__m128i tab_l = _mm_load_si128((__m128i const *) __gf16_squ_sl4 );
	return _mm_shuffle_epi8(tab_l,a);
}

static inline __m128i tbl_gf256_squ( __m128i a )
{
	__m128i mask_f = _mm_load_si128((__m128i const *) __mask_low);
	__m128i a0 = a&mask_f;
	__m128i a1 = _mm_srli_epi16(a,4)&mask_f;
	__m128i a0squ = tbl_gf16_squ(a0);
	__m128i a1squ_sl4 = tbl_gf16_squ_sl4(a1);
	__m128i a1squ_x8 = tbl_gf16_squ_x8( a1 );
	return a1squ_sl4^a0squ^a1squ_x8;
}

#endif



static inline __m128i tbl_gf256_inv( __m128i a )
{
#if 1
// faster
#ifdef LINUX_VERSION  
	__m128i mask_f = _mm_load_si128((__m128i const *) __mask_low);
	__m128i a0 = a&mask_f;
	__m128i a1 = _mm_srli_epi16(a,4)&mask_f;
	__m128i a0_a1 = a0^a1;
	__m128i a1squx8 = tbl_gf16_squ_x8( a1 );
	__m128i a0xa0_a1 = tbl_gf16_mul( a0 , a0_a1 );
	__m128i denominator = a1squx8^a0xa0_a1;
	__m128i _denominator = tbl_gf16_inv( denominator );
	__m128i b1 = tbl_gf16_mul( _denominator , a1 );
	__m128i a1inv = tbl_gf16_inv(a1);
	__m128i b01 = tbl_gf16_mul( a0_a1 , a1inv );
	b01 = tbl_gf16_mul( b01 , b1 );
	__m128i a1x8 = tbl_gf16_mul_0x8( a1 );
	__m128i a0inv = tbl_gf16_inv(a0);
	__m128i a1x8xb1_1 = tbl_gf16_mul( a1x8 , b1 ) ^ _mm_set1_epi8(1);
	__m128i b02 = tbl_gf16_mul( a0inv , a1x8xb1_1 );
	__m128i b0 = _mm_setzero_si128();
	b0 |= _mm_andnot_si128( _mm_cmpeq_epi8(b0,a1inv), b01 ) |  _mm_andnot_si128( _mm_cmpeq_epi8(b0,a0inv), b02 );
	return _mm_slli_epi16(b1,4)^b0;
#else
	__m128i mask_f = _mm_load_si128((__m128i const*) __mask_low);
	__m128i a0 = _mm_and_si128(a , mask_f);
	__m128i a1 = _mm_and_si128(_mm_srli_epi16(a, 4) , mask_f);
	__m128i a0_a1 = _mm_xor_si128(a0 , a1);
	__m128i a1squx8 = tbl_gf16_squ_x8(a1);
	__m128i a0xa0_a1 = tbl_gf16_mul(a0, a0_a1);
	__m128i denominator = _mm_xor_si128(a1squx8 , a0xa0_a1);
	__m128i _denominator = tbl_gf16_inv(denominator);
	__m128i b1 = tbl_gf16_mul(_denominator, a1);
	__m128i a1inv = tbl_gf16_inv(a1);
	__m128i b01 = tbl_gf16_mul(a0_a1, a1inv);
	b01 = tbl_gf16_mul(b01, b1);
	__m128i a1x8 = tbl_gf16_mul_0x8(a1);
	__m128i a0inv = tbl_gf16_inv(a0);
	__m128i a1x8xb1_1 = _mm_xor_si128(tbl_gf16_mul(a1x8, b1) , _mm_set1_epi8(1));
	__m128i b02 = tbl_gf16_mul(a0inv, a1x8xb1_1);
	__m128i b0 = _mm_setzero_si128();
	b0 = _mm_or_si128(b0, _mm_or_si128(_mm_andnot_si128(_mm_cmpeq_epi8(b0, a1inv), b01) , _mm_andnot_si128(_mm_cmpeq_epi8(b0, a0inv), b02)));
	return _mm_xor_si128(_mm_slli_epi16(b1, 4) , b0);

#endif
#else
// slow
	__m128i a2 = tbl_gf256_squ(a);
	__m128i a3 = tbl_gf256_mul(a2,a);
	__m128i a6 = tbl_gf256_squ(a3);
	__m128i a7 = tbl_gf256_mul(a6,a);
	__m128i ae = tbl_gf256_squ(a7);
	__m128i af = tbl_gf256_mul(ae,a);
	__m128i af1 = tbl_gf256_squ(af);
	__m128i af2 = tbl_gf256_squ(af1);
	__m128i af3 = tbl_gf256_squ(af2);
	__m128i a7f = tbl_gf256_mul(af3,a7);
	return tbl_gf256_squ(a7f);
#endif
}




static inline __m128i tbl_gf256_set_value( unsigned char a ) { return _mm_set1_epi8(a); }


static inline void _tbl_gf256_set_value( unsigned char * b, unsigned char a ) {
	_mm_storeu_si128( (__m128i *)b , _mm_set1_epi8(a) );
}


static inline unsigned char tbl_gf256_get_1st_value( __m128i a ) { return (_mm_extract_epi16(a,0)&0xff); }


#endif


#endif // _GF16_SSE_H_

