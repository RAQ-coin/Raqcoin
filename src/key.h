// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers

#ifndef RAQCOIN_KEY_H
#define RAQCOIN_KEY_H

#include <stdexcept>
#include <vector>

#include "allocators.h"
#include "serialize.h"
#include "uint256.h"
#include "hash.h"

#include <boost/variant.hpp>
/**
 * RAINBOW signature:
 */
static const unsigned int RAINBOW_PUBLIC_KEY_SIZE           = 152097;
static const unsigned int RAINBOW_SIGNATURE_SIZE            = 64;
static const unsigned int RAINBOW_PRIVATE_KEY_SIZE          = 100209;
static const unsigned int HASH_LEN_BYTES                    = 32;

static const unsigned int RAINBOWFORkHEIGHT                 = 267120;       //nNewInterval(504) * n


class key_error : public std::runtime_error
{
public:
    explicit key_error(const std::string& str) : std::runtime_error(str) {}
};

/** A reference to a CKey: the Hash of its serialized public key */
class CKeyID : public uint256
{
public:
    CKeyID() : uint256(0) { }
    CKeyID(const uint256 &in) : uint256(in) { }
	CKeyID(const std::string& in) : uint256(in) { }
};

/** A reference to a CScript: the Hash of its serialization (see script.h) */
class CScriptID : public uint256
{
public:
    CScriptID() : uint256(0) { }
    CScriptID(const uint256 &in) : uint256(in) { }
};

class CNoDestination {
public:
    friend bool operator==(const CNoDestination &a, const CNoDestination &b) { return true; }
    friend bool operator<(const CNoDestination &a, const CNoDestination &b) { return true; }
};

/** A txout script template with a specific destination. It is either:
 *  * CNoDestination: no destination set
 *  * CKeyID: TX_PUBKEYHASH destination
 *  * CScriptID: TX_SCRIPTHASH destination
 *  A CTxDestination is the internal data type encoded in a CRaqcoinAddress
 */
typedef boost::variant<CNoDestination, CKeyID, CScriptID> CTxDestination;

/** An encapsulated public key. */
class CPubKey {
public:
    std::vector<unsigned char> vchPubKey;
    
public:
    CPubKey() {}

    CPubKey(const CPubKey &pk) {
        vchPubKey = pk.vchPubKey;
    }

    explicit CPubKey(const std::vector<unsigned char> &vchPubKeyIn) { 
        vchPubKey = vchPubKeyIn;
    }
    
    friend bool operator==(const CPubKey &a, const CPubKey &b) { return a.vchPubKey == b.vchPubKey; }
    friend bool operator!=(const CPubKey &a, const CPubKey &b) { return a.vchPubKey != b.vchPubKey; }

    CPubKey &operator = (const CPubKey &rhs) 
    {
        if (this == &rhs) {
            return *this;
        }
        
        vchPubKey = rhs.vchPubKey;
        return *this;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(vchPubKey);
    )

    CKeyID GetID() const {
        return CKeyID(Hash(vchPubKey));
    }

    uint256 GetHash() const {
        return Hash(vchPubKey.begin(), vchPubKey.end());
    }
    
    int getPubKeyIndex();
    
    bool IsValid() {
        int index = getPubKeyIndex();
        if (-1 == index) {
            return false;
        }
        
        return true;
    }
    
    bool Verify(unsigned int config_value, unsigned char* hash_buf, unsigned int hash_size, unsigned char* sign_buf, unsigned int sign_size, bool bMsg);
};

// secure_allocator is defined in allocators.h
// CPrivKey is a serialized private key, with all parameters included
typedef std::vector<unsigned char > CPrivKey;
// CSecret is a serialization of just the secret parameter 
typedef std::vector<unsigned char > CSecret;

class CKey
{
protected:
    bool fSet;
    std::vector<unsigned char > privKey;
    CPubKey pubKey;

public:
    CKey();
    CKey(const CKey& b);
    ~CKey(){}
    CKey& operator=(const CKey& b);
    bool IsNull() const{return !fSet;}
    void MakeNewKey(unsigned int config_value);
    bool SetPrivKey(const CPrivKey& vchPrivKey);
    CPrivKey GetPrivKey() const  {  return privKey;}
    bool SetPubKey(const CPubKey& vchPubKey);
    bool SetPubKey(std::vector<unsigned char> pk);
    CPubKey GetPubKey() const {      return (pubKey);}
    bool IsValid();
    bool Sign(unsigned int config_value, unsigned char* m_hash, unsigned int mlen_hash, std::vector<unsigned char>& vchSig, bool bMsg);
};

uint32_t get_publicKey_size(unsigned char *pubkeyPrefix);
void get_choised_info(unsigned int *pMin_byte_of_pk, unsigned int *pMin_byte_of_sign, unsigned int *max_byte_of_sign, unsigned int *default_config_value);
bool publicKey_check_len(unsigned int len);
std::string get_choised_config_values();
unsigned int get_choised_config_from_config_value(unsigned int config_value, char *choised_sign_name);

#endif
