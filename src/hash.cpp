#include "hash.h"
#include "rainbow18/RAINBOW_PRO_APIs.h"

inline uint32_t ROTL32 ( uint32_t x, int8_t r )
{
    return (x << r) | (x >> (32 - r));
}

unsigned int MurmurHash3(unsigned int nHashSeed, const std::vector<unsigned char>& vDataToHash)
{
    // The following is MurmurHash3 (x86_32), see http://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp
    uint32_t h1 = nHashSeed;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    const int nblocks = vDataToHash.size() / 4;

    //----------
    // body
    const uint32_t * blocks = (const uint32_t *)(&vDataToHash[0] + nblocks*4);

    for(int i = -nblocks; i; i++)
    {
        uint32_t k1 = blocks[i];

        k1 *= c1;
        k1 = ROTL32(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1 = ROTL32(h1,13); 
        h1 = h1*5+0xe6546b64;
    }

    //----------
    // tail
    const uint8_t * tail = (const uint8_t*)(&vDataToHash[0] + nblocks*4);

    uint32_t k1 = 0;

    switch(vDataToHash.size() & 3)
    {
    case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
            k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization
    h1 ^= vDataToHash.size();
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    return h1;
}

unsigned int get_hashsize(unsigned int config_value) {
    unsigned int waste_sk_size, waste_pk_size, waste_sign_size;
    unsigned int hashsize = 32;
    if (config_value > 0) {
        unsigned int status = rainbowplus_get_size(config_value, 0, &waste_sk_size, &waste_pk_size, &hashsize, &waste_sign_size);
        if (status != 1) {
            hashsize = 32;
        }
    }

    return hashsize;
}

uint256 Hash(const std::vector<unsigned char>& vch) {
    uint256 hash1;
    pqcSha256(&vch[0], vch.size(), (unsigned char*)&hash1);
    uint256 hash2;
    pqcSha256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

uint384 Hash384(const std::vector<unsigned char>& vch) {
    uint384 hash1;
    pqcSha384(vch.data(), vch.size(), (unsigned char*)&hash1);
    uint384 hash2;
    pqcSha384((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

uint512 Hash512(const std::vector<unsigned char>& vch) {
    uint512 hash1;
    pqcSha512(vch.data(), vch.size(), (unsigned char*)&hash1);
    uint512 hash2;
    pqcSha512((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

std::vector<unsigned char> HashPro(unsigned int config_value, const std::vector<unsigned char>& in) {
    return HashPro(config_value, in.begin(), in.end());
}

void CHashWriter::Init() {
   sha256Init(&ctx);
}

CHashWriter::CHashWriter(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn) {
    Init();
}

CHashWriter& CHashWriter::write(const char *pch, size_t size) {
    sha256Process(&ctx, (const unsigned char *)pch, size);
    return (*this);
}

uint256 CHashWriter::GetHash() {
    uint256 hash1;
    sha256Done(&ctx, (unsigned char*)&hash1);
    uint256 hash2;
    pqcSha256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

void CHashWriter384::Init() {
    sha384Init(&ctx);
}

CHashWriter384::CHashWriter384(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn) {
   Init();
}

CHashWriter384& CHashWriter384::write(const char *pch, size_t size) {
    sha384Process(&ctx, (const unsigned char*)pch, size);
    return (*this);
}

uint384 CHashWriter384::GetHash() {
    uint384 hash1;
    sha384Done(&ctx, (unsigned char*)&hash1);
    uint384 hash2;
    pqcSha384((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

void CHashWriter512::Init() {
    sha512Init(&ctx);
}

CHashWriter512::CHashWriter512(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn) {
    Init();
}

CHashWriter512& CHashWriter512::write(const char *pch, size_t size) {
    sha512Process(&ctx, (const unsigned char *)pch, size);
    return (*this);
}

uint512 CHashWriter512::GetHash() {
    uint512 hash1;
    sha512Done(&ctx, (unsigned char*)&hash1);
    uint512 hash2;
    pqcSha512((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

CHashWriterPro::CHashWriterPro(unsigned int config_value, int nTypeIn, int nVersionIn) {
    m_pHashWrite256 = NULL;
    m_pHashWrite384 = NULL;
    m_pHashWrite512 = NULL;

    unsigned int hashSize = get_hashsize(config_value);
    if (hashSize < 48) {
        m_pHashWrite256 = new CHashWriter(nTypeIn, nVersionIn);
    } else if (hashSize < 64) {
        m_pHashWrite384 = new CHashWriter384(nTypeIn, nVersionIn);
    } else {
        m_pHashWrite512 = new CHashWriter512(nTypeIn, nVersionIn);
    }
}

CHashWriterPro::~CHashWriterPro() {
    if (NULL != m_pHashWrite256) {
        delete m_pHashWrite256;
    } else if (NULL != m_pHashWrite384) {
        delete m_pHashWrite384;
    } else if (NULL != m_pHashWrite512) {
        delete m_pHashWrite512;
    }
}

std::vector<unsigned char> CHashWriterPro::GetHash() {
    std::vector<unsigned char> out;
    if (NULL != m_pHashWrite256) {
        uint256 hash = m_pHashWrite256->GetHash();
        out.assign(hash.begin(), hash.end());
    } else if (NULL != m_pHashWrite384) {
        uint384 hash = m_pHashWrite384->GetHash();
        out.assign(hash.begin(), hash.end());
    } else if (NULL != m_pHashWrite512) {
        uint512 hash = m_pHashWrite512->GetHash();
        out.assign(hash.begin(), hash.end());
    }
    return out;
}