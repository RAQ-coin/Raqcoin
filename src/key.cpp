// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers

#include "pqcrypto/api.h"
#include "key.h"
#include "rainbow18/RAINBOW_PRO_APIs.h"
#include <uuid/uuid.h>

extern int nBestHeight;

CKey::CKey(): pubKey() {
    fSet = false;
}

CKey::CKey(const CKey& b) {
    privKey = b.privKey;
    pubKey = b.pubKey;
    fSet = b.fSet;
}

CKey& CKey::operator=(const CKey& b) {
    privKey = b.privKey;
    pubKey = b.pubKey;
    fSet = b.fSet;
    return (*this);
}

void CKey::MakeNewKey(unsigned int config_value)
{
    int status;
    if (0 == config_value) {
        privKey.resize(RAINBOW_PRIVATE_KEY_SIZE);
        pubKey.vchPubKey.resize(RAINBOW_PUBLIC_KEY_SIZE);
        status = crypto_sign_keypair(pubKey.vchPubKey.data(), privKey.data());
        if (status != 0) {
            throw key_error("CKey::MakeNewKey, make key pair failure.");
        }
    } else {
        if (1 != rainbowplus_if_the_choised_configvalue(config_value)) {
            throw key_error("CKey::MakeNewKey, config_value not supported.");
        } else {
            unsigned int waste_hash_size, waste_sign_size;
            unsigned int sk_size, pk_size;
            status = rainbowplus_get_size(config_value, 1, &sk_size, &pk_size, &waste_hash_size, &waste_sign_size);
            if (status != 1) {
                throw key_error("CKey::MakeNewKey, rainbowplus_get_size failure.");
            }
            
            privKey.resize(sk_size);
            pubKey.vchPubKey.resize(pk_size);

            char uu_buf[32];
            uuid_t uu;
            uuid_generate(uu);
            uuid_unparse(uu, uu_buf);
            status = rainbowplus_get_keypair(config_value, 0, (unsigned char *)uu_buf, sizeof(uu_buf), privKey.data(), sk_size, pubKey.vchPubKey.data(), pk_size, NULL, NULL);
            if (status != 1) {
                throw key_error("CKey::MakeNewKey, rainbowplus_get_keypair failure.");
            }
        }
    }
    fSet = true;
}

bool CKey::SetPrivKey(const CPrivKey& vchPrivKey)
{
    privKey = vchPrivKey;
    return true;
}

int CPubKey::getPubKeyIndex() {
    if (vchPubKey.size() < 6) {
        return -1;
    }

    unsigned int config_value = 0;
    unsigned int waste_sk_size, waste_hash_size, waste_sign_size;
    unsigned int pubKeySize = RAINBOW_PUBLIC_KEY_SIZE;
    unsigned int status = rainbowplus_get_config(vchPubKey.data(), &config_value, NULL, NULL, NULL, NULL, NULL);
    if (1 == status) {
        status = rainbowplus_get_size(config_value, 0, &waste_sk_size, &pubKeySize, &waste_hash_size, &waste_sign_size);
        if (1 != status) {
            return -1;
        }
    } else {
        config_value = 0;
    }

    if (vchPubKey.size() != pubKeySize) {
        return -1;
    }

    return config_value;
}

bool CPubKey::Verify(unsigned int config_value, unsigned char* hash_buf, unsigned int hash_size, unsigned char* sign_buf, unsigned int sign_size, bool bMsg)
{
    if (0 == config_value) {
        int status = rainbow_verify(hash_buf, sign_buf, vchPubKey.data());
        if (0 == status) {
            return true;
        }
    } else if (bMsg || nBestHeight >= RAINBOWFORkHEIGHT) {
        unsigned int hashsize;
        unsigned int waste_sk_size, waste_pk_size, waste_sign_size;
        int status = rainbowplus_get_size(config_value, 0, &waste_sk_size, &waste_pk_size, &hashsize, &waste_sign_size);
        if (1 == status) {
            std::vector<unsigned char> hashbuf;
            hashbuf.resize(hashsize);
            rainbowplus_hash(config_value, 0, hash_buf, hash_size, hashbuf.data(), hashbuf.size());
            status = rainbowplus_verify(config_value, 0, hashbuf.data(), hashbuf.size(), sign_buf, sign_size, vchPubKey.data(), vchPubKey.size());
            if (1 == status) {
                return true;
            }
        }
    }

    return false;
}

bool CKey::SetPubKey(const CPubKey& vchPubKey)
{
    pubKey = vchPubKey;
    return true;
}

bool CKey::SetPubKey(std::vector<unsigned char> pk)
{
    pubKey.vchPubKey = pk;
    return true;
}

bool CKey::IsValid() {
    int config_value = pubKey.getPubKeyIndex();
    if (-1 == config_value) {
        return false;
    }

    if (0 == config_value) {
        if (privKey.size() == RAINBOW_PRIVATE_KEY_SIZE) {
            return true;
        }
    } else {
        uint32_t privKeySize;
        unsigned int waste_pk_size, waste_hash_size, waste_sign_size;
        int status = rainbowplus_get_size(config_value, 0, &privKeySize, &waste_pk_size, &waste_hash_size, &waste_sign_size);
        if (1 == status) {
            if (privKey.size() == privKeySize) {
                return true;
            }
        }
    }

    return false;
}

