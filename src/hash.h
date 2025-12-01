// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers

#ifndef RAQCOIN_HASH_H
#define RAQCOIN_HASH_H

#include "uint256.h"
#include "serialize.h"
#include "pqcrypto/pqcrypto.h"

#include <vector>

template<typename T1>
inline uint256 Hash(const T1 pbegin, const T1 pend) {
    static unsigned char pblank[1];
    uint256 hash1;
	pqcSha256(pbegin == pend ? pblank : (unsigned char*)&pbegin[0], (pend - pbegin) * sizeof(pbegin[0]),(unsigned char*)&hash1);
    uint256 hash2;
	pqcSha256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}
uint256 Hash(const std::vector<unsigned char>& vch);

template<typename T1>
inline uint384 Hash384(const T1 pbegin, const T1 pend) {
    static unsigned char pblank[1];
    uint384 hash1;
	pqcSha384(pbegin == pend ? pblank : (unsigned char*)&pbegin[0], (pend - pbegin) * sizeof(pbegin[0]),(unsigned char*)&hash1);
    uint384 hash2;
	pqcSha384((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}
uint384 Hash384(const std::vector<unsigned char>& vch);

template<typename T1>
inline uint512 Hash512(const T1 pbegin, const T1 pend) {
    static unsigned char pblank[1];
    uint512 hash1;
	pqcSha512(pbegin == pend ? pblank : (unsigned char*)&pbegin[0], (pend - pbegin) * sizeof(pbegin[0]),(unsigned char*)&hash1);
    uint512 hash2;
	pqcSha512((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}
uint512 Hash512(const std::vector<unsigned char>& vch);

unsigned int get_hashsize(unsigned int config_value);

template<typename T1>
inline std::vector<unsigned char> HashPro(unsigned int config_value, const T1 pbegin, const T1 pend) {
    std::vector<unsigned char> out;
    unsigned int hashSize = get_hashsize(config_value);

    if (hashSize < 48) {
        uint256 hash256 = Hash(pbegin, pend);
        out.assign(hash256.begin(), hash256.end());
    } else if (hashSize < 64) {
        uint384 hash384 = Hash384(pbegin, pend);
        out.assign(hash384.begin(), hash384.end());
    } else {
        uint512 hash512 = Hash512(pbegin, pend);
        out.assign(hash512.begin(), hash512.end());
    }

    return out;
}
std::vector<unsigned char> HashPro(unsigned int config_value, const std::vector<unsigned char>& in);

class CHashWriter
{
private:
    Sha256 ctx;
    int nType;
    int nVersion;

public:
    void Init();
    CHashWriter(int nTypeIn, int nVersionIn);
    CHashWriter& write(const char *pch, size_t size);
    uint256 GetHash();

    template<typename T>
    CHashWriter& operator<<(const T& obj) {
        ::Serialize(*this, obj, nType, nVersion);
        return (*this);
    }
};

class CHashWriter384
{
private:
    Sha384 ctx;
    int nType;
    int nVersion;

public:
    void Init();
    CHashWriter384(int nTypeIn, int nVersionIn);
    CHashWriter384& write(const char *pch, size_t size);
    uint384 GetHash();

    template<typename T>
    CHashWriter384& operator<<(const T& obj) {
        ::Serialize(*this, obj, nType, nVersion);
        return (*this);
    }
};

class CHashWriter512
{
private:
    Sha512 ctx;
    int nType;
    int nVersion;

public:
    void Init();
    CHashWriter512(int nTypeIn, int nVersionIn);
    CHashWriter512& write(const char *pch, size_t size);
    uint512 GetHash();

    template<typename T>
    CHashWriter512& operator<<(const T& obj) {
        ::Serialize(*this, obj, nType, nVersion);
        return (*this);
    }
};

class CHashWriterPro
{
private:
    CHashWriter *m_pHashWrite256;
    CHashWriter384 *m_pHashWrite384;
    CHashWriter512 *m_pHashWrite512;

public:
    CHashWriterPro(unsigned int index, int nTypeIn, int nVersionIn);
    ~CHashWriterPro();
    std::vector<unsigned char> GetHash();

    template<typename T>
    CHashWriterPro& operator<<(const T& obj) {
        if (NULL != m_pHashWrite256) {
            *m_pHashWrite256 << obj;
        } else if (NULL != m_pHashWrite384) {
            *m_pHashWrite384 << obj;
        } else if (NULL != m_pHashWrite512) {
            *m_pHashWrite512 << obj;
        }  
        return (*this);
    }
};

template<typename T1, typename T2>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end)
{
    static unsigned char pblank[1];
    uint256 hash1;
    Sha256 ctx;
    sha256Init(&ctx);
    sha256Process(&ctx, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
    sha256Process(&ctx, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
    sha256Done(&ctx, (unsigned char*)&hash1);
    uint256 hash2;
    pqcSha256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

template<typename T1, typename T2, typename T3>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end)
{
    static unsigned char pblank[1];
    uint256 hash1;
    Sha256 ctx;
    sha256Init(&ctx);
    sha256Process(&ctx, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
    sha256Process(&ctx, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
    sha256Process(&ctx, (p3begin == p3end ? pblank : (unsigned char*)&p3begin[0]), (p3end - p3begin) * sizeof(p3begin[0]));
    sha256Done(&ctx, (unsigned char*)&hash1);
    uint256 hash2;
    pqcSha256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

template<typename T>
inline uint256 SerializeHash(const T& obj, int nType=SER_GETHASH, int nVersion=PROTOCOL_VERSION)
{
    CHashWriter ss(nType, nVersion);
    ss << obj;
    return ss.GetHash();
}

unsigned int MurmurHash3(unsigned int nHashSeed, const std::vector<unsigned char>& vDataToHash);

#endif
