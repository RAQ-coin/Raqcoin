// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers

#include "diskpubkeypos.h"
#include "util.h"
#include "init.h"
#include "wallet.h"



bool FindPubKeyPos(std::string& pubKeyIn, CDiskPubKeyPos& pubKeyPos, bool ignoreMaturity, CBlockIndex* pIndexStart)
{
    CBlockIndex* pBlockIndex = pIndexStart ? pIndexStart : pindexGenesisBlock;
    while (pBlockIndex && pindexBest) {
        if (!ignoreMaturity && (pindexBest->nHeight - pBlockIndex->nHeight + 1 < COINBASE_MATURITY+20)) {
            printf("%s: block not maturity, height:%u\n", __func__, pindexBest->nHeight - pBlockIndex->nHeight + 1);
            return false;
        }
        FILE* file = OpenBlockFile(pBlockIndex->GetBlockPos(), true);
        if (NULL == file)
            return error("%s() : open file blk%d.dat at %u error", __PRETTY_FUNCTION__, pBlockIndex->nFile, pBlockIndex->nDataPos);

        CAutoFile filein(file, SER_DISK, CLIENT_VERSION);

        // Read block
        CBlock block;
        try {
            filein >> block;
        }
        catch (const std::exception& e) {
            return error("%s: Deserialize or I/O error - %s, file blk%d.dat at %u", __func__, e.what(),
                         pBlockIndex->nFile, pBlockIndex->nDataPos);
        }

        unsigned int offset = sizeof(CBlockHeader); //block header
        unsigned int nTxCount = block.vtx.size();
        offset += GetSizeOfCompactSize(nTxCount);

        for (unsigned int i=0; i<nTxCount; i++)
        {
            const CTransaction &tx = block.vtx[i];
            unsigned int offsetVin = sizeof(tx.nVersion); // the vin offset

            unsigned int nVinSize = tx.vin.size();
            offsetVin += GetSizeOfCompactSize(nVinSize);

            for (unsigned int i = 0; i < nVinSize; i++) {
                CTxIn txIn = tx.vin[i];

                unsigned int nScriptSigSize = txIn.scriptSig.size();

                //compare the pubkey in the scripts with the input pubKeyIn
                std::string strScripts = HexStr(txIn.scriptSig);
                std::size_t found = strScripts.find(pubKeyIn);
                if (found!=std::string::npos) {
                    pubKeyPos.nHeight = pBlockIndex->nHeight;

                    offsetVin += sizeof(COutPoint); //transation hash length + prevout index length
                    offsetVin += GetSizeOfCompactSize(nScriptSigSize);
                    pubKeyPos.nPubKeyOffset = offset + offsetVin + found/2; // the offset in the current block, 2 hex char for one byte
                    unsigned int npubKeyLength = getInScriptCompactSize(pubKeyIn.length()/2);
                    printf("public key found: offset:%u, offsetVin:%u, found:%u, npubKeyLength:%u. \n",
                            offset, offsetVin, found, npubKeyLength);
                    pubKeyPos.nPubKeyOffset -= npubKeyLength;
                    return true;
                } else {
                    offsetVin += ::GetSerializeSize(txIn, SER_DISK, CLIENT_VERSION);
                }
            }

            offset += GetSerializeSize(tx, SER_DISK, CLIENT_VERSION); //CTransaction length
        }

        pBlockIndex = pBlockIndex->pnext;
    }

    //still no transation contain this pubkey in the block chain
    return false;
}

bool FindPubKeyPos(std::string& pubKeyIn, CDiskPubKeyPos& pubKeyPos)
{
    return FindPubKeyPos(pubKeyIn, pubKeyPos, false, NULL);
}

bool GetPubKeyByPos(CDiskPubKeyPos pos, CPubKey& pubKey)
{
    CBlockIndex* pblockindex = pindexGenesisBlock;
    bool bFindBlockByHeight = false;
    while (pblockindex) {
        if ((unsigned int)(pblockindex->nHeight) == pos.nHeight) {
            bFindBlockByHeight = true;
            break;
        } else {
            pblockindex = pblockindex->pnext;
        }
    }

    if (!bFindBlockByHeight)
        return error("%s() : can't find block at height: %u", __PRETTY_FUNCTION__, pos.nHeight);

    //in scripts, the public key is deserialize as
    //4e (21 52 02 00) (e2 b8 8a 76 1b 0d d7 8e b3...)--4e is the opcode, (21 52 02 00) is the length
    CDiskBlockPos blockPos(pblockindex->nFile , pblockindex->nDataPos + pos.nPubKeyOffset);
    FILE* pFile = OpenBlockFile(blockPos, true);
    if (NULL == pFile)
        return error("%s() : open file blk%d.dat error", __PRETTY_FUNCTION__, pblockindex->nFile);

    CAutoFile file(pFile, SER_DISK, CLIENT_VERSION);
    try {
        unsigned char opcode;
        READDATA(file, opcode);
        unsigned int nSize = 0;
        if (opcode < OP_PUSHDATA1)
        {
            nSize = opcode;
        }
        else if (opcode == OP_PUSHDATA1)
        {
            unsigned char chSize;
            READDATA(file, chSize);
            nSize = chSize;
        }
        else if (opcode == OP_PUSHDATA2)
        {
            unsigned short chSize;
            READDATA(file, chSize);
            nSize = chSize;
        }
        else if (opcode == OP_PUSHDATA4)
        {
            READDATA(file, nSize);
        } else
            return error("%s() : invalid opcode=%x or I/O error", __PRETTY_FUNCTION__, opcode);

        unsigned int i = 0;
        while (i < nSize)
        {
            unsigned int blk = std::min(nSize - i, (unsigned int)(1 + 4999999 / sizeof(unsigned char)));
            pubKey.vchPubKey.resize(i + blk);
            file.read((char*)&pubKey.vchPubKey[i], blk * sizeof(unsigned char));
            i += blk;
        }

        if (!pubKey.IsValid()) {
            return error("%s() : public key size %d invalid", __PRETTY_FUNCTION__, nSize);
        }

    } catch (std::exception &e) {
        return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
    }

    return true;
}