bool CKey::Sign(unsigned int config_value, unsigned char* m_hash, unsigned int mlen_hash, std::vector<unsigned char>& vchSig, bool bMsg)
{
    if (0 == config_value) {
        vchSig.resize(RAINBOW_SIGNATURE_SIZE);
        int status = rainbow_sign(vchSig.data(), privKey.data(), m_hash);
        if (0 == status) {
            return true;
        }
    } else if (bMsg || nBestHeight >= RAINBOWFORkHEIGHT) {
        unsigned int hashsize, signSize;
        unsigned int waste_sk_size, waste_pk_size;
        int status = rainbowplus_get_size(config_value, 0, &waste_sk_size, &waste_pk_size, &hashsize, &signSize);
        if (1 == status) {
            std::vector<unsigned char> hashbuf;
            hashbuf.resize(hashsize);
            rainbowplus_hash(config_value, 0, m_hash, mlen_hash, hashbuf.data(), hashbuf.size());
            vchSig.resize(signSize);
            status = rainbowplus_sign(config_value, 0, 0, vchSig.data(), vchSig.size(), hashbuf.data(), hashbuf.size(), privKey.data(), privKey.size(), NULL);
            if (1 == status) {
                return true;
            }
        }
    }

    vchSig.clear();

    return false;
}

uint32_t get_publicKey_size(unsigned char *pubkeyPrefix) {
    unsigned int pubKeySize = RAINBOW_PUBLIC_KEY_SIZE;
    unsigned int config_value = 0;
    unsigned int status = rainbowplus_get_config(pubkeyPrefix, &config_value, NULL, NULL, NULL, NULL, NULL);
    if (1 == status) {
        unsigned int waste_sk_size, waste_hash_size, waste_sign_size;
        status = rainbowplus_get_size(config_value, 0, &waste_sk_size, &pubKeySize, &waste_hash_size, &waste_sign_size);
        if (status != 1) {
            pubKeySize = RAINBOW_PUBLIC_KEY_SIZE;
        }
    }

    return pubKeySize;
}

void get_choised_info(unsigned int *pMin_byte_of_pk, unsigned int *pMin_byte_of_sign, unsigned int *pByte_of_sign, unsigned int *pConfig_value) {
    unsigned int max_byte_of_sk;
	unsigned int min_byte_of_sk;
	unsigned int max_byte_of_pk;
	unsigned int min_byte_of_pk;
	unsigned int max_byte_of_hash;
	unsigned int min_byte_of_hash;
	unsigned int max_byte_of_sign;
	unsigned int min_byte_of_sign;
	unsigned int default_choised_index;
	unsigned int default_config_value;
	unsigned int default_type;
	unsigned int default_subtype;
    unsigned int status = rainbowplus_get_choised_info(&max_byte_of_sk, &min_byte_of_sk, &max_byte_of_pk, &min_byte_of_pk, &max_byte_of_hash, &min_byte_of_hash, &max_byte_of_sign, &min_byte_of_sign, &default_choised_index, &default_config_value, &default_type, &default_subtype);
    if (status != 1) {
        max_byte_of_sign = RAINBOW_SIGNATURE_SIZE;
        default_config_value = 0;
    } else if (max_byte_of_sign < RAINBOW_SIGNATURE_SIZE) {
        max_byte_of_sign = RAINBOW_SIGNATURE_SIZE;
    }
    
    if (pMin_byte_of_pk != NULL) {
        *pMin_byte_of_pk = min_byte_of_pk;
    }

    if (pMin_byte_of_sign != NULL) {
        *pMin_byte_of_sign = min_byte_of_sign;
    }

    if (pByte_of_sign != NULL) {
        *pByte_of_sign = max_byte_of_sign;
    }
    if (pConfig_value != NULL) {
        *pConfig_value = default_config_value;
    }
}

bool publicKey_check_len(unsigned int len) {
    if (len == RAINBOW_PUBLIC_KEY_SIZE) {
        return true;
    }

    unsigned int status = rainbowplus_choised_check_len(2, len);
    if (2 == status) {
        return true;
    }

    return false;
}

std::string get_choised_config_values() {
    std::string strConfigValues;
    unsigned int choised_total = 0;
    rainbowplus_get_number_of_choised_total_config(&choised_total);
    for (int i = 1; i <= choised_total; i++) {
        unsigned int tmp_config_value = 0;
        unsigned int out_type, out_subtype;
        char config_name[64] = {0};
        char choised_sign_name[32] = {0};
        char choised_sign_discription[160] = {0};
        rainbowplus_get_choised_config(i, &tmp_config_value, &out_type, &out_subtype, config_name, choised_sign_name, choised_sign_discription);
        strConfigValues += "config_value:";
        strConfigValues += std::to_string(tmp_config_value);
        strConfigValues += ",  type:";
        strConfigValues += std::to_string(out_type);
        strConfigValues += ",  subType:";
        strConfigValues += std::to_string(out_subtype);
        strConfigValues += ",  config_name:";
        strConfigValues += config_name;
        strConfigValues += ",  choised_sign_name:";
        strConfigValues += choised_sign_name;
        strConfigValues += ",  choised_sign_discription:";
        strConfigValues += choised_sign_discription;
        strConfigValues += "\r\n\r\n";
    }

    return strConfigValues;
}

unsigned int get_choised_config_from_config_value(unsigned int config_value, char *choised_sign_name) {
    unsigned int out_type, out_subtype;
    return rainbowplus_get_choised_config_from_config_value(config_value, &out_type, &out_subtype, NULL, choised_sign_name, NULL);
}