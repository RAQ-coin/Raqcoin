#ifndef rb___ROCKY_SHA512__H
#define rb___ROCKY_SHA512__H

#include "rb_core_config.h"

#define ERR_OK           0
#define ERR_ERR         -1  
#define ERR_INV_PARAM   -2  
#define ERR_TOO_LONG    -3  
#define ERR_STATE_ERR   -4  


typedef struct {
    uint64_t high;
    uint64_t low; 
} rb_uint128_t;

typedef struct sha512_context {
    /* message total length in bytes */
    rb_uint128_t total;

    /* intermedia hash value for each block */
    struct {
        uint64_t a;
        uint64_t b;
        uint64_t c;
        uint64_t d;
        uint64_t e;
        uint64_t f;
        uint64_t g;
        uint64_t h;
    }hash;

    /* last block */
    struct {
        uint32_t used;      /* used bytes */
        uint8_t  buf[128];  /* block data buffer */
    }last;

    uint32_t ext;           /* t value of SHA512/t */
}rb_SHA512_CTX;

/* https://www.openssl.org/docs/man1.1.1/man3/SHA256_Final.html */

int rb_SHA384_Init(rb_SHA512_CTX* c);
int rb_SHA384_Update(rb_SHA512_CTX* c, const void* data, size_t len);
int rb_SHA384_Final(rb_SHA512_CTX* c, unsigned char* md);
unsigned char* rb_SHA384(const unsigned char* d, size_t n, unsigned char* md);

int rb_SHA512_Init(rb_SHA512_CTX* c);
int rb_SHA512_Update(rb_SHA512_CTX* c, const void* data, size_t len);
int rb_SHA512_Final(rb_SHA512_CTX* c,unsigned char* md);
unsigned char* rb_SHA512(const unsigned char* d, size_t n, unsigned char* md);

/* SHA512/224 */
int rb_SHA512_224_Init(rb_SHA512_CTX* c);
int rb_SHA512_224_Update(rb_SHA512_CTX* c, const void* data, size_t len);
int rb_SHA512_224_Final(rb_SHA512_CTX* c,unsigned char* md);
unsigned char* rb_SHA512_224(const unsigned char* d, size_t n, unsigned char* md);

/* SHA512/256 */
int rb_SHA512_256_Init(rb_SHA512_CTX* c);
int rb_SHA512_256_Update(rb_SHA512_CTX* c, const void* data, size_t len);
int rb_SHA512_256_Final( rb_SHA512_CTX* c, unsigned char* md);
unsigned char* rb_SHA512_256(const unsigned char* d, size_t n, unsigned char* md);

int rb_SHA512t_Init(rb_SHA512_CTX* c, unsigned int t);
int rb_SHA512t_Update(rb_SHA512_CTX* c, const void* data, size_t len);
int rb_SHA512t_Final(unsigned char* md, rb_SHA512_CTX* c);
unsigned char* rb_SHA512t(const unsigned char* d, size_t n, unsigned char* md, unsigned int t);

#endif
