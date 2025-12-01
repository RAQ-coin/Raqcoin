#ifndef _GF16_H_
#define _GF16_H_

#include <stdint.h>


//#include "gf16_tabs.h"

#ifdef  __cplusplus
extern  "C" {
#endif


typedef unsigned sto_t;


// gf4 := gf2[x]/x^2+x+1
static inline unsigned char gf4_mul_2( unsigned char a )
{
#if 0
/// parallel ver.
	unsigned char bit0 = a&0x55;
	unsigned char bit1 = a&0xaa;
	return (bit0<<1)^bit1^(bit>>1);
#else
/// single ver.
	unsigned char r = a<<1;
	r ^= (a>>1)*7;
	return r;
#endif
}

static inline unsigned char gf4_mul_3( unsigned char a )
{
	unsigned char msk = (a-2)>>1;
	return (msk&(a*3)) | ((~msk)&(a-1));
}

static inline unsigned char gf16_is_nonzero( unsigned char a )
{
	unsigned a4 = a&0xf;
	unsigned r = ((unsigned)0)-a4;
	r >>= 8;
	return r&1;
}

static inline unsigned char gf4_mul( unsigned char a , unsigned char b )
{
	unsigned char r = a*(b&1);
	return r ^ (gf4_mul_2(a)*(b>>1));
}

static inline unsigned char gf4_squ( unsigned char a )
{
	return a^(a>>1);
}

static inline unsigned char gf4_inv( unsigned char a )
{
	return a^(a>>1);
}

//////

// gf4 := gf2[x]/x^2+x+1
static inline void bs_gf4_mul_2( sto_t * r , const sto_t * a )
{
	sto_t r0 = a[1];
	sto_t r1 = a[0]^a[1];
	r[0] = r0;
	r[1] = r1;
}

static inline void bs_gf4_mul_3( sto_t * r , const sto_t * a )
{
	sto_t r0 = a[0]^a[1];
	sto_t r1 = a[0];
	r[0] = r0;
	r[1] = r1;
}

static inline void bs_gf4_mul( sto_t * r , const sto_t * a , const sto_t * b )
{
	sto_t t = a[1]&b[1];
	sto_t r0 = (a[0]&b[0])^t;
	sto_t r1 = (a[0]&b[1])^(a[1]&b[0])^t;
	r[0] = r0;
	r[1] = r1;
}

static inline void bs_gf4_squ( sto_t * r , const sto_t * a )
{
	sto_t r0 = a[0]^a[1];
	sto_t r1 = a[1];
	r[0] = r0;
	r[1] = r1;
}

static inline void bs_gf4_inv( sto_t * r , const sto_t * a )
{
	sto_t r0 = a[0]^a[1];
	sto_t r1 = a[1];
	r[0] = r0;
	r[1] = r1;
}




//////////////////////////////////////////////////////////////////////////////////





// gf16 := gf4[y]/y^2+y+x
static inline unsigned char gf16_mul( unsigned char a , unsigned char b )
{
	unsigned char a0 = a&3;
	unsigned char a1 = (a>>2);
	unsigned char b0 = b&3;
	unsigned char b1 = (b>>2);
	unsigned char a0b0 = gf4_mul( a0 , b0 );
	unsigned char a1b1 = gf4_mul( a1 , b1 );
	unsigned char a0b1_a1b0 = gf4_mul( a0^a1 , b0^b1 ) ^ a0b0 ^ a1b1;
	unsigned char a1b1_x2 = gf4_mul_2( a1b1 );
	return ((a0b1_a1b0^a1b1)<<2) ^ a0b0 ^ a1b1_x2;
}

static inline unsigned char gf16_squ( unsigned char a )
{
	unsigned char a0 = a&3;
	unsigned char a1 = (a>>2);
	a1 = gf4_squ(a1);
	unsigned char a1squ_x2 = gf4_mul_2( a1 );
	return (a1<<2)^a1squ_x2^gf4_squ(a0);
}

static inline unsigned char gf16_inv( unsigned char a )
{
	unsigned char a2 = gf16_squ(a);
	unsigned char a4 = gf16_squ(a2);
	unsigned char a8 = gf16_squ(a4);
	unsigned char a6 = gf16_mul( a4 , a2 );
	return gf16_mul( a8 , a6 );
}

static inline unsigned char gf16_mul_4( unsigned char a )
{
	return (((a<<2)^a)&(8+4))^gf4_mul_2( a>>2 );
}

static inline unsigned char gf16_mul_8( unsigned char a )
{
	unsigned char a0 = a&3;
	unsigned char a1 = a>>2;
	return (gf4_mul_2(a0^a1)<<2)|gf4_mul_3(a1);
}

////////////

// gf16 := gf4[y]/y^2+y+x

static inline void bs_gf16_mul( sto_t * r , const sto_t * a , const sto_t * b )
{
	sto_t r0[2];
	sto_t r1[2];
	sto_t r2[2];
	bs_gf4_mul( r0 , a , b );
	bs_gf4_mul( r2 , a+2 , b+2 );
	sto_t a01[2]; a01[0] = a[0]^a[2]; a01[1] = a[1]^a[3];
	sto_t b01[2]; b01[0] = b[0]^b[2]; b01[1] = b[1]^b[3];
	bs_gf4_mul( r1 , a01 , b01 );
	r1[0] ^= r0[0];
	r1[1] ^= r0[1];
	/// r0 ^= r2x2
	r0[0] ^= r2[1];
	r0[1] ^= r2[0]^r2[1];
	r[0] = r0[0];
	r[1] = r0[1];
	r[2] = r1[0];
	r[3] = r1[1];
}

static inline void bs_gf16_squ( sto_t * r , const sto_t * a )
{
	sto_t r0[2];
	sto_t r1[2];
	bs_gf4_squ( r0 , a );
	bs_gf4_squ( r1 , a+2 );
	/// r0 ^= r2x2
	r0[0] ^= r1[1];
	r0[1] ^= r1[0]^r1[1];
	r[0] = r0[0];
	r[1] = r0[1];
	r[2] = r1[0];
	r[3] = r1[1];
}

static inline void bs_gf16_inv( sto_t * r , const sto_t * a )
{
	sto_t a2[4]; bs_gf16_squ( a2 , a );
	sto_t a4[4]; bs_gf16_squ( a4 , a2 );
	sto_t a8[4]; bs_gf16_squ( a8 , a4 );
	bs_gf16_mul( a2 , a2 , a4 ); /// a6
	bs_gf16_mul( r , a8 , a2 );
}

static inline void bs_gf16_mul_4( sto_t * r , const sto_t * a )
{
	sto_t r1[2];
	r1[0] = a[0]^a[2];
	r1[1] = a[1]^a[3];
	bs_gf4_mul_2( r , a+2 );
	r[2] = r1[0];
	r[3] = r1[1];
}

static inline void bs_gf16_mul_8( sto_t * r , const sto_t * a )
{
	sto_t r1[2]; r1[0] = a[0]^a[2]; r1[1] = a[1]^a[3];
	bs_gf4_mul_2( r1 , r1 );
	bs_gf4_mul_3( r , a+2 );
	r[2] = r1[0];
	r[3] = r1[1];
}

////////////

static inline unsigned char gf256_is_nonzero( unsigned char a )
{
	unsigned a8 = a;
	unsigned r = ((unsigned)0)-a8;
	r >>= 8;
	return r&1;
}


// gf256 := gf16[X]/X^2+X+xy
static inline unsigned char gf256_mul( unsigned char a , unsigned char b )
{
	unsigned char a0 = a&15;
	unsigned char a1 = (a>>4);
	unsigned char b0 = b&15;
	unsigned char b1 = (b>>4);
	unsigned char a0b0 = gf16_mul( a0 , b0 );
	unsigned char a1b1 = gf16_mul( a1 , b1 );
	unsigned char a0b1_a1b0 = gf16_mul( a0^a1 , b0^b1 ) ^ a0b0 ^ a1b1;
	unsigned char a1b1_x8 = gf16_mul_8( a1b1 );
	return ((a0b1_a1b0^a1b1)<<4) ^ a0b0 ^ a1b1_x8;
}

static inline unsigned char gf256_mul_0x2( unsigned char a )
{
	unsigned char bit0 = a&0x55;
	unsigned char bit1 = a&0xaa;
	return (bit0<<1)^bit1^(bit1>>1);
}


static inline unsigned char gf256_mul_0x4( unsigned char a )
{
	unsigned char a0 = a&0xf;
	unsigned char a1 = (a>>4);
	unsigned char r0 = gf16_mul_4(a0);
	unsigned char r1 = gf16_mul_4(a1);
	return r0^(r1<<4);
}

static inline unsigned char gf256_mul_0x10( unsigned char a )
{
	unsigned char a1 = (a>>4);
	unsigned char rd0 = gf16_mul_8(a1);
	unsigned char a0 = a<<4;
	unsigned char _a1 = a&0xf0;
	return a0^_a1^rd0;
}


static inline unsigned char gf256_mul_0x80( unsigned char a )
{
	unsigned char a0 = a&0xf;
	unsigned char a1 = (a>>4);
	unsigned char a0x8 = gf16_mul_8(a0);
	unsigned char a1x8 = gf16_mul_8(a1);
/// 0x8*0x8 = 0xd
	unsigned char a1x8x8 = gf16_mul_8(a1x8);
	return a1x8x8^((a0x8^a1x8)<<4);

}

static inline unsigned char gf256_mul_gf16( unsigned char a , unsigned char gf16_b )
{
	unsigned char a0 = a&15;
	unsigned char a1 = (a>>4);
	unsigned char b0 = gf16_b&15;
	unsigned char a0b0 = gf16_mul( a0 , b0 );
	unsigned char a1b0 = gf16_mul( a1 , b0 );
	return a0b0^(a1b0<<4);
}


static inline unsigned char gf256_squ( unsigned char a )
{
	unsigned char a0 = a&15;
	unsigned char a1 = (a>>4);
	a1 = gf16_squ(a1);
	unsigned char a1squ_x8 = gf16_mul_8( a1 );
	return (a1<<4)^a1squ_x8^gf16_squ(a0);
}

static inline unsigned char gf256_inv( unsigned char a )
{
// 128+64+32+16+8+4+2 = 254
	unsigned char a2 = gf256_squ(a);
	unsigned char a4 = gf256_squ(a2);
	unsigned char a8 = gf256_squ(a4);
	unsigned char a4_2 = gf256_mul( a4 , a2 );
	unsigned char a8_4_2 = gf256_mul( a4_2 , a8 );
	unsigned char a64_ = gf256_squ( a8_4_2 );
	a64_ = gf256_squ( a64_ );
	a64_ = gf256_squ( a64_ );
	unsigned char a64_2 = gf256_mul( a64_ , a8_4_2 );
	unsigned char a128_ = gf256_squ( a64_2 );
	return gf256_mul( a2 , a128_ );

}

////////

// gf256 := gf16[X]/X^2+X+xy
static inline void bs_gf256_mul( sto_t * r , const sto_t * a , const sto_t * b )
{
	sto_t r0[4];
	//sto_t r1[4];
	sto_t r2[4];
	bs_gf16_mul( r0 , a , b );
	bs_gf16_mul( r2 , a+4 , b+4 );
	sto_t a01[4];
	a01[0] = a[0]^a[4]; a01[1] = a[1]^a[5];
	a01[2] = a[2]^a[6]; a01[3] = a[3]^a[7];
	sto_t b01[4];
	b01[0] = b[0]^b[4]; b01[1] = b[1]^b[5];
	b01[2] = b[2]^b[6]; b01[3] = b[3]^b[7];
	bs_gf16_mul( r+4 , a01 , b01 );
	r[4] ^= r0[0];
	r[5] ^= r0[1];
	r[6] ^= r0[2];
	r[7] ^= r0[3];
	/// r0 ^= r2x2
	bs_gf16_mul_8( r2 , r2 );
	r[0] = r0[0]^r2[0];
	r[1] = r0[1]^r2[1];
	r[2] = r0[2]^r2[2];
	r[3] = r0[3]^r2[3];
}

#if 1

static inline void bs_gf16_set_value( sto_t * regs , unsigned char val )
{
	int i;
	for(i=0;i<4;i++ ) {
		regs[i] = 0-(val&0x1);
		val >>= 1;
	}
}

static inline void bs_gf256_set_value( sto_t * regs , unsigned char val )
{
	int i;
	for(i=0;i<8;i++ ) {
		regs[i] = 0-(val&0x1);
		val >>= 1;
	}
}


static inline unsigned char bs_gf16_get_1st_value( const sto_t * regs )
{
	int i;
	unsigned char ret = regs[3]&0x1;
	for(i=2;i>=0;i--) {
		ret <<= 1;
		ret |= (regs[i]&0x1);
	}
	return ret;
}

static inline unsigned char bs_gf256_get_1st_value( const sto_t * regs )
{
	int i;
	unsigned char ret = regs[7]&0x1;
	for(i=6;i>=0;i--) {
		ret <<= 1;
		ret |= (regs[i]&0x1);
	}
	return ret;
}

static inline void bs_gf4_mul_0x2( sto_t * a ) {
	sto_t t = a[1];
	a[1] ^= a[0];
	a[0] = t;
}
static inline void bs_gf4_mul_0x3( sto_t * a ) {
	sto_t t = a[1];
	a[1] = a[0];
	a[0] ^= t;
}

static inline void bs_gf16_mul_0x2( sto_t * a ) {
	bs_gf4_mul_0x2(a);
	bs_gf4_mul_0x2(a+2);
}
static inline void bs_gf16_mul_0x3( sto_t * a ) {
	bs_gf4_mul_0x3(a);
	bs_gf4_mul_0x3(a+2);
}
static inline void bs_gf16_mul_0x4( sto_t * a ){
	sto_t t[2];
	t[0] = a[2];
	t[1] = a[3];
	a[2] ^= a[0];
	a[3] ^= a[1];
	a[0] = t[0];
	a[1] = t[1];
	bs_gf4_mul_0x2( a );
}
static inline void bs_gf16_mul_0x5( sto_t * a ){
	sto_t t[2];
	t[0] = a[2];
	t[1] = a[3];
	a[2] = a[0];
	a[3] = a[1];
	bs_gf4_mul_0x2( t );
	a[0] ^= t[0];
	a[1] ^= t[1];
}
static inline void bs_gf16_mul_0x6( sto_t * a ){
#if 1
	a[0] ^= a[2];
	a[1] ^= a[3];
	bs_gf4_mul_0x2(a+2);
	a[2] ^= a[0];
	a[3] ^= a[1];
	bs_gf4_mul_0x2(a);
#else
	sto_t t[2];
	t[0] = a[0];
	t[1] = a[1];
	a[0] ^= a[2];
	a[1] ^= a[3];
	bs_gf4_mul_0x2( a );
	bs_gf4_mul_0x3( a+2 );
	a[2] ^= t[0];
	a[3] ^= t[1];
#endif
}
static inline void bs_gf16_mul_0x7( sto_t * a ){
	sto_t t[2];
	t[0] = a[0];
	t[1] = a[1];
	bs_gf4_mul_0x3( a );
	bs_gf4_mul_0x2( a+2 );
	a[0] ^= a[2];
	a[1] ^= a[3];
	a[2] ^= t[0];
	a[3] ^= t[1];
}
static inline void bs_gf16_mul_0x8( sto_t * a ){
	sto_t t[2];
	t[0] = a[2];
	t[1] = a[3];
	a[2] ^= a[0];
	a[3] ^= a[1];
	a[0] = t[0];
	a[1] = t[1];
	bs_gf4_mul_0x3( a );
	bs_gf4_mul_0x2( a+2 );
}
/* XXX */
static inline void bs_gf16_mul_0x9( sto_t * a ){
	sto_t t[4];
	t[0] = a[0]; t[1] = a[1]; t[2] = a[2]; t[3] = a[3];
	bs_gf16_mul_0x8( a );
	a[0] ^= t[0];
	a[1] ^= t[1];
	a[2] ^= t[2];
	a[3] ^= t[3];
}
/* XXX */
static inline void bs_gf16_mul_0xa( sto_t * a ){
	sto_t t[4];
	t[0] = a[0]; t[1] = a[1]; t[2] = a[2]; t[3] = a[3];
	bs_gf16_mul_0x8( a );
	bs_gf16_mul_0x2( t );
	a[0] ^= t[0];
	a[1] ^= t[1];
	a[2] ^= t[2];
	a[3] ^= t[3];
}
/* XXX */
static inline void bs_gf16_mul_0xb( sto_t * a ){
	sto_t t[4];
	t[0] = a[0]; t[1] = a[1]; t[2] = a[2]; t[3] = a[3];
	bs_gf16_mul_0x8( a );
	bs_gf16_mul_0x3( t );
	a[0] ^= t[0];
	a[1] ^= t[1];
	a[2] ^= t[2];
	a[3] ^= t[3];
}
/* XXX */
static inline void bs_gf16_mul_0xc( sto_t * a ){
	sto_t t[4];
	t[0] = a[0]; t[1] = a[1]; t[2] = a[2]; t[3] = a[3];
	bs_gf16_mul_0x8( a );
	bs_gf16_mul_0x4( t );
	a[0] ^= t[0];
	a[1] ^= t[1];
	a[2] ^= t[2];
	a[3] ^= t[3];
}
/* XXX */
static inline void bs_gf16_mul_0xe( sto_t * a ){
	sto_t t[4];
	t[0] = a[0]; t[1] = a[1]; t[2] = a[2]; t[3] = a[3];
	bs_gf16_mul_0x8( a );
	bs_gf16_mul_0x6( t );
	a[0] ^= t[0];
	a[1] ^= t[1];
	a[2] ^= t[2];
	a[3] ^= t[3];
}
static inline void bs_gf256_mul_0x2( sto_t * r , const sto_t * a )
{
	r[0] = a[0]; r[1] = a[1]; r[2] = a[2]; r[3] = a[3];
	r[4] = a[4]; r[5] = a[5]; r[6] = a[6]; r[7] = a[7];
	bs_gf16_mul_0x2( r );
	bs_gf16_mul_0x2( r+4 );
}
static inline void bs_gf256_mul_0x3( sto_t * r , const sto_t * a )
{
	r[0] = a[0]; r[1] = a[1]; r[2] = a[2]; r[3] = a[3];
	r[4] = a[4]; r[5] = a[5]; r[6] = a[6]; r[7] = a[7];
	bs_gf16_mul_0x3( r );
	bs_gf16_mul_0x3( r+4 );
}
static inline void bs_gf256_mul_0x4( sto_t * r , const sto_t * a )
{
	r[0] = a[0]; r[1] = a[1]; r[2] = a[2]; r[3] = a[3];
	r[4] = a[4]; r[5] = a[5]; r[6] = a[6]; r[7] = a[7];
	bs_gf16_mul_0x4( r );
	bs_gf16_mul_0x4( r+4 );
}
static inline void bs_gf256_mul_0x5( sto_t * r , const sto_t * a )
{
	r[0] = a[0]; r[1] = a[1]; r[2] = a[2]; r[3] = a[3];
	r[4] = a[4]; r[5] = a[5]; r[6] = a[6]; r[7] = a[7];
	bs_gf16_mul_0x5( r );
	bs_gf16_mul_0x5( r+4 );
}
static inline void bs_gf256_mul_0x6( sto_t * r , const sto_t * a )
{
	r[0] = a[0]; r[1] = a[1]; r[2] = a[2]; r[3] = a[3];
	r[4] = a[4]; r[5] = a[5]; r[6] = a[6]; r[7] = a[7];
	bs_gf16_mul_0x6( r );
	bs_gf16_mul_0x6( r+4 );
}
static inline void bs_gf256_mul_0x7( sto_t * r , const sto_t * a )
{
	r[0] = a[0]; r[1] = a[1]; r[2] = a[2]; r[3] = a[3];
	r[4] = a[4]; r[5] = a[5]; r[6] = a[6]; r[7] = a[7];
	bs_gf16_mul_0x7( r );
	bs_gf16_mul_0x7( r+4 );
}
static inline void bs_gf256_mul_0x8( sto_t * r , const sto_t * a )
{
	r[0] = a[0]; r[1] = a[1]; r[2] = a[2]; r[3] = a[3];
	r[4] = a[4]; r[5] = a[5]; r[6] = a[6]; r[7] = a[7];
	bs_gf16_mul_0x8( r );
	bs_gf16_mul_0x8( r+4 );
}
static inline void bs_gf256_mul_0x9( sto_t * r , const sto_t * a )
{
	r[0] = a[0]; r[1] = a[1]; r[2] = a[2]; r[3] = a[3];
	r[4] = a[4]; r[5] = a[5]; r[6] = a[6]; r[7] = a[7];
	bs_gf16_mul_0x9( r );
	bs_gf16_mul_0x9( r+4 );
}
static inline void bs_gf256_mul_0xa( sto_t * r , const sto_t * a )
{
	sto_t t[8];
	int i;
	bs_gf256_mul_0x2( t , a );
	bs_gf256_mul_0x8( r , a );
	for(i=0;i<8;i++) r[i] ^= t[i];
}
static inline void bs_gf256_mul_0xb( sto_t * r , const sto_t * a )
{
	r[0] = a[0]; r[1] = a[1]; r[2] = a[2]; r[3] = a[3];
	r[4] = a[4]; r[5] = a[5]; r[6] = a[6]; r[7] = a[7];
	bs_gf16_mul_0xb( r );
	bs_gf16_mul_0xb( r+4 );
}
static inline void bs_gf256_mul_0xc( sto_t * r , const sto_t * a )
{
	sto_t t[8];
	int i;
	bs_gf256_mul_0x4( t , a );
	bs_gf256_mul_0x8( r , a );
	for(i=0;i<8;i++) r[i] ^= t[i];
}
static inline void bs_gf256_mul_0xd( sto_t * r , const sto_t * a )
{
	sto_t t[8];
	int i;
	bs_gf256_mul_0x5( t , a );
	bs_gf256_mul_0x8( r , a );
	for(i=0;i<8;i++) r[i] ^= t[i];
}
static inline void bs_gf256_mul_0xe( sto_t * r , const sto_t * a )
{
	sto_t t[8];
	int i;
	bs_gf256_mul_0x6( t , a );
	bs_gf256_mul_0x8( r , a );
	for(i=0;i<8;i++) r[i] ^= t[i];
}
static inline void bs_gf256_mul_0xf( sto_t * r , const sto_t * a )
{
	sto_t t[8];
	int i;
	bs_gf256_mul_0x7( t , a );
	bs_gf256_mul_0x8( r , a );
	for(i=0;i<8;i++) r[i] ^= t[i];
}
static inline void bs_gf256_mul_0x10( sto_t * r , const sto_t * a )
{
	sto_t t[8];
	int i;
	for(i=0;i<8;i++) t[i] = a[i];
	t[0] ^= t[4];
	t[1] ^= t[5];
	t[2] ^= t[6];
	t[3] ^= t[7];
	bs_gf16_mul_0x8( t + 4 );
	r[0] = t[4];
	r[1] = t[5];
	r[2] = t[6];
	r[3] = t[7];
	r[4] = t[0];
	r[5] = t[1];
	r[6] = t[2];
	r[7] = t[3];
}
static inline void bs_gf256_mul_0x11( sto_t * r , const sto_t * a )
{
	sto_t t[8];
	int i;
	for(i=0;i<8;i++) t[i] = a[i];
	bs_gf256_mul_0x10( r , a );
	for(i=0;i<8;i++) r[i] ^= t[i];
}
static inline void bs_gf256_mul_0x12( sto_t * r , const sto_t * a )
{
	sto_t t[8];
	int i;
	bs_gf256_mul_0x2( t , a );
	bs_gf256_mul_0x10( r , a );
	for(i=0;i<8;i++) r[i] ^= t[i];
}

#define Fn_BS_256_MUL(n0,n1,n2) \
static inline void bs_gf256_mul_0x ## n0 ( sto_t * r , const sto_t * a ) { \
	sto_t t[8]; int i; \
	bs_gf256_mul_0x ## n2  ( t , a ); \
	bs_gf256_mul_0x ## n1 ( r , a ); \
	for(i=0;i<8;i++) r[i] ^= t[i]; \
}

