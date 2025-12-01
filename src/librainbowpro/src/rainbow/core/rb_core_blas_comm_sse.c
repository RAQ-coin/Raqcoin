#include "rb_core_config.h"

#ifdef RB_SIMD_X86

#include <string.h>
#include <stdint.h>

#include <emmintrin.h>
#include <tmmintrin.h>

#include "rb_core_blas_comm_sse.h"
#include "rb_core_blas_comm.h"
#include "rb_core_blas.h"


#include "rb_core_gf16_sse.h"
#include "rb_core_blas_sse.h"


#if defined(__GNUC__)
#define ALIGN(x) __attribute__ ((aligned(x)))
#elif defined(_MSC_VER)
#define ALIGN(x) __declspec(align(x))
#elif defined(__ARMCC_VERSION)
#define ALIGN(x) __align(x)
#else
#define ALIGN(x)
#endif

void gf16mat_prod_add_multab_sse(unsigned long handle, uint8_t* c, const uint8_t* matA, unsigned n_A_vec_byte, unsigned n_A_width, const uint8_t* multab) 
{
	assert(n_A_vec_byte <= 512);
	__m128i tmp_c[32];
	gf16mat_prod_multab_sse(handle,(uint8_t*)tmp_c, matA, n_A_vec_byte, n_A_width, multab);
	gf256v_add_sse(c, (uint8_t*)tmp_c, n_A_vec_byte);
}



static
void gf16mat_prod_multab_16_sse(unsigned long handle, uint8_t * c , const uint8_t * matA , unsigned n_A_width , const uint8_t * multab ) {
	//assert( n_A_width <= 224 );
	//assert( n_A_width > 0 );

	__m128i mask_f = _mm_set1_epi8(0xf);

	__m128i r0 = _mm_setzero_si128();
	__m128i r1 = _mm_setzero_si128();
	while( n_A_width-- ) {
		__m128i ml = _mm_load_si128( (__m128i*) multab );
		__m128i inp = _mm_loadu_si128( (__m128i*) matA );
#ifdef LINUX_VERSION  
		r0 ^= _mm_shuffle_epi8( ml , inp&mask_f );
		r1 ^= _mm_shuffle_epi8( ml , _mm_srli_epi16(_mm_andnot_si128(mask_f,inp),4) );
#else
		r0 = _mm_xor_si128(r0, _mm_shuffle_epi8(ml, _mm_and_si128(inp , mask_f)));
		r1 = _mm_xor_si128(r1,_mm_shuffle_epi8(ml, _mm_srli_epi16(_mm_andnot_si128(mask_f, inp), 4)));
#endif
		matA += 16;
		multab += 16;
	}
#ifdef LINUX_VERSION  
	_mm_storeu_si128( (__m128i*) c , r0^_mm_slli_epi16(r1,4) );
#else
	_mm_storeu_si128((__m128i*) c, _mm_xor_si128(r0 , _mm_slli_epi16(r1, 4)));
#endif
}



