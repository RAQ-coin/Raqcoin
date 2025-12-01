
#ifndef rb__SHA256_H
#define rb__SHA256_H

#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif

#include "rb_core_config.h"

#define rb_SHA256_HASH_SIZE 32

/* Hash size in 32-bit words */
#define rb_SHA256_HASH_WORDS 8

typedef struct rb_sha256_context
{
    uint64_t totalLength;
    uint32_t hash[rb_SHA256_HASH_WORDS];
    uint32_t bufferLength;
    union {
        uint32_t words[16];
        uint8_t bytes[64];
    } buffer;
#ifdef RUNTIME_ENDIAN
    int littleEndian;
#endif /* RUNTIME_ENDIAN */
}rb_SHA256_CTX;



#ifdef __cplusplus
extern "C" {
#endif

    void rb_SHA256Init(rb_SHA256_CTX* sc);
    void rb_SHA256Update(rb_SHA256_CTX* sc, const void* data, uint32_t len);
    void rb_SHA256Final(rb_SHA256_CTX* sc, uint8_t hash[rb_SHA256_HASH_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* !rb__SHA256_H */