Fn_BS_256_MUL(13,10,3)
Fn_BS_256_MUL(14,10,4)
Fn_BS_256_MUL(16,10,6)
Fn_BS_256_MUL(18,10,8)
Fn_BS_256_MUL(19,10,9)
Fn_BS_256_MUL(1a,10,a)
Fn_BS_256_MUL(1b,10,b)
Fn_BS_256_MUL(1c,10,c)
Fn_BS_256_MUL(1d,10,d)
Fn_BS_256_MUL(1e,10,e)


static inline void bs_gf256_mul_0x20( sto_t * r , const sto_t * a )
{
	sto_t t[8];
	int i;
	for(i=0;i<8;i++) t[i] = a[i];
	t[0] ^= t[4];
	t[1] ^= t[5];
	t[2] ^= t[6];
	t[3] ^= t[7];
	bs_gf16_mul_0x2( t );
	bs_gf16_mul_0xc( t + 4 );
	r[0] = t[4];
	r[1] = t[5];
	r[2] = t[6];
	r[3] = t[7];
	r[4] = t[0];
	r[5] = t[1];
	r[6] = t[2];
	r[7] = t[3];
}


Fn_BS_256_MUL(22,20,2)
Fn_BS_256_MUL(24,20,4)
Fn_BS_256_MUL(26,20,6)
Fn_BS_256_MUL(28,20,8)
Fn_BS_256_MUL(2a,20,a)
Fn_BS_256_MUL(2c,20,c)
Fn_BS_256_MUL(2e,20,e)