void gf16mat_prod_multab_sse(unsigned long handle, uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * multab )
{
	if( 16 == n_A_vec_byte ) 
	{ 
		return gf16mat_prod_multab_16_sse(handle,c,matA,n_A_width,multab);
	}
	//assert( n_A_vec_byte <= 512 );
	__m128i mask_f = _mm_set1_epi8(0xf);

	__m128i r0[32];
	__m128i r1[32];
	unsigned n_xmm = ((n_A_vec_byte + 15)>>4);
	for(unsigned i=0;i<n_xmm;i++) r0[i] = _mm_setzero_si128();
	for(unsigned i=0;i<n_xmm;i++) r1[i] = _mm_setzero_si128();

	for(unsigned i=0;i<n_A_width-1;i++) {
		__m128i ml = _mm_load_si128( (__m128i*)( multab + i*16) );
		//__m128i mh = _mm_slli_epi16( ml , 4 );
		for(unsigned j=0;j<n_xmm;j++) {
			__m128i inp = _mm_loadu_si128( (__m128i*)(matA+j*16) );
#ifdef LINUX_VERSION  
			r0[j] ^= _mm_shuffle_epi8( ml , inp&mask_f );
			r1[j] ^= _mm_shuffle_epi8( ml , _mm_srli_epi16(inp,4)&mask_f );
#else
			r0[j] = _mm_xor_si128(r0[j], _mm_shuffle_epi8(ml, _mm_and_si128(inp , mask_f)));
			r1[j] = _mm_xor_si128(r1[j],_mm_shuffle_epi8(ml, _mm_and_si128(_mm_srli_epi16(inp, 4) , mask_f)));
#endif
		}
		matA += n_A_vec_byte;
	}
	unsigned n_16 = (n_A_vec_byte>>4);
	unsigned n_16_rem = n_A_vec_byte&0xf;
	{
		// last column
		unsigned i=n_A_width-1;

		__m128i ml = _mm_load_si128( (__m128i*)( multab + i*16) );
		for(unsigned j=0;j<n_16;j++) {
			__m128i inp = _mm_loadu_si128( (__m128i*)(matA+j*16) );
#ifdef LINUX_VERSION 
			r0[j] ^= _mm_shuffle_epi8( ml , inp&mask_f );
			r1[j] ^= _mm_shuffle_epi8( ml , _mm_srli_epi16(inp,4)&mask_f );
#else
			r0[j] = _mm_xor_si128(r0[j], _mm_shuffle_epi8(ml, _mm_and_si128(inp , mask_f)));
			r1[j] = _mm_xor_si128(r1[j], _mm_shuffle_epi8(ml, _mm_and_si128(_mm_srli_epi16(inp, 4) , mask_f)));
#endif
		}
		if( n_16_rem ) {
			__m128i inp = _load_xmm( matA + n_16*16 , n_16_rem );
#ifdef LINUX_VERSION 
			r0[n_16] ^= _mm_shuffle_epi8( ml , inp&mask_f );
			r1[n_16] ^= _mm_shuffle_epi8( ml , _mm_srli_epi16(inp,4)&mask_f );
#else
			r0[n_16] = _mm_xor_si128(r0[n_16], _mm_shuffle_epi8(ml, _mm_and_si128(inp , mask_f)));
			r1[n_16] = _mm_xor_si128(r1[n_16], _mm_shuffle_epi8(ml, _mm_and_si128(_mm_srli_epi16(inp, 4) , mask_f)));
#endif
		}
	}
#ifdef LINUX_VERSION 
	for(unsigned i=0;i<n_16;i++) _mm_storeu_si128( (__m128i*)(c + i*16) , r0[i]^_mm_slli_epi16(r1[i],4) );
	if( n_16_rem ) _store_xmm( c + n_16*16 , n_16_rem , r0[n_16]^_mm_slli_epi16(r1[n_16],4) );
#else
	for (unsigned i = 0; i < n_16; i++) _mm_storeu_si128((__m128i*)(c + i * 16), _mm_xor_si128( r0[i] , _mm_slli_epi16(r1[i], 4)));
	if (n_16_rem) _store_xmm(c + n_16 * 16, n_16_rem, _mm_xor_si128(r0[n_16] , _mm_slli_epi16(r1[n_16], 4)));
#endif
}



static inline
uint8_t _gf16v_get_ele( const uint8_t *a , unsigned i ) {
	uint8_t r = a[i>>1];
	if( 0 == (i&1) ) return r&0xf;
	else return r>>4;
}




void gf16mat_prod_16_sse(unsigned long handle, uint8_t * c , const uint8_t * mat_a , unsigned a_w , const uint8_t * b ) {
	assert( 0 == (a_w&0x1f) );
	__m128i mask_f = _mm_set1_epi8(0xf);

	__m128i r0 = _mm_setzero_si128();
	__m128i r1 = _mm_setzero_si128();

	ALIGN(32) uint8_t _x[32];
	while( a_w ) {
		__m128i x = _mm_loadu_si128( (__m128i*)b );
		b += 16;
		__m128i xx[2];
		gf16v_split_16to32_sse( xx , x );
		_mm_store_si128( (__m128i*)(_x) ,    tbl_gf16_log( xx[0] ) );
		_mm_store_si128( (__m128i*)(_x+16) , tbl_gf16_log( xx[1] ) );
		for(unsigned i=0;i<32;i++) {
			__m128i ml = _mm_set1_epi8( _x[i] );
			__m128i inp = _mm_loadu_si128( (__m128i*)(mat_a) );
			mat_a += 16;
#ifdef LINUX_VERSION  
			r0 ^= tbl_gf16_mul_log( inp&mask_f , ml , mask_f );
			r1 ^= tbl_gf16_mul_log( _mm_srli_epi16(_mm_andnot_si128(mask_f,inp),4) , ml , mask_f );
#else
			r0 = _mm_xor_si128(r0, tbl_gf16_mul_log(_mm_and_si128(inp , mask_f), ml, mask_f));
			r1 = _mm_xor_si128(r1, tbl_gf16_mul_log(_mm_srli_epi16(_mm_andnot_si128(mask_f, inp), 4), ml, mask_f));
#endif
		}
		a_w -= 32;
	}
#ifdef LINUX_VERSION  
	_mm_storeu_si128( (__m128i*)(c) , r0^_mm_slli_epi16(r1,4) );
#else
	_mm_storeu_si128((__m128i*)(c), _mm_xor_si128(r0 , _mm_slli_epi16(r1, 4)));

#endif
}


