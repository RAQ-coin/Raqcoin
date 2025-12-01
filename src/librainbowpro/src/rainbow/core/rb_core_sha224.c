#include <stdlib.h>
#include <string.h>

#include "rb_core_sha224.h"

#define CONFIG_SHA256_SMALLER 1

/*
 * 32-bit int32_teger manipulation macros (big endian)
 */
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n,b,i)                            \
do {                                                    \
    (n) = ( (uint32_t) (b)[(i)    ] << 24 )             \
        | ( (uint32_t) (b)[(i) + 1] << 16 )             \
        | ( (uint32_t) (b)[(i) + 2] <<  8 )             \
        | ( (uint32_t) (b)[(i) + 3]       );            \
} while( 0 )
#endif

#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i)                            \
do {                                                    \
    (b)[(i)    ] = (uint8_t) ( (n) >> 24 );       \
    (b)[(i) + 1] = (uint8_t) ( (n) >> 16 );       \
    (b)[(i) + 2] = (uint8_t) ( (n) >>  8 );       \
    (b)[(i) + 3] = (uint8_t) ( (n)       );       \
} while( 0 )
#endif

static const uint32_t K[] =
{
    0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
    0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
    0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
    0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
    0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
    0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
    0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
    0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
    0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
    0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
    0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
    0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
    0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
    0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
    0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
    0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2,
};

#define  SHR(x,n) (((x) & 0xFFFFFFFF) >> (n))
#define ROTR(x,n) (SHR(x,n) | ((x) << (32 - (n))))

#define S0(x) (ROTR(x, 7) ^ ROTR(x,18) ^  SHR(x, 3))
#define S1(x) (ROTR(x,17) ^ ROTR(x,19) ^  SHR(x,10))

#define S2(x) (ROTR(x, 2) ^ ROTR(x,13) ^ ROTR(x,22))
#define S3(x) (ROTR(x, 6) ^ ROTR(x,11) ^ ROTR(x,25))

#define F0(x,y,z) (((x) & (y)) | ((z) & ((x) | (y))))
#define F1(x,y,z) ((z) ^ ((x) & ((y) ^ (z))))

#define R(t)                                                        \
    (                                                               \
        local.W[t] = S1(local.W[(t) -  2]) + local.W[(t) -  7] +    \
                     S0(local.W[(t) - 15]) + local.W[(t) - 16]      \
    )

#define P(a,b,c,d,e,f,g,h,x,K)                                      \
    do                                                              \
    {                                                               \
        local.temp1 = (h) + S3(e) + F1((e),(f),(g)) + (K) + (x);    \
        local.temp2 = S2(a) + F0((a),(b),(c));                      \
        (d) += local.temp1; (h) = local.temp1 + local.temp2;        \
    } while( 0 )

/*
 * SHA-224/256 context setup
 */
void crypto_sha224_sha256_init(rb_sha224_ctx_t *ctx, int32_t is_224 )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    if( is_224 == 0 ) {
        /* SHA-256 */
        ctx->state[0] = 0x6A09E667;
        ctx->state[1] = 0xBB67AE85;
        ctx->state[2] = 0x3C6EF372;
        ctx->state[3] = 0xA54FF53A;
        ctx->state[4] = 0x510E527F;
        ctx->state[5] = 0x9B05688C;
        ctx->state[6] = 0x1F83D9AB;
        ctx->state[7] = 0x5BE0CD19;
    } else {
        /* SHA-224 */
        ctx->state[0] = 0xC1059ED8;
        ctx->state[1] = 0x367CD507;
        ctx->state[2] = 0x3070DD17;
        ctx->state[3] = 0xF70E5939;
        ctx->state[4] = 0xFFC00B31;
        ctx->state[5] = 0x68581511;
        ctx->state[6] = 0x64F98FA7;
        ctx->state[7] = 0xBEFA4FA4;
    }

    ctx->is_224 = is_224;
}

void rb_crypto_sha224_init(rb_sha224_ctx_t *ctx )
{
    memset( ctx, 0, sizeof(rb_sha224_ctx_t ) );

    crypto_sha224_sha256_init(ctx, 1);
}

