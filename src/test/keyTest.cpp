#include <gtest/gtest.h>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "util.h"
#include "base58.h"
#include "pqcrypto/random.h"
#include "pqcrypto/pqcrypto.h"
#include "key.h"


void printKeyInf() {
    
}

void printStr(const unsigned char *cstr, unsigned int cstrlen) {
	std::string str = HexStr(cstr, cstr+cstrlen);
	std::cout<<str<<std::endl;
}

#if 0
TEST(keyTest, keyGen) {
    CKey key;
	key.MakeNewKey();
	CPrivKey sk;
	unsigned int printLen = 32;
	
	sk = key.GetPrivKey();
	//printStr(sk.data(), printLen);
	//printStr(sk.data()+RAINBOW_PRIVATE_KEY_SIZE-printLen, printLen);

    CPubKey pk;
	pk = key.GetPubKey();
	//printStr(pk.vchPubKey.data(), printLen);
	//printStr(pk.vchPubKey.data()+RAINBOW_PUBLIC_KEY_SIZE-printLen, printLen);

	
    CKeyID id = pk.GetID();
	CRaqcoinAddress addr;
	EXPECT_TRUE(addr.Set(id));
	std::cout<<"key id : "<<id.ToString()<<std::endl;
    std::cout<<"key address : "<<addr.ToString()<<std::endl;

}

TEST(keyTest, keySignAndVerify) {
    CKey key1, key2;
	CPrivKey sk1, sk2;
    CPubKey pk1, pk2;
	unsigned int printLen = 32;
    for (int i = 0; i < 10; i++) {
	key1.MakeNewKey();
    key2.MakeNewKey();

    std::cout<<"sk1"<<std::endl;
	sk1 = key1.GetPrivKey();
	//printStr(sk1.data(), printLen);
	//printStr(sk1.data()+RAINBOW_PRIVATE_KEY_SIZE-printLen, printLen);
	pk1 = key1.GetPubKey();
	//printStr(pk1.vchPubKey.data(), printLen);
	//printStr(pk1.vchPubKey.data()+RAINBOW_PUBLIC_KEY_SIZE-printLen, printLen);
    CKeyID id = pk1.GetID();
	CRaqcoinAddress addr;
	EXPECT_TRUE(addr.Set(id));
	//std::cout<<"sk1 key id : "<<id.ToString()<<std::endl;
    std::cout<<"sk1 key address : "<<addr.ToString()<<std::endl;

    //std::cout<<"sk2"<<std::endl;
	sk2 = key2.GetPrivKey();
	//printStr(sk2.data(), printLen);
	//printStr(sk2.data()+RAINBOW_PRIVATE_KEY_SIZE-printLen, printLen);
	pk2 = key2.GetPubKey();
	//printStr(pk2.vchPubKey.data(), printLen);
	//printStr(pk2.vchPubKey.data()+RAINBOW_PUBLIC_KEY_SIZE-printLen, printLen);
	id = pk2.GetID();
	EXPECT_TRUE(addr.Set(id));
	//std::cout<<"sk2 key id : "<<id.ToString()<<std::endl;
    std::cout<<"sk2 key address : "<<addr.ToString()<<std::endl;

    }
	
	for (int n=0; n<1600; n++)
    {
        std::string strMsg = strprintf("Very secret message %i: 11", n);
        uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());

        // normal signatures

        std::vector<unsigned char> sign1, sign2;

        EXPECT_TRUE(key1.Sign(hashMsg, sign1));
        EXPECT_TRUE(key2.Sign(hashMsg, sign2));

        EXPECT_TRUE( key1.pubKey.Verify(hashMsg, sign1));
        EXPECT_FALSE(key1.pubKey.Verify(hashMsg, sign2));

        EXPECT_FALSE(key2.pubKey.Verify(hashMsg, sign1));
        EXPECT_TRUE( key2.pubKey.Verify(hashMsg, sign2));
    }
}
#endif