void gf16mat_prod_sse(unsigned long handle, uint8_t * c , const uint8_t * mat_a , unsigned a_h_byte , unsigned a_w , const uint8_t * b )
{
	if( 16==a_h_byte && (0==(a_w&0x1f)) ) 
	{ 
		return gf16mat_prod_16_sse(handle, c, mat_a,a_w,b); 
	}

	//assert( a_w <= 224 );
	//assert( a_h_byte <= 512 );

	__m128i mask_f = _mm_set1_epi8(0xf);

	ALIGN(32) __m128i r0[32];
	ALIGN(32) __m128i r1[32];
	unsigned n_xmm = ((a_h_byte+15)>>4);
	for(unsigned i=0;i<n_xmm;i++) r0[i] = _mm_setzero_si128();
	for(unsigned i=0;i<n_xmm;i++) r1[i] = _mm_setzero_si128();

	ALIGN(32) uint8_t _x[224];
	gf16v_split_sse( _x , b , a_w );
	for(unsigned i=0;i < ((a_w+15)>>4); i++) {
		__m128i xi = _mm_load_si128( (__m128i*)(_x+i*16) );
		_mm_store_si128( (__m128i*)(_x+i*16) , tbl_gf16_log( xi ) );
	}

	for(unsigned i=0;i< a_w -1;i++) {
		__m128i ml = _mm_set1_epi8( _x[i] );
		for(unsigned j=0;j<n_xmm;j++) {
			__m128i inp = _mm_loadu_si128( (__m128i*)(mat_a+j*16) );
#ifdef LINUX_VERSION 
			r0[j] ^= tbl_gf16_mul_log( inp&mask_f , ml , mask_f );
			r1[j] ^= tbl_gf16_mul_log( _mm_srli_epi16(inp,4)&mask_f , ml , mask_f );
#else
			r0[j] = _mm_xor_si128(r0[j], tbl_gf16_mul_log(_mm_and_si128(inp , mask_f), ml, mask_f));
			r1[j] = _mm_xor_si128(r1[j], tbl_gf16_mul_log(_mm_and_si128(_mm_srli_epi16(inp, 4) , mask_f), ml, mask_f));
#endif
		}
		mat_a += a_h_byte;
	}
	unsigned n_16 = (a_h_byte>>4);
	unsigned n_16_rem = a_h_byte&0xf;
	{
		// last column
		unsigned i=a_w-1;
		__m128i ml = _mm_set1_epi8( _x[i] );
		for(unsigned j=0;j<n_16;j++) {
			__m128i inp = _mm_loadu_si128( (__m128i*)(mat_a+j*16) );
#ifdef LINUX_VERSION  
			r0[j] ^= tbl_gf16_mul_log( inp&mask_f , ml , mask_f );
			r1[j] ^= tbl_gf16_mul_log( _mm_srli_epi16(inp,4)&mask_f , ml , mask_f );
#else
			r0[j] = _mm_xor_si128(r0[j], tbl_gf16_mul_log(_mm_and_si128(inp , mask_f), ml, mask_f));
			r1[j] = _mm_xor_si128(r1[j], tbl_gf16_mul_log(_mm_and_si128(_mm_srli_epi16(inp, 4) , mask_f), ml, mask_f));
#endif
		}
		if( n_16_rem ) 
		{
			__m128i inp = _load_xmm( mat_a + n_16*16 , n_16_rem );
#ifdef LINUX_VERSION  
			r0[n_16] ^= tbl_gf16_mul_log( inp&mask_f , ml , mask_f );
			r1[n_16] ^= tbl_gf16_mul_log( _mm_srli_epi16(inp,4)&mask_f , ml , mask_f );
#else
			r0[n_16] = _mm_xor_si128( r0[n_16], tbl_gf16_mul_log(_mm_and_si128(inp , mask_f), ml, mask_f));
			r1[n_16] = _mm_xor_si128( r1[n_16], tbl_gf16_mul_log(_mm_and_si128(_mm_srli_epi16(inp, 4) , mask_f), ml, mask_f));
#endif
		}
	}
#ifdef LINUX_VERSION  
	for(unsigned i=0;i<n_16;i++) _mm_storeu_si128( (__m128i*)(c + i*16) , r0[i]^_mm_slli_epi16(r1[i],4) );
	if( n_16_rem ) _store_xmm( c + n_16*16 , n_16_rem , r0[n_16]^_mm_slli_epi16(r1[n_16],4) );
#else
	for (unsigned i = 0; i < n_16; i++) _mm_storeu_si128((__m128i*)(c + i * 16), _mm_xor_si128(r0[i] , _mm_slli_epi16(r1[i], 4)));
	if (n_16_rem) _store_xmm(c + n_16 * 16, n_16_rem, _mm_xor_si128(r0[n_16] , _mm_slli_epi16(r1[n_16], 4)));
#endif
}




