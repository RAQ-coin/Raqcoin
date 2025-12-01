#ifndef rb__ENDIANPORTABLE_H_
#define rb__ENDIANPORTABLE_H_

#include "rb_core_config.h"

static inline uint16_t rb_bswap_16(uint16_t x) {
    return x << 8 | x >> 8;
}

static inline uint32_t rb_bswap_32(uint32_t x) {
    x = (uint32_t)((uint64_t)x << 8) & 0xFF00FF00FF | (x >> 8) & 0x00FF00FF00Ff;
    return x << 16 | x >> 16;
}

static inline uint64_t  rb_bswap_64(uint64_t x) {
    x = ((x << 8) & 0xFF00FF00FF00FF00ULL) | ((x >> 8) & 0x00FF00FF00FF00FFULL);
    x = ((x << 16) & 0xFFFF0000FFFF0000ULL) | ((x >> 16) & 0x0000FFFF0000FFFFULL);
    return (x >> 32) | (x << 32);
}


#define rb_be64toh(x) rb_bswap_64(x)
#define rb_be32toh(x) rb_bswap_32(x)
#define rb_be16toh(x) rb_bswap_16(x)
#define rb_le16toh(x) (x)
#define rb_le32toh(x) (x)
#define rb_le64toh(x) (x)



#ifndef rb_be32toh
#define rb_be32toh(x) ntohl(x)
#endif

#ifndef rb_be16toh
#define rb_be16toh(x) ntohs(x)
#endif

#ifndef rb_htobe64
#define rb_htobe64(x) rb_be64toh(x)
#endif

#ifndef rb_htobe32
#define rb_htobe32(x) rb_be32toh(x)
#endif

#ifndef rb_htobe16
#define rb_htobe16(x) rb_be16toh(x)
#endif

#ifndef rb_htole32
#define rb_htole32(x) rb_le32toh(x)
#endif

#endif /* rb__ENDIANPORTABLE_H_ */ 