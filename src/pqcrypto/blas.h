#ifndef _BLAS_H_
#define _BLAS_H_

#include "random.h"
#include "gf16.h"

#include "blas_config.h"
#include "assert.h"


#ifdef  __cplusplus
extern  "C" {
#endif
#if defined(__APPLE__) && defined(__MACH__)
    #include <stdlib.h>
#else
    #include <malloc.h>
#endif


#ifdef _BLAS_UINT64_

#include "blas_u64.h"

#define gf16v_mul_scalar  _gf16v_mul_scalar_u64
#define gf16v_madd        _gf16v_madd_u64

#define gf256v_add        _gf256v_add_u64
#define gf256v_mul_scalar  _gf256v_mul_scalar_u64
#define gf256v_madd        _gf256v_madd_u64
#define gf256v_m0x10_add  _gf256v_m0x10_add_u64

#define gf16mat_prod      _gf16mat_prod
#define gf16v_dot         _gf16v_dot

#define gf256v_m0x4_add  _gf256v_m0x4_add
#define gf256mat_prod      _gf256mat_prod


#else

#define gf16v_mul_scalar  _gf16v_mul_scalar
#define gf16v_madd        _gf16v_madd
#define gf16mat_prod      _gf16mat_prod
#define gf16v_dot         _gf16v_dot

#define gf256v_add        _gf256v_add
#define gf256v_mul_scalar  _gf256v_mul_scalar
#define gf256v_madd        _gf256v_madd
#define gf256v_m0x10_add  _gf256v_m0x10_add
#define gf256v_m0x4_add  _gf256v_m0x4_add
#define gf256mat_prod      _gf256mat_prod

#endif



extern unsigned char __zero_32[32];

static inline
void gf256v_fdump(FILE * fp, const uint8_t *v, unsigned _num_byte) {
	fprintf(fp,"[%2d][",_num_byte);
	for(unsigned i=0;i<_num_byte;i++) { fprintf(fp,"0x%02x,",v[i]); if(7==(i%8)) fprintf(fp," ");}
	fprintf(fp,"]");
}

static inline
void _gf256v_add( uint8_t * accu_b, const uint8_t * a , unsigned _num_byte ) {
	for(unsigned i=0;i<_num_byte;i++) accu_b[i]^=a[i];
}

static inline
void gf256v_set_zero( uint8_t * b, unsigned _num_byte ) { gf256v_add( b , b , _num_byte ); }

static inline
unsigned gf256v_is_zero( const uint8_t * a, unsigned _num_byte ) {
	unsigned char r = 0;
	for(unsigned i=0;i<_num_byte;i++) r |= a[i];
	return (0==r);
}

static inline
void _gf16v_mul_scalar( uint8_t * a, uint8_t gf16_b , unsigned _num_byte ) {
	for(unsigned i=0;i<_num_byte;i++) a[i] = gf256_mul_gf16( a[i] , gf16_b );
}

static inline
void _gf16v_madd( uint8_t * accu_c, const uint8_t * a , uint8_t gf16_b, unsigned _num_byte ) {
	for(unsigned i=0;i<_num_byte;i++) accu_c[i] ^= gf256_mul_gf16( a[i] , gf16_b );
}

static inline
void _gf256v_mul_scalar( uint8_t *a, uint8_t b, unsigned _num_byte ) {
	for(unsigned i=0;i<_num_byte;i++) a[i] = gf256_mul( a[i] , b );
}

static inline
void _gf256v_madd( uint8_t * accu_c, const uint8_t * a , uint8_t gf256_b, unsigned _num_byte ) {
	for(unsigned i=0;i<_num_byte;i++) accu_c[i] ^= gf256_mul( a[i] , gf256_b );
}

static inline
void _gf256v_m0x10_add( uint8_t * accu_c, const uint8_t * a , unsigned _num_byte ) {
	for(unsigned i=0;i<_num_byte;i++) accu_c[i] ^= gf256_mul_0x10( a[i] );
}

static inline
void _gf256v_m0x4_add( uint8_t * accu_c, const uint8_t * a , unsigned _num_byte ) {
	for(unsigned i=0;i<_num_byte;i++) accu_c[i] ^= gf256_mul_0x4( a[i] );
}

static inline
unsigned char gf16v_get_ele( const uint8_t * a , unsigned i ) {
	unsigned char r = a[i>>1];
	r = ( i&1 )? (r>>4):(r&0xf);
	return r;
}

static inline
unsigned char gf16v_set_ele( uint8_t * a , unsigned i , uint8_t v ) {
	unsigned char m = (i&1)? 0xf : 0xf0;
	a[i>>1] &= m;
	m = ( i&1 )? v<<4 : v&0xf;
	a[i>>1] |= m;
	return v;
}

static inline
void gf16v_split( uint8_t * z , const uint8_t * x , unsigned n )
{
	for(unsigned i=0;i<n;i++) z[i] = gf16v_get_ele( x , i );
}

static inline
unsigned char _gf16v_dot( const uint8_t *a , const uint8_t *b, unsigned _num_ele ){
	unsigned char r = 0;
	for(unsigned i=0;i<_num_ele;i++) {
		unsigned char _a = gf16v_get_ele(a,i);
		unsigned char _b = gf16v_get_ele(b,i);
		r ^= gf16_mul(_a,_b);
	}
	return r;
}

//static inline
//unsigned char gf256v_is_zero( const uint8_t * a , unsigned _num_byte ) {
//	unsigned char r = 0;
//	for(unsigned i=0;i<_num_byte;i++) r |= a[i];
//	return r==0;
//}

static inline
void gf16v_rand( uint8_t * a , unsigned _num_ele ) {
	if( 0 == _num_ele ) return;
	unsigned _num_byte = (_num_ele+1)/2;
	getRandBytes( a , _num_byte );
	if( _num_ele & 1 ) a[_num_byte-1] &= 0xf;
}

static inline
void gf256v_rand( uint8_t * a , unsigned _num_byte ) {
	getRandBytes( a , _num_byte );
}



static inline
void gf16mat_fdump(FILE * fp, const uint8_t *v, unsigned n_vec_byte , unsigned n_vec ) {
	for(unsigned i=0;i<n_vec;i++) {
		fprintf(fp,"[%d]",i);
		gf256v_fdump(fp,v,n_vec_byte);
		fprintf(fp,"\n");
		v += n_vec_byte;
	}
}

static inline
void gf256mat_fdump(FILE * fp, const uint8_t *v, unsigned n_vec_byte , unsigned n_vec ) { gf16mat_fdump(fp,v,n_vec_byte,n_vec); }


static inline
void _gf16mat_prod( uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * b ) {
	gf256v_set_zero(c,n_A_vec_byte);
	for(unsigned i=0;i<n_A_width;i++) {
		uint8_t bb = gf16v_get_ele( b , i );
		gf16v_madd( c , matA , bb , n_A_vec_byte );
		matA += n_A_vec_byte;
	}
}

static inline
void gf16mat_mul( uint8_t * c , const uint8_t * a , const uint8_t * b , unsigned len_vec ) {
	unsigned n_vec_byte = (len_vec+1)/2;
	for(unsigned k=0;k<len_vec;k++){
		gf256v_set_zero( c , n_vec_byte );
		const uint8_t * bk = b + n_vec_byte * k;
		for(unsigned i=0;i<len_vec;i++) {
			uint8_t bb = gf16v_get_ele( bk , i );
			gf16v_madd( c , a + n_vec_byte * i , bb , n_vec_byte  );
		}
		c += n_vec_byte;
	}
}

static inline
unsigned gf16mat_gauss_elim( uint8_t * mat , unsigned h , unsigned w )
{
	unsigned n_w_byte = (w+1)/2;
	unsigned r8 = 1;
	for(unsigned i=0;i<h;i++) {
		uint8_t * ai = mat + n_w_byte*i;
		for(unsigned j=i+1;j<h;j++) {
			uint8_t * aj = mat + n_w_byte*j;
			gf16v_madd( ai , aj , gf16_is_nonzero( gf16v_get_ele(ai,i) )^gf16_is_nonzero( gf16v_get_ele(aj,i) ) , n_w_byte );
		}
		uint8_t pivot = gf16v_get_ele( ai , i );
		r8 &= gf16_is_nonzero( pivot );
		pivot = gf16_inv( pivot );
		gf16v_mul_scalar( ai , pivot , n_w_byte );
		for(unsigned j=0;j<h;j++) {
			if(i==j) continue;
			uint8_t * aj = mat + n_w_byte*j;
			gf16v_madd( aj , ai , gf16v_get_ele( aj , i ) , n_w_byte );
		}
	}
	return r8;
}

static inline
void gf16mat_submat( uint8_t * mat2 , unsigned w2 , unsigned st , const uint8_t * mat , unsigned w , unsigned h )
{
	unsigned n_byte_w1 = (w+1)/2;
	unsigned n_byte_w2 = (w2+1)/2;
	unsigned st_2 = st/2;
	for(unsigned i=0;i<h;i++) {
		for(unsigned j=0;j<n_byte_w2;j++) mat2[i*n_byte_w2+j] = mat[i*n_byte_w1+st_2+j];
	}
}

static inline
void gf16mat_subcolumn( uint8_t * col , unsigned idx , const uint8_t * mat , unsigned w , unsigned h )
{
	unsigned n_byte_w = (w+1)/2;
	for(unsigned i=0;i<h;i++) {
		const uint8_t * mat_i = mat + n_byte_w*i;
		uint8_t qq = gf16v_get_ele( mat_i , idx );
		gf16v_set_ele( col , i , qq );
	}
}

static inline
unsigned gf16mat_rand_inv( uint8_t * a , uint8_t * b , unsigned H )
{
	unsigned n_w_byte = (H+1)/2;
	uint8_t * aa = (uint8_t *)malloc( H*n_w_byte*2 );
	unsigned k;
	for(k=0;k<100;k++){
		gf256v_set_zero( aa , H*n_w_byte*2 );
		//memset( aa , 0 , H*H*2 );
		for(unsigned i=0;i<H;i++){
			uint8_t * ai = aa + i*2*n_w_byte;
			gf16v_rand( ai , H );
			gf16v_set_ele( ai + n_w_byte , i , 1 );
		}
		gf16mat_submat( a , H , 0 , aa , 2*H , H );
		unsigned char r8 = gf16mat_gauss_elim( aa , H , 2*H );
		if( r8 ) {
			gf16mat_submat( b , H , H , aa , 2*H , H );
			break;
		}
	}
	free( aa );
	return (100!=k);
}




static inline
void gf256v_polymul( uint8_t * c, const uint8_t * a , const uint8_t * b , unsigned _num ) {
	for(unsigned i=0;i<_num*2-1;i++) c[i] = 0;
	for(unsigned i=0;i<_num;i++) _gf256v_madd( c+i , a , b[i] , _num );
}



static inline
void _gf256mat_prod( uint8_t * c , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * b ) {
	gf256v_set_zero(c,n_A_vec_byte);
	for(unsigned i=0;i<n_A_width;i++) {
		gf256v_madd( c , matA , b[i] , n_A_vec_byte );
		matA += n_A_vec_byte;
	}
}

static inline
void gf256mat_mul( uint8_t * c , const uint8_t * a , const uint8_t * b , unsigned len_vec ) {
	unsigned n_vec_byte = len_vec;
	for(unsigned k=0;k<len_vec;k++){
		gf256v_set_zero( c , n_vec_byte );
		const uint8_t * bk = b + n_vec_byte * k;
		for(unsigned i=0;i<len_vec;i++) {
			gf256v_madd( c , a + n_vec_byte * i , bk[i] , n_vec_byte  );
		}
		c += n_vec_byte;
	}
}


static inline
unsigned gf256mat_gauss_elim( uint8_t * mat , unsigned h , unsigned w )
{
	unsigned r8 = 1;
	for(unsigned i=0;i<h;i++) {
		uint8_t * ai = mat + w*i;
		for(unsigned j=i+1;j<h;j++) {
			uint8_t * aj = mat + w*j;
			gf256v_madd( ai , aj , gf256_is_nonzero(ai[i])^gf256_is_nonzero(aj[i]) , w );
		}
		r8 &= gf256_is_nonzero(ai[i]);
		uint8_t pivot = ai[i];
		pivot = gf256_inv( pivot );
		gf256v_mul_scalar( ai , pivot , w );
		for(unsigned j=0;j<h;j++) {
			if(i==j) continue;
			uint8_t * aj = mat + w*j;
			gf256v_madd( aj , ai , aj[i] , w );
		}
	}
	return r8;
}

static inline
void gf256mat_submat( uint8_t * mat2 , unsigned w2 , unsigned st , const uint8_t * mat , unsigned w , unsigned h )
{
	for(unsigned i=0;i<h;i++) {
		for(unsigned j=0;j<w2;j++) mat2[i*w2+j] = mat[i*w+st+j];
	}
}


static inline
unsigned gf256mat_rand_inv( uint8_t * a , uint8_t * b , unsigned H )
{
	uint8_t * aa = (uint8_t *)malloc( H*H*2 );
	unsigned k;
	for(k=0;k<100;k++){
		gf256v_set_zero( aa , H*H*2 );
		//memset( aa , 0 , H*H*2 );
		for(unsigned i=0;i<H;i++){
			uint8_t * ai = aa + i*2*H;
			gf256v_rand( ai , H );
			ai[H+i] = 1;
		}
		gf256mat_submat( a , H , 0 , aa , 2*H , H );
		unsigned char r8 = gf256mat_gauss_elim( aa , H , 2*H );
		if( r8 ) {
			gf256mat_submat( b , H , H , aa , 2*H , H );
			break;
		}
	}
	free( aa );
	return (100!=k);
}




#ifdef  __cplusplus
}
#endif



#endif