/// access aligned memory.
static
unsigned _gf16mat_gauss_elim_sse(unsigned long handle, uint8_t * mat , unsigned h , unsigned w_byte )
{
	/*assert( 0==(w_byte&31) ); /// w_byte is a multiple of 32.
	assert( 224 >= h );*/

	ALIGN(32)	uint8_t pivot_column[224];
	ALIGN(32)	uint8_t _multab[224*16];
	__m128i mask_f = _mm_set1_epi8( 0xf );
	unsigned w_2 = w_byte;
	unsigned w_n_16 = w_byte>>4;

	uint8_t r8 = 1;
	for(unsigned i=0;i<h;i++) {
		unsigned offset_16 = i>>5;
		uint8_t * ai = mat + w_2*i;
		for(unsigned j=i+1;j<h;j++) {
			uint8_t * aj = mat + w_2*j;

			uint8_t predicate = !rb_gf16_is_nonzero( _gf16v_get_ele(ai,i) );
			uint32_t pr_u32 = ((uint32_t)0)-((uint32_t)predicate);
			__m128i pr_u128 = _mm_set1_epi32( pr_u32 );
			for(unsigned k=offset_16;k<w_n_16;k++) {
				__m128i inp0 = _mm_load_si128( (__m128i*)(ai+k*16) );
				__m128i inp1 = _mm_load_si128( (__m128i*)(aj+k*16) );
#ifdef LINUX_VERSION 
				__m128i r = inp0^(inp1&pr_u128);
#else
				__m128i r = _mm_xor_si128(inp0 , _mm_and_si128(inp1 , pr_u128));
#endif
				_mm_store_si128( (__m128i*)(ai+k*16) , r );
			}
		}
		pivot_column[0] = _gf16v_get_ele( ai , i );
		r8 &= rb_gf16_is_nonzero( pivot_column[0] );
		__m128i p128 = _mm_load_si128( (__m128i*) pivot_column );
		__m128i inv_p = tbl_gf16_inv( p128 );
		_mm_store_si128( (__m128i*) pivot_column , inv_p );
		pivot_column[i] = pivot_column[0];
		for(unsigned j=0;j<h;j++) {
			if( i==j) continue;
			uint8_t * aj = mat + w_2*j;
			pivot_column[j] = _gf16v_get_ele( aj , i );
		}
		unsigned h_16 = (h+15)>>4;
		for(unsigned j=0;j<h_16;j++) { gf16v_generate_multab_16_sse( _multab + j*16*16 , pivot_column + j*16 ); }

		{
			// pivot row
			unsigned j=i;
			uint8_t * aj = mat + w_2*j;
			__m128i ml = _mm_load_si128( (__m128i*)(_multab + 16*j) );
			__m128i mh = _mm_slli_epi16( ml , 4 );
			for(unsigned k=offset_16;k<w_n_16;k++) {
				__m128i inp = _mm_load_si128( (__m128i*)(aj+k*16) );
				__m128i r = linear_transform_8x8_128b( ml , mh , inp , mask_f );
				_mm_store_si128( (__m128i*)(aj+k*16) , r );
			}
		}
		for(unsigned j=0;j<h;j++) {
			if( i == j ) continue;
			uint8_t * aj = mat + w_2*j;
			__m128i ml = _mm_load_si128( (__m128i*)(_multab + 16*j) );
			__m128i mh = _mm_slli_epi16( ml , 4 );
			for(unsigned k=offset_16;k<w_n_16;k++) {
				__m128i inp0 = _mm_load_si128( (__m128i*)(ai+k*16) );
				__m128i inp = _mm_load_si128( (__m128i*)(aj+k*16) );
#ifdef LINUX_VERSION  
				__m128i r = inp ^ linear_transform_8x8_128b( ml , mh , inp0 , mask_f );
#else
				__m128i r = _mm_xor_si128(inp , linear_transform_8x8_128b(ml, mh, inp0, mask_f));
#endif
				_mm_store_si128( (__m128i*)(aj+k*16) , r );
			}
		}
	}
	return r8;
}