bool UpdatePubKeyPos(CPubKey& pubKey, const std::string& address)
{
    CDiskPubKeyPos pos;
    bool found = false;
    std::string strPubKey = HexStr(pubKey.vchPubKey);

    if (!pwalletMain->GetPubKeyPos(address, pos)) {
        if (!FindPubKeyPos(strPubKey, pos)) {
            printf("public key %s not found in block chain! \n", address.c_str());
            found = false;
        } else {
            printf("public key %s found at height=%d, offset=%u. \n", address.c_str(), pos.nHeight, pos.nPubKeyOffset);
            pwalletMain->AddPubKeyPos(address, pos);
            found = true;
        }
    } else {
        //read the public key by position and compare, if not match, find it again
        CPubKey diskPubKey;
        printf("public key pos is not null, height=%u, offset=%u. checking! \n", pos.nHeight, pos.nPubKeyOffset);
        if (!GetPubKeyByPos(pos, diskPubKey)) {
            printf("can't get public key %s at height=%u, offset=%u. will find again! \n",
                address.c_str(), pos.nHeight, pos.nPubKeyOffset);
            pos.SetNull();
        } else if (diskPubKey != pubKey) {
            printf("public key foud but not equal, will find again! \n");
            pos.SetNull();
        }

        if (pos.IsNull()) {
            //find again
            if (!FindPubKeyPos(strPubKey, pos)) {
                printf("find again, public key %s not found in block chain! \n", address.c_str());
                //got a wrong position, update it to null
                pwalletMain->AddPubKeyPos(address, pos);
                found = false;
            } else {
                printf("find again, public key %s found at height=%d, offset=%u. \n",
                    address.c_str(), pos.nHeight, pos.nPubKeyOffset);
                pwalletMain->AddPubKeyPos(address, pos);
                found = true;
            }
        } else {
            printf("public key %s found and match at height=%d, offset=%u. \n",
                address.c_str(), pos.nHeight, pos.nPubKeyOffset);
            found = true;
        }
    }

    return found;
}

void PubKeyScanner(CWallet* pwalletMain)
{
    printf("PubKeyScanner started\n");
    RenameThread("PubKeyScanner");
    bool allPosFound = true;
    SetThreadPriority(THREAD_PRIORITY_NORMAL);

    try {
        while(true) {
            std::set<CKeyID> setAddress;
            pwalletMain->GetKeys(setAddress);
            for (std::set<CKeyID>::iterator it = setAddress.begin(); it != setAddress.end(); ++it) {
                std::string address = CRaqcoinAddress(*it).ToString();

                CPubKey pubKey;
                if (!pwalletMain->GetPubKey(*it, pubKey)) {
                    printf("address %s not found in wallet.\n", address.c_str());
                    continue;
                }

                if(!UpdatePubKeyPos(pubKey, address)) {
                  //  printf("address %s update public key position failed\n", address.c_str());
                    allPosFound = false;
                    continue;
                } else {
                    std::map<CTxDestination, std::string>::iterator mi = pwalletMain->mapAddressBook.find(*it);
                    if (mi != pwalletMain->mapAddressBook.end())
                        pwalletMain->NotifyAddressBookChanged(pwalletMain, *it, mi->second, true, CT_UPDATED);
                }
            }

            if (allPosFound) {
                SetThreadPriority(THREAD_PRIORITY_LOWEST);
            } else
                SetThreadPriority(THREAD_PRIORITY_NORMAL);

            MilliSleep(allPosFound ? 120*60*1000 : 10*60*1000);
        }
    }
    catch (boost::thread_interrupted)
    {
        printf("PubKeyScanner terminated\n");
        throw;
    }
}

//more logic extention (offer rpc command to user to control this thread)
void SearchPubKeyPos(boost::thread_group& threadGroup)
{
    threadGroup.create_thread(boost::bind(&PubKeyScanner, pwalletMain));
}