static inline void bs_gf256_mul_0x30( sto_t * r , const sto_t * a )
{
	sto_t t[8];
	int i;
	bs_gf256_mul_0x20( t , a );
	bs_gf256_mul_0x10( r , a );
	for(i=0;i<8;i++) r[i] ^= t[i];
}

Fn_BS_256_MUL(32,30,2)
Fn_BS_256_MUL(34,30,4)
Fn_BS_256_MUL(36,30,6)
Fn_BS_256_MUL(38,30,8)
Fn_BS_256_MUL(3a,30,a)
Fn_BS_256_MUL(3c,30,c)
Fn_BS_256_MUL(3e,30,e)



#endif


static inline void bs_gf256_squ( sto_t * r , const sto_t * a )
{
	sto_t r0[4];
	sto_t r2[4];
	bs_gf16_squ( r0 , a );
	bs_gf16_squ( r+4 , a+4 );
	/// r0 ^= r2x2
	bs_gf16_mul_8( r2 , r+4 );
	r[0] = r0[0]^r2[0];
	r[1] = r0[1]^r2[1];
	r[2] = r0[2]^r2[2];
	r[3] = r0[3]^r2[3];
}

static inline void bs_gf256_inv( sto_t * r , const sto_t * a )
{
// 128+64+32+16+8+4+2 = 254
	sto_t a2[8]; bs_gf256_squ( a2 , a );
	sto_t a4[8]; bs_gf256_squ( a4 , a2 );
	sto_t a8[8]; bs_gf256_squ( a8 , a4 );
	bs_gf256_mul( a4 , a4 , a2 ); /// a4_2
	bs_gf256_mul( a8 , a8 , a4 ); /// a8_4_2
	bs_gf256_squ( a4 , a8 ); /// a64_
	bs_gf256_squ( a4 , a4 ); /// a64_
	bs_gf256_squ( a4 , a4 ); /// a64_
	bs_gf256_mul( a4 , a4 , a8 );
	bs_gf256_squ( a4 , a4 ); /// a128_
	bs_gf256_mul( r , a4 , a2 );
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

//
// gf4 := gf2[x]/x^2+x+1
//

static inline uint64_t gf4_mul_2_u64( uint64_t a )
{
	uint64_t bit0 = a&_0x55;
	uint64_t bit1 = a&_0xaa;
	return (bit0<<1)^bit1^(bit1>>1);
}

static inline uint64_t gf4_mul_3_u64( uint64_t a )
{
	uint64_t bit0 = a&_0x55;
	uint64_t bit1 = a&_0xaa;
	return (bit0<<1)^bit0^(bit1>>1);
}


static inline uint64_t gf4_mul_u64( uint64_t a , unsigned char b )
{
	uint64_t r = a*( (uint64_t)(b&1) );
	return r ^ (gf4_mul_2_u64(a)*(  (uint64_t)(b>>1) ));
}

static inline uint64_t gf4_squ_u64( uint64_t a )
{
	uint64_t a1 = a&_0xaa;
	return a^(a1>>1);
}


//
// gf16 := gf4[y]/y^2+y+x
//

static inline uint64_t gf16_mul_u64( uint64_t a , unsigned char b )
{
	uint64_t a0 = a&_0x33;
	uint64_t a1 = (a>>2)&_0x33;
	unsigned char b0 = b&3;
	unsigned char b1 = (b>>2);

	uint64_t a0b0 = gf4_mul_u64( a0 , b0 );
	uint64_t a1b1 = gf4_mul_u64( a1 , b1 );
	uint64_t a0b1_a1b0 = gf4_mul_u64( a0^a1 , b0^b1 ) ^ a0b0 ^ a1b1;
	uint64_t a1b1_x2 = gf4_mul_2_u64( a1b1 );
	return ((a0b1_a1b0^a1b1)<<2) ^ a0b0 ^ a1b1_x2;
}

static inline uint64_t gf16_squ_u64( uint64_t a )
{
	uint64_t a0 = a&_0x33;
	uint64_t a1 = (a>>2)&_0x33;

	a1 = gf4_squ_u64(a1);
	uint64_t a1squ_x2 = gf4_mul_2_u64( a1 );
	return (a1<<2)^a1squ_x2^gf4_squ_u64(a0);
}

static inline uint64_t gf16_mul_8_u64( uint64_t a )
{
	uint64_t a0 = a&_0x33;
	uint64_t a1 = (a>>2)&_0x33;

	return (gf4_mul_2_u64(a0^a1)<<2)|gf4_mul_3_u64(a1);
}


//
// gf256 := gf16[X]/X^2+X+xy
//

static inline uint64_t gf256_mul_u64( uint64_t a , unsigned char b )
{
	uint64_t a0 = a&_0x0f;
	uint64_t a1 = (a>>4)&_0x0f;
	unsigned char b0 = b&15;
	unsigned char b1 = (b>>4);

	uint64_t a0b0 = gf16_mul_u64( a0 , b0 );
	uint64_t a1b1 = gf16_mul_u64( a1 , b1 );
	uint64_t a0b1_a1b0 = gf16_mul_u64( a0^a1 , b0^b1 ) ^ a0b0 ^ a1b1;
	uint64_t a1b1_x8 = gf16_mul_8_u64( a1b1 );
	return ((a0b1_a1b0^a1b1)<<4) ^ a0b0 ^ a1b1_x8;
}

static inline uint64_t gf256_mul_0x10_u64( uint64_t a )
{
	uint64_t a1 = a&_0xf0;
	uint64_t a0 = (a&_0x0f)<<4;

	uint64_t a1_sr4 = a1>>4;
	uint64_t rd0 = gf16_mul_8_u64(a1_sr4);
	return a0^a1^rd0;
}

static inline uint64_t gf256_squ_u64( uint64_t a )
{
	uint64_t a0 = a&_0x0f;
	uint64_t a1 = (a>>4)&_0x0f;

	a1 = gf16_squ_u64(a1);
	uint64_t a1squ_x8 = gf16_mul_8_u64( a1 );
	return (a1<<4)^a1squ_x8^gf16_squ_u64(a0);
}


#ifdef  __cplusplus
}
#endif


#endif