unsigned gf16mat_gauss_elim_sse(unsigned long handle, uint8_t * mat , unsigned h , unsigned w )
{
	assert( 0 == (w&1) ); // w is even.
	assert( 448 >= w );
	assert( 224 >= h );

	ALIGN(32) uint8_t _mat[224*224];
	unsigned w_2 = (w+1)>>1;
	unsigned w_byte_32 = ((w_2+31)>>5) <<5;

	for(unsigned i=0;i<h;i++) for(unsigned j=0;j<w_2;j++) _mat[i*w_byte_32+j] = mat[i*w_2+j];
	unsigned r = _gf16mat_gauss_elim_sse(handle, _mat , h , w_byte_32 );
	for(unsigned i=0;i<h;i++) for(unsigned j=0;j<w_2;j++) mat[i*w_2+j] = _mat[i*w_byte_32+j];
	return r;
}


unsigned int rb_gf16mat_gauss_elim_8x16_sse(unsigned long handle, uint8_t* mat)
{
	return gf16mat_gauss_elim_sse(handle, mat, 8, 16);
}


///////////////////////////////  GF( 256 ) ////////////////////////////////////////////////////






void gf256mat_prod_add_multab_sse(unsigned long handle, __m128i * r , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * multab ) {
	__m128i mask_f = _mm_set1_epi8(0xf);
	unsigned n_xmm = ((n_A_vec_byte + 15)>>4);

	for(unsigned i=n_A_width;i>0;i--) {
		__m128i ml = _mm_load_si128( (__m128i*)( multab ) );
		__m128i mh = _mm_load_si128( (__m128i*)( multab + 16) );
		for(unsigned j=0;j<n_xmm;j++) {
			__m128i inp = _mm_loadu_si128( (__m128i*)(matA+j*16) );
#ifdef LINUX_VERSION 
			r[j] ^= linear_transform_8x8_128b( ml , mh , inp , mask_f );
#else
			r[j] = _mm_xor_si128(r[j], linear_transform_8x8_128b(ml, mh, inp, mask_f));
#endif
		}
		multab += 32;
		matA += n_A_vec_byte;
	}
}


