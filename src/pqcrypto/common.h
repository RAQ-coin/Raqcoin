#ifndef PQCRYPTO_COMMON_H
#define PQCRYPTO_COMMON_H

#include <stdint.h>
#include <string.h>


inline uint32_t bswap_32(uint32_t x)
{
    return (((x & 0xff000000U) >> 24) | ((x & 0x00ff0000U) >>  8) |
            ((x & 0x0000ff00U) <<  8) | ((x & 0x000000ffU) << 24));
}

#if defined(WORDS_BIGENDIAN)
inline uint32_t override_htole32(uint32_t host_32bits)
{
    return bswap_32(host_32bits);
}
#else
inline uint32_t override_htole32(uint32_t host_32bits)
{
    return host_32bits;
}
#endif

void static inline WriteLE32(unsigned char* ptr, uint32_t x)
{
    uint32_t v = override_htole32(x);
    memcpy(ptr, (char*)&v, 4);
}

#endif