static int32_t local_sha224_process(rb_sha224_ctx_t *ctx,
                                const uint8_t data[64] )
{
    struct {
        uint32_t temp1, temp2, W[64];
        uint32_t A[8];
    } local;

    uint32_t i;

    for( i = 0; i < 8; i++ ) {
        local.A[i] = ctx->state[i];
    }

#if defined(CONFIG_SHA256_SMALLER)
    for( i = 0; i < 64; i++ ) {
        if( i < 16 ) {
            GET_UINT32_BE( local.W[i], data, 4 * i );
        } else {
            R( i );
        }

        P( local.A[0], local.A[1], local.A[2], local.A[3], local.A[4],
           local.A[5], local.A[6], local.A[7], local.W[i], K[i] );

        local.temp1 = local.A[7]; local.A[7] = local.A[6];
        local.A[6] = local.A[5]; local.A[5] = local.A[4];
        local.A[4] = local.A[3]; local.A[3] = local.A[2];
        local.A[2] = local.A[1]; local.A[1] = local.A[0];
        local.A[0] = local.temp1;
    }
#else /* CONFIG_SHA256_SMALLER */
    for( i = 0; i < 16; i++ ) {
        GET_UINT32_BE( local.W[i], data, 4 * i );
    }

    for( i = 0; i < 16; i += 8 ) {
        P( local.A[0], local.A[1], local.A[2], local.A[3], local.A[4],
           local.A[5], local.A[6], local.A[7], local.W[i+0], K[i+0] );
        P( local.A[7], local.A[0], local.A[1], local.A[2], local.A[3],
           local.A[4], local.A[5], local.A[6], local.W[i+1], K[i+1] );
        P( local.A[6], local.A[7], local.A[0], local.A[1], local.A[2],
           local.A[3], local.A[4], local.A[5], local.W[i+2], K[i+2] );
        P( local.A[5], local.A[6], local.A[7], local.A[0], local.A[1],
           local.A[2], local.A[3], local.A[4], local.W[i+3], K[i+3] );
        P( local.A[4], local.A[5], local.A[6], local.A[7], local.A[0],
           local.A[1], local.A[2], local.A[3], local.W[i+4], K[i+4] );
        P( local.A[3], local.A[4], local.A[5], local.A[6], local.A[7],
           local.A[0], local.A[1], local.A[2], local.W[i+5], K[i+5] );
        P( local.A[2], local.A[3], local.A[4], local.A[5], local.A[6],
           local.A[7], local.A[0], local.A[1], local.W[i+6], K[i+6] );
        P( local.A[1], local.A[2], local.A[3], local.A[4], local.A[5],
           local.A[6], local.A[7], local.A[0], local.W[i+7], K[i+7] );
    }

    for( i = 16; i < 64; i += 8 ) {
        P( local.A[0], local.A[1], local.A[2], local.A[3], local.A[4],
           local.A[5], local.A[6], local.A[7], R(i+0), K[i+0] );
        P( local.A[7], local.A[0], local.A[1], local.A[2], local.A[3],
           local.A[4], local.A[5], local.A[6], R(i+1), K[i+1] );
        P( local.A[6], local.A[7], local.A[0], local.A[1], local.A[2],
           local.A[3], local.A[4], local.A[5], R(i+2), K[i+2] );
        P( local.A[5], local.A[6], local.A[7], local.A[0], local.A[1],
           local.A[2], local.A[3], local.A[4], R(i+3), K[i+3] );
        P( local.A[4], local.A[5], local.A[6], local.A[7], local.A[0],
           local.A[1], local.A[2], local.A[3], R(i+4), K[i+4] );
        P( local.A[3], local.A[4], local.A[5], local.A[6], local.A[7],
           local.A[0], local.A[1], local.A[2], R(i+5), K[i+5] );
        P( local.A[2], local.A[3], local.A[4], local.A[5], local.A[6],
           local.A[7], local.A[0], local.A[1], R(i+6), K[i+6] );
        P( local.A[1], local.A[2], local.A[3], local.A[4], local.A[5],
           local.A[6], local.A[7], local.A[0], R(i+7), K[i+7] );
    }
#endif /* CONFIG_SHA256_SMALLER */

    for( i = 0; i < 8; i++ ) {
        ctx->state[i] += local.A[i];
    }

    return( 0 );
}

/*
 * SHA-224 process buffer
 */
void rb_crypto_sha224_update(rb_sha224_ctx_t *ctx,
                               const uint8_t *data,
                               uint32_t ilen )
{
    int32_t ret = -1;
    uint32_t fill;
    uint32_t left;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += (uint32_t) ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if( ctx->total[0] < (uint32_t) ilen ) {
        ctx->total[1]++;
    }

    if( left && ilen >= fill ) {
        memcpy( (void *) (ctx->buffer + left), data, fill );

        if( ( ret = local_sha224_process( ctx, ctx->buffer ) ) != 0 ) {
            /* error */
        	return;
        }

        data += fill;
        ilen  -= fill;
        left = 0;
    }

    while( ilen >= 64 ) {
        if( ( ret = local_sha224_process( ctx, data ) ) != 0 ) {
            /* error */
        	return;
        }

        data += 64;
        ilen  -= 64;
    }

    if( ilen > 0 ) {
        memcpy( (void *) (ctx->buffer + left), data, ilen );
    }
}

/*
 * SHA-224 final digest
 */
void rb_crypto_sha224_final(rb_sha224_ctx_t *ctx, uint8_t *digest )
{
    int32_t ret = -1;
    uint32_t used;
    uint32_t high, low;

    /*
     * Add padding: 0x80 then 0x00 until 8 bytes remain for the length
     */
    used = ctx->total[0] & 0x3F;

    ctx->buffer[used++] = 0x80;

    if( used <= 56 ) {
        /* Enough room for padding + length in current block */
        memset( ctx->buffer + used, 0, 56 - used );
    } else {
        /* We'll need an extra block */
        memset( ctx->buffer + used, 0, 64 - used );

        if( ( ret = local_sha224_process( ctx, ctx->buffer ) ) != 0 ) {
            /* error */
        	return;
        }

        memset( ctx->buffer, 0, 56 );
    }

    /*
     * Add message length
     */
    high = ( ctx->total[0] >> 29 )
         | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_UINT32_BE( high, ctx->buffer, 56 );
    PUT_UINT32_BE( low,  ctx->buffer, 60 );

    if( ( ret = local_sha224_process( ctx, ctx->buffer ) ) != 0 ) {
    	/* error */
        return;
    }

    /*
     * Output final state
     */
    PUT_UINT32_BE( ctx->state[0], digest,  0 );
    PUT_UINT32_BE( ctx->state[1], digest,  4 );
    PUT_UINT32_BE( ctx->state[2], digest,  8 );
    PUT_UINT32_BE( ctx->state[3], digest, 12 );
    PUT_UINT32_BE( ctx->state[4], digest, 16 );
    PUT_UINT32_BE( ctx->state[5], digest, 20 );
    PUT_UINT32_BE( ctx->state[6], digest, 24 );

    if( ctx->is_224 == 0 ) {
        PUT_UINT32_BE( ctx->state[7], digest, 28 );
    }
}