void gf256mat_prod_multab_sse(unsigned long handle, uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * multab ) {
	assert( n_A_vec_byte <= 48*48 );

	__m128i r[48*48/16];
	unsigned n_xmm = ((n_A_vec_byte + 15)>>4);
	for(unsigned i=0;i<n_xmm;i++) r[i] = _mm_setzero_si128();

	if(0==n_A_width) { rb_gf256v_set_zero(handle,c,n_A_vec_byte);  return; }
	if(1 < n_A_width) gf256mat_prod_add_multab_sse(handle, r , matA, n_A_vec_byte , n_A_width - 1 , multab );

	// last column
	__m128i mask_f = _mm_set1_epi8(0xf);
	unsigned n_16 = (n_A_vec_byte>>4);
	unsigned n_16_rem = n_A_vec_byte&0xf;
	unsigned i=n_A_width-1;
	__m128i ml = _mm_load_si128( (__m128i*)( multab + i*32) );
	__m128i mh = _mm_load_si128( (__m128i*)( multab + i*32+16) );
	matA += i*n_A_vec_byte;
	for(unsigned j=0;j<n_16;j++) {
		__m128i inp = _mm_loadu_si128( (__m128i*)(matA+j*16) );
#ifdef LINUX_VERSION  
		r[j] ^= linear_transform_8x8_128b( ml , mh , inp , mask_f );
#else
		r[j] = _mm_xor_si128(r[j], linear_transform_8x8_128b(ml, mh, inp, mask_f));
#endif
	}
	if( n_16_rem ) {
		__m128i inp = _load_xmm( matA + n_16*16 , n_16_rem );
#ifdef LINUX_VERSION  
		r[n_16] ^= linear_transform_8x8_128b( ml , mh , inp , mask_f );
#else
		r[n_16] = _mm_xor_si128(r[n_16], linear_transform_8x8_128b(ml, mh, inp, mask_f));
#endif
	}
	for(unsigned i=0;i<n_16;i++) _mm_storeu_si128( (__m128i*)(c + i*16) , r[i] );
	if( n_16_rem ) _store_xmm( c + n_16*16 , n_16_rem , r[n_16] );
}


void gf256mat_prod_add_sse(unsigned long handle, __m128i * r , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * b ) {

	ALIGN(32) uint8_t multab[16*16*2];
	while( 16 < n_A_width ){
		gf256v_generate_multab_sse( multab , b , 16 );
		gf256mat_prod_add_multab_sse(handle, r , matA , n_A_vec_byte , 16 , multab );
		matA += n_A_vec_byte*16;
		b += 16;
		n_A_width -= 16;
	}
	// last 16 column
	gf256v_generate_multab_sse( multab , b , 16 );
	if(0 == n_A_width ){ return; }
	if(1 < n_A_width) gf256mat_prod_add_multab_sse(handle, r , matA, n_A_vec_byte , n_A_width - 1 , multab );
	// last column
	__m128i mask_f = _mm_set1_epi8(0xf);
	unsigned n_16 = (n_A_vec_byte>>4);
	unsigned n_16_rem = n_A_vec_byte&0xf;
	unsigned i=n_A_width-1;
	__m128i ml = _mm_load_si128( (__m128i*)( multab + i*32) );
	__m128i mh = _mm_load_si128( (__m128i*)( multab + i*32+16) );
	matA += i*n_A_vec_byte;
	for(unsigned j=0;j<n_16;j++) {
		__m128i inp = _mm_loadu_si128( (__m128i*)(matA+j*16) );
#ifdef LINUX_VERSION  
		r[j] ^= linear_transform_8x8_128b( ml , mh , inp , mask_f );
#else
		r[j] = _mm_xor_si128(r[j], linear_transform_8x8_128b(ml, mh, inp, mask_f));
#endif
	}
	if( n_16_rem ) {
		__m128i inp = _load_xmm( matA + n_16*16 , n_16_rem );
#ifdef LINUX_VERSION  
		r[n_16] ^= linear_transform_8x8_128b( ml , mh , inp , mask_f );
#else
		r[n_16] = _mm_xor_si128(r[n_16], linear_transform_8x8_128b(ml, mh, inp, mask_f));
#endif
	}
}


void gf256mat_prod_sse(unsigned long handle, uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * b )
{
	//assert( n_A_vec_byte <= 48*48 );

	//__m128i r[48*48/16];
	__m128i r[96 * 96 / 16];
	unsigned n_xmm = ((n_A_vec_byte + 15)>>4);
	for(unsigned i=0;i<n_xmm;i++) r[i] = _mm_setzero_si128();

	gf256mat_prod_add_sse(handle, r , matA , n_A_vec_byte , n_A_width , b );

	unsigned n_16 = (n_A_vec_byte>>4);
	unsigned n_16_rem = n_A_vec_byte&0xf;
	for(unsigned i=0;i<n_16;i++) _mm_storeu_si128( (__m128i*)(c + i*16) , r[i] );
	if( n_16_rem ) _store_xmm( c + n_16*16 , n_16_rem , r[n_16] );
}



