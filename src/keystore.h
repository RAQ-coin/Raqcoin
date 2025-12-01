// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers

#ifndef RAQCOIN_KEYSTORE_H
#define RAQCOIN_KEYSTORE_H

#include "crypter.h"
#include "sync.h"
#include <boost/signals2/signal.hpp>
#include "diskpubkeypos.h"

class CScript;

/** A virtual base class for key stores */
class CKeyStore
{
protected:
    mutable CCriticalSection cs_KeyStore;

public:
    virtual ~CKeyStore() {}

    // Add a key to the store.
    virtual bool AddKey(const CKey& key) =0;

    // Check whether a key corresponding to a given address is present in the store.
    virtual bool HaveKey(const CKeyID &address) const =0;
    virtual bool GetKey(const CKeyID &address, CKey& keyOut) const =0;
    virtual void GetKeys(std::set<CKeyID> &setAddress) const =0;
    virtual bool GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const;

    // Support for BIP 0013 : see https://en.raqcoin.it/wiki/BIP_0013
    virtual bool AddCScript(const CScript& redeemScript) =0;
    virtual bool HaveCScript(const CScriptID &hash) const =0;
    virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const =0;
    virtual bool GetSecret(const CKeyID &address, CSecret& vchSecret) const
    {
        CKey key;
        if (!GetKey(address, key))
            return false;
        vchSecret = key.GetPrivKey();
        return true;
    }

};

typedef std::map<CKeyID, CKey> KeyMap;
typedef std::map<CScriptID, CScript > ScriptMap;
typedef std::map<std::string, CDiskPubKeyPos> PubKeyPosMap;

/** Basic key store, that keeps keys in an address->secret map */
class CBasicKeyStore : public CKeyStore
{
protected:
    KeyMap mapKeys;
    ScriptMap mapScripts;
    PubKeyPosMap mapPubKeyPos;

public:
    bool AddKey(const CKey& key);
    bool AddKeyPubKey(const CKey& key, const CPubKey &pubkey);

    bool HaveKey(const CKeyID &address) const {
        bool result;
        {
            LOCK(cs_KeyStore);
            result = (mapKeys.count(address) > 0);
        }
        return result;
    }   

    void GetKeys(std::set<CKeyID> &setAddress) const
    {
        setAddress.clear();
        {
            LOCK(cs_KeyStore);
            KeyMap::const_iterator mi = mapKeys.begin();
            while (mi != mapKeys.end())
            {
                setAddress.insert((*mi).first);
                mi++;
            }
        }
    }

    bool GetKey(const CKeyID &address, CKey &keyOut) const {
        {
            LOCK(cs_KeyStore);
            KeyMap::const_iterator mi = mapKeys.find(address);
            if (mi != mapKeys.end())
            {
                const CKey* pKey = &(mi->second);
                keyOut.SetPubKey(pKey->GetPubKey());
                keyOut.SetPrivKey(pKey->GetPrivKey());
                return true;
            }
        }
        return false;
    }
    
    bool AddPubKeyPos2Map(const std::string& address, const CDiskPubKeyPos& pos)
    {
    	{
            LOCK(cs_KeyStore);
            mapPubKeyPos[address] = pos;
    	}
        return true;
    }
    
    bool GetPubKeyPos(const std::string& address, CDiskPubKeyPos& posOut)
    {
      {
        LOCK(cs_KeyStore);
        PubKeyPosMap::const_iterator mi = mapPubKeyPos.find(address);
        if (mi != mapPubKeyPos.end())
        {
            const CDiskPubKeyPos* pos = &(mi->second);
            posOut.nHeight = pos->nHeight;
            posOut.nPubKeyOffset = pos->nPubKeyOffset;
            return true;
        }
      }
        return false;
    }

    virtual bool AddCScript(const CScript& redeemScript);
    virtual bool HaveCScript(const CScriptID &hash) const;
    virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const;
};

typedef std::map<CKeyID, std::pair<CPubKey, std::vector<unsigned char> > > CryptedKeyMap;

/** Keystore which keeps the private keys encrypted.
 * It derives from the basic key store, which is used if no encryption is active.
 */
class CCryptoKeyStore : public CBasicKeyStore
{
private:
    CryptedKeyMap mapCryptedKeys;

    CKeyingMaterial vMasterKey;

    // if fUseCrypto is true, mapKeys must be empty
    // if fUseCrypto is false, vMasterKey must be empty
    bool fUseCrypto;

protected:
    bool SetCrypted();

    // will encrypt previously unencrypted keys
    bool EncryptKeys(CKeyingMaterial& vMasterKeyIn);

    bool Unlock(const CKeyingMaterial& vMasterKeyIn);

public:
    CCryptoKeyStore() : fUseCrypto(false)
    {
    }

    bool IsCrypted() const
    {
        return fUseCrypto;
    }

    bool IsLocked() const
    {
        if (!IsCrypted())
            return false;
        bool result;
        {
            LOCK(cs_KeyStore);
            result = vMasterKey.empty();
        }
        return result;
    }

    bool Lock();

    virtual bool AddCryptedKey(const CPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret);
    bool AddKey(const CKey& key);
    bool HaveKey(const CKeyID &address) const
    {
        {
            LOCK(cs_KeyStore);
            if (!IsCrypted())
                return CBasicKeyStore::HaveKey(address);
            return mapCryptedKeys.count(address) > 0;
        }
        return false;
    }
    bool GetKey(const CKeyID &address, CKey& keyOut) const;
    bool GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const;
    void GetKeys(std::set<CKeyID> &setAddress) const;

    /* Wallet status (encrypted, locked) changed.
     * Note: Called without locks held.
     */
    boost::signals2::signal<void (CCryptoKeyStore* wallet)> NotifyStatusChanged;
};

#endif
