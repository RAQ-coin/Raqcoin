// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers

#ifndef RAQCOIN_TXDB_LEVELDB_H
#define RAQCOIN_TXDB_LEVELDB_H

#include "main.h"
#include "leveldb.h"

/** CCoinsView backed by the LevelDB coin database (chainstate/) */
class CCoinsViewDB : public CCoinsView
{
protected:
    CLevelDB db;
public:
    CCoinsViewDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);

    bool GetCoins(const uint256 &txid, CCoins &coins);
    bool SetCoins(const uint256 &txid, const CCoins &coins);
    bool HaveCoins(const uint256 &txid);
    CBlockIndex *GetBestBlock();
    bool SetBestBlock(CBlockIndex *pindex);
    bool BatchWrite(const std::map<uint256, CCoins> &mapCoins, CBlockIndex *pindex);
    bool GetStats(CCoinsStats &stats);
};

/** Access to the block database (blocks/index/) */
class CBlockTreeDB : public CLevelDB
{
public:
    CBlockTreeDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);
private:
    CBlockTreeDB(const CBlockTreeDB&);
    void operator=(const CBlockTreeDB&);
public:
    bool WriteBlockIndex(const CDiskBlockIndex& blockindex);
    bool ReadBestInvalidWork(uint64& bnBestInvalidWork);
    bool WriteBestInvalidWork(const uint64& bnBestInvalidWork);
    bool ReadBlockFileInfo(int nFile, CBlockFileInfo &fileinfo);
    bool WriteBlockFileInfo(int nFile, const CBlockFileInfo &fileinfo);
    bool ReadLastBlockFile(int &nFile);
    bool WriteLastBlockFile(int nFile);
    bool WriteReindexing(bool fReindex);
    bool ReadReindexing(bool &fReindex);
    bool ReadTxIndex(const uint256 &txid, CDiskTxPos &pos);
    bool WriteTxIndex(const std::vector<std::pair<uint256, CDiskTxPos> > &list);
    bool WriteFlag(const std::string &name, bool fValue);
    bool ReadFlag(const std::string &name, bool &fValue);
    bool LoadBlockIndexGuts();
};

class CPublicKeyPosDB : public CLevelDB
{
public:
    CPublicKeyPosDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);

    bool WritePublicKeyPos(const std::string& address, const CDiskPubKeyPos& entity);
    bool ReadPublicKeyPos(const std::string& address, CDiskPubKeyPos& entity);
    bool DeletePublicKeyPos(const std::string& address);
};

#endif // RAQCOIN_TXDB_LEVELDB_H