static
unsigned _gf256mat_gauss_elim_sse(unsigned long handle, uint8_t * mat , unsigned h , unsigned w )
{
	assert( 0 == (w&15) );
	unsigned n_xmm = w>>4;

	__m128i mask_0 = _mm_setzero_si128();

	uint8_t rr8 = 1;
	for(unsigned i=0;i<h;i++) {
		unsigned char i_r = i&0xf;
		unsigned i_d = i>>4;

		uint8_t * mi = mat+i*w;

		for(unsigned j=i+1;j<h;j++) {
			__m128i piv_i = _mm_load_si128( (__m128i*)( mi + i_d*16 ) );
			uint8_t * mj = mat+j*w;
			__m128i piv_j = _mm_load_si128( (__m128i*)( mj + i_d*16 ) );
#ifdef LINUX_VERSION  
			__m128i is_madd = _mm_cmpeq_epi8( piv_i , mask_0 ) ^ _mm_cmpeq_epi8( piv_j , mask_0 );
#else
			__m128i is_madd = _mm_xor_si128(_mm_cmpeq_epi8(piv_i, mask_0) , _mm_cmpeq_epi8(piv_j, mask_0));
#endif
			__m128i madd_mask = _mm_shuffle_epi8( is_madd , _mm_set1_epi8(i_r) );
#ifdef LINUX_VERSION  
			piv_i ^= madd_mask&piv_j;
#else
			piv_i = _mm_xor_si128(piv_i, _mm_and_si128(madd_mask , piv_j));
#endif
			_mm_store_si128( (__m128i*)( mi+ i_d*16 ) , piv_i );
			for(unsigned k=i_d+1;k<n_xmm;k++) {
				piv_i = _mm_load_si128( (__m128i*)( mi + k*16 ) );
				piv_j = _mm_load_si128( (__m128i*)( mj + k*16 ) );
#ifdef LINUX_VERSION  
				piv_i ^= madd_mask&piv_j;
#else
				piv_i = _mm_xor_si128(piv_i, _mm_and_si128(madd_mask , piv_j));
#endif
				_mm_store_si128( (__m128i*)( mi+ k*16 ) , piv_i );
			}
		}
		rr8 &= rb_gf256_is_nonzero( mi[i] );

		__m128i _pivot = _mm_set1_epi8( mi[i] );
		__m128i _ip = tbl_gf256_inv( _pivot );
		for(unsigned k=i_d;k<n_xmm;k++) {
			__m128i rowi = _mm_load_si128( (__m128i*)(mi+k*16) );
			rowi = tbl_gf256_mul( rowi , _ip );
			_mm_store_si128( (__m128i*)(mi+k*16) , rowi );
		}

		for(unsigned j=0;j<h;j++) {
			if(i==j) continue;

			uint8_t * mj = mat+j*w;
			__m128i mm = _mm_set1_epi8( mj[i] );

			for(unsigned k=i_d;k<n_xmm;k++) {
				__m128i rowi = _mm_load_si128( (__m128i*)(mi+k*16) );
				rowi = tbl_gf256_mul( rowi , mm );
#ifdef LINUX_VERSION 
				rowi ^= _mm_load_si128( (__m128i*)(mj+k*16) );
#else
				rowi = _mm_xor_si128(rowi, _mm_load_si128((__m128i*)(mj + k * 16)));
#endif
				_mm_store_si128( (__m128i*)(mj+k*16) , rowi );
			}
		}
	}
	return rr8;
}



unsigned gf256mat_gauss_elim_sse(unsigned long handle, uint8_t * mat , unsigned h , unsigned w )
{
	assert( 512 >= w );
	assert( 256 >= h );

	ALIGN(32) uint8_t _mat[512*256];
	unsigned w_16 = ((w+15)>>4) <<4;

	for(unsigned i=0;i<h;i++) for(unsigned j=0;j<w;j++) _mat[i*w_16+j] = mat[i*w+j];
	unsigned r = _gf256mat_gauss_elim_sse(handle, _mat , h , w_16 );
	for(unsigned i=0;i<h;i++) for(unsigned j=0;j<w;j++) mat[i*w+j] = _mat[i*w_16+j];
	return r;
}






unsigned int rb_gf256mat_gauss_elim_sse(unsigned long handle, uint8_t* mat, unsigned int h, unsigned int w)
{
	return gf256mat_gauss_elim_sse(handle, mat, h, w);
}

#endif