// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers


#include <vector>
#include <string>
#ifdef WIN32
#include <windows.h>
#endif

#include "crypter.h"
#include "pqcrypto/pqcrypto.h"

int CCrypter::BytesToKeySHA512AES(const std::vector<unsigned char>& chSalt, 
                                              const SecureString& strKeyData, 
                                              int count, 
                                              unsigned char *key,
                                              unsigned char *iv) const
{
    // Because sha512's output size (64b) is
    // greater than the aes256 block size (16b) + aes256 key size (32b),
    // there's no need to process more than once (D_0).

    if(!count || !key || !iv)
        return 0;

    unsigned char buf[64];
	Sha512 sha512;
	sha512Init(&sha512);
	sha512Process(&sha512, (const unsigned char*)strKeyData.c_str(), strKeyData.size());
    if(chSalt.size()) {
        sha512Process(&sha512, &chSalt[0], chSalt.size());
    }
	sha512Done(&sha512, buf);

    for(int i = 0; i != count - 1; i++) {
		sha512Init(&sha512);
        sha512Process(&sha512, buf, sizeof(buf));
		sha512Done(&sha512, buf);
	}

    memcpy(key, buf, WALLET_CRYPTO_KEY_SIZE);
    memcpy(iv, buf + WALLET_CRYPTO_KEY_SIZE, WALLET_CRYPTO_IV_SIZE);
    zeromem(buf, sizeof(buf));
    return WALLET_CRYPTO_KEY_SIZE;
}



bool CCrypter::SetKeyFromPassphrase(const SecureString& strKeyData, 
	                                            const std::vector<unsigned char>& chSalt, 
	                                            const unsigned int nRounds, 
	                                            const unsigned int nDerivationMethod)
{
    if (nRounds < 1 || chSalt.size() != WALLET_CRYPTO_SALT_SIZE)
        return false;

    int i = 0;
    if (nDerivationMethod == 0)
        i = BytesToKeySHA512AES(chSalt, strKeyData, nRounds, chKey, chIV);


    if (i != (int)WALLET_CRYPTO_KEY_SIZE)
    {
        zeromem(chKey, sizeof(chKey));
        zeromem(chIV, sizeof(chIV));
        return false;
    }

    fKeySet = true;
    return true;
}

bool CCrypter::SetKey(const CKeyingMaterial& chNewKey, const std::vector<unsigned char>& chNewIV)
{
    if (chNewKey.size() != WALLET_CRYPTO_KEY_SIZE || chNewIV.size() != WALLET_CRYPTO_KEY_SIZE)
        return false;

    memcpy(&chKey[0], &chNewKey[0], sizeof chKey);
    memcpy(&chIV[0], &chNewIV[0], sizeof chIV);

    fKeySet = true;
    return true;
}

bool CCrypter::Encrypt(const CKeyingMaterial& vchPlaintext, std::vector<unsigned char> &vchCiphertext)
{
    if (!fKeySet)
        return false;

    // max ciphertext len for a n bytes of plaintext is
    // n + AES_BLOCKSIZE bytes
    vchCiphertext.resize(vchPlaintext.size() + AES_BLOCKSIZE);

    AES256CBCEncrypt enc(chKey, chIV, true);
    size_t nLen = enc.Encrypt(&vchPlaintext[0], vchPlaintext.size(), &vchCiphertext[0]);
    if(nLen < vchPlaintext.size())
        return false;
    vchCiphertext.resize(nLen);

    return true;
}

bool CCrypter::Decrypt(const std::vector<unsigned char>& vchCiphertext, CKeyingMaterial& vchPlaintext)
{
    if (!fKeySet)
        return false;

    // plaintext will always be equal to or lesser than length of ciphertext
    int nLen = vchCiphertext.size();

    vchPlaintext.resize(nLen);

    AES256CBCDecrypt dec(chKey, chIV, true);
    nLen = dec.Decrypt(vchCiphertext.data(), vchCiphertext.size(), &vchPlaintext[0]);
    if(nLen == 0)
        return false;
    vchPlaintext.resize(nLen);
    return true;
}


bool EncryptSecret(CKeyingMaterial& vMasterKey, const CSecret &vchPlaintext, const uint256& nIV, std::vector<unsigned char> &vchCiphertext)
{
    CCrypter cKeyCrypter;
    std::vector<unsigned char> chIV(WALLET_CRYPTO_KEY_SIZE);
    memcpy(&chIV[0], &nIV, WALLET_CRYPTO_KEY_SIZE);
    if(!cKeyCrypter.SetKey(vMasterKey, chIV))
        return false;
    return cKeyCrypter.Encrypt((CKeyingMaterial)vchPlaintext, vchCiphertext);
}

bool DecryptSecret(const CKeyingMaterial& vMasterKey, const std::vector<unsigned char>& vchCiphertext, const uint256& nIV, CSecret& vchPlaintext)
{
    CCrypter cKeyCrypter;
    std::vector<unsigned char> chIV(WALLET_CRYPTO_KEY_SIZE);
    memcpy(&chIV[0], &nIV, WALLET_CRYPTO_KEY_SIZE);
    if(!cKeyCrypter.SetKey(vMasterKey, chIV))
        return false;
    return cKeyCrypter.Decrypt(vchCiphertext, *((CKeyingMaterial*)&vchPlaintext));
}
