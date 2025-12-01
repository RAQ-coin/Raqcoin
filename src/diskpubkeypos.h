// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers

#ifndef RAQCOIN_DISKPUBKEYPOS_H
#define RAQCOIN_DISKPUBKEYPOS_H

#include <vector>
#include <boost/thread.hpp>
#include "key.h"
#include "serialize.h"

class CBlockIndex;

static const unsigned int RAINBOW_PUBLIC_KEY_POS_SIZE         = 8;
static const unsigned int RAINBOW_PUBLIC_KEY_REUSED_SIZE      = 4;

//nHeight is different from CDiskBlockPos.nFile, because it need to cast to unsigned char in CScript
class CDiskPubKeyPos
{
public:
    unsigned int nHeight;
    unsigned int nPubKeyOffset;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(VARINT(nHeight));
        READWRITE(VARINT(nPubKeyOffset));
    )

    CDiskPubKeyPos(unsigned int nHeightIn,         unsigned int nPubKeyOffsetIn)
    :nHeight(nHeightIn), nPubKeyOffset(nPubKeyOffsetIn)
        {}

    CDiskPubKeyPos() {
        SetNull();
    }

    void SetNull() {
        nHeight = 0xFFFFFFFF; nPubKeyOffset = 0;
    }

    bool IsNull() const { return (nHeight == 0xFFFFFFFF); }

    friend bool operator==(const CDiskPubKeyPos &a, const CDiskPubKeyPos &b) {
        return (a.nHeight == b.nHeight && a.nPubKeyOffset == b.nPubKeyOffset);
    }

    friend bool operator!=(const CDiskPubKeyPos &a, const CDiskPubKeyPos &b) {
        return !(a == b);
    }

    CDiskPubKeyPos& operator<<(const std::vector<unsigned char>& v)
    {
        if (v.size() < RAINBOW_PUBLIC_KEY_POS_SIZE) {
            nHeight = 0xFFFFFFFF;
            return *this;
        }

        unsigned int cursor0 = ((unsigned char)v[0]) & 0xff;
        unsigned int cursor1 = ((unsigned char)v[1]) & 0xff;
        unsigned int cursor2 = ((unsigned char)v[2]) & 0xff;
        unsigned int cursor3 = ((unsigned char)v[3]) & 0xff;
        nHeight = cursor0 + (cursor1<<8) + (cursor2<<16) + (cursor3<<24);

        cursor0 = ((unsigned char)v[4]) & 0xff;
        cursor1 = ((unsigned char)v[5]) & 0xff;
        cursor2 = ((unsigned char)v[6]) & 0xff;
        cursor3 = ((unsigned char)v[7]) & 0xff;
        nPubKeyOffset  = cursor0 + (cursor1<<8) + (cursor2<<16) + (cursor3<<24);

        return *this;
    }

    std::vector<unsigned char> ToVector() const
    {
        std::vector<unsigned char> v;
        v.clear();

        unsigned char* ch = (unsigned char*)&nHeight;
        unsigned int i =0;
        while(i < sizeof(nHeight)) {
            v.push_back(*ch);
            i++;
            ch++;
        }

        i =0;
        ch = (unsigned char*)&nPubKeyOffset;
        while(i < sizeof(nPubKeyOffset)) {
            v.push_back(*ch);
            i++;
            ch++;
        }

        return v;
    }
};

bool FindPubKeyPos(std::string& pubKeyIn, CDiskPubKeyPos& pubKeyPos, bool ignoreMaturity, CBlockIndex* pIndexStart);

bool GetPubKeyByPos(CDiskPubKeyPos pos, CPubKey& pubKey);

bool UpdatePubKeyPos(CPubKey& pubKey, const std::string& address);

void SearchPubKeyPos(boost::thread_group& threadGroup);

#endif

