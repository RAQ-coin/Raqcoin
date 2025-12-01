#include <gtest/gtest.h>
#include <boost/test/unit_test.hpp>
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"

#include "base58.h"
#include "util.h"
#include "pqcrypto/random.h"
#include "pqcrypto/pqcrypto.h"
#include "key.h"

using namespace json_spirit;
extern Array read_json(const std::string& filename);

TEST(base58Test, base58_EncodeBase58)
{
    Array tests = read_json("base58_encode_decode.json");

    BOOST_FOREACH(Value& tv, tests)
    {
        Array test = tv.get_array();
        std::string strTest = write_string(tv, false);
        if (test.size() < 2) // Allow for extra stuff (useful for comments)
        {
            BOOST_ERROR("Bad test: " << strTest);
            continue;
        }
        std::vector<unsigned char> sourcedata = ParseHex(test[0].get_str());
        std::string base58string = test[1].get_str();
		//std::cout<<EncodeBase58(&sourcedata[0], &sourcedata[sourcedata.size()])<<std::endl;
        EXPECT_TRUE(EncodeBase58(&sourcedata[0], &sourcedata[sourcedata.size()]) == base58string);
    }
	std::string str = "0123456789abcdef";
    std::vector<unsigned char> srcdata = ParseHex(str); 
	std::string encodedata = EncodeBase58(&srcdata[0], &srcdata[srcdata.size()]);
	//std::cout<<"encodedata    "<<encodedata<<std::endl;
	std::vector<unsigned char> decodedata;
    EXPECT_TRUE(DecodeBase58("C3CPq7c1PY", decodedata));
	EXPECT_TRUE(std::equal(decodedata.begin(), decodedata.end(), srcdata.begin()));

}


// Goal: test low-level base58 decoding functionality
TEST(base58Test, base58_DecodeBase58)
{
    Array tests = read_json("base58_encode_decode.json");
    std::vector<unsigned char> result;

    BOOST_FOREACH(Value& tv, tests)
    {
        Array test = tv.get_array();
        std::string strTest = write_string(tv, false);
        if (test.size() < 2) // Allow for extra stuff (useful for comments)
        {
            BOOST_ERROR("Bad test: " << strTest);
            continue;
        }
        std::vector<unsigned char> expected = ParseHex(test[0].get_str());
        std::string base58string = test[1].get_str();
        DecodeBase58(base58string, result);
		EXPECT_TRUE(result.size() == expected.size());
		EXPECT_TRUE(std::equal(result.begin(), result.end(), expected.begin()));
    }
    EXPECT_FALSE(DecodeBase58("invalid", result));

}

TEST(base58Test, decodeBase58Check) {
	std::string addr[12] = {"8QuKs7NwfvQNKdXNTvCj3xPmP4JV6Bn5iwDGquiCsQR1617TJ",
                            "8DvidgF54gcRQnZEWK1BuJ2Lyu9tD3faMGMvZTtrpBiCMzx1p",
                            "8FF5eDdGFo2Jcgqq1vZcdJeQ5Wgmnon4Ldggkzkdg3hs8i3CL",
                            "8AmoTYrJ9kbJMt2f24o3Pi9fKYfyfERwVxMDL9zoj6jV1Wqi",
                            "86jj8n7n1qQiHJ79G2UyBXg3JTnscVupJotQ6kNk1b5YVRsm3",
                            "8RRhtyWYHUXxKgA4kYbBdm6WLwiStRFVhAe5bXtE2Lsy6vRJA",
                            "83EE3bgxVs5KpkrzhGn5DL3uyqJCdRyyRKgNz9dS8KsuhNAWd",
                            "87GEqYZ4CeeagTghXWDXSXt3mSCvab8wJo2brzGnY99xj9Mtn",
                            "89hdunE6Rut6PWtHB4ftZQRSThFTJpbvbKgT76dfjdzdRPLQD",
                            "8CfteAH1ihcMg5nMv4xQRvQNHi6kgSJzq19dykUKGnWcihKqg",
                            "8Qrve1DTjsHDQ5rhmkH74MWUiA71im48ZqYvx2ofoqX545t4q",
                            "8Rj5WDxT2VaM8HZpPv4uDtae9qLxPsQtLRGtTgUzZhJtpg1px"
	                         };
    CRaqcoinAddress address;
	CKeyID keyID;
	for (int i = 0; i < 12; i++) {
	    EXPECT_TRUE(address.SetString(addr[i]));
		EXPECT_TRUE(address.GetKeyID(keyID));
       // std::cout<<keyID.ToString()<<std::endl;
		//std::cout<<address.ToString()<<std::endl;
	}

}




#if 0

TEST(base58Test, base58_public_key_address) 
{
    CKey key;
	key.MakeNewKey();
	CPrivKey sk;
	sk = key.GetPrivKey();
    CPubKey pk;
	pk = key.GetPubKey();
    CKeyID id = pk.GetID();
	CRaqcoinAddress addr;
	EXPECT_TRUE(addr.Set(id));
	std::cout<<"key id : "<<id.ToString()<<std::endl;
    std::cout<<"key address : "<<addr.ToString()<<std::endl;
	//EXPECT_TRUE(addr.SetString(addr.ToString()));
	EXPECT_TRUE(addr.IsValid());
	EXPECT_FALSE(addr.IsScript());
	CKeyID getkeyID;
	EXPECT_TRUE(addr.GetKeyID(getkeyID));
	std::cout<<"get key id : "<<getkeyID.ToString()<<std::endl;
	EXPECT_EQ(id.ToString(),  getkeyID.ToString());
	
	//std::string longstr = HexStr(sk);
    //std::vector<unsigned char> skhex = ParseHex(longstr);
//	std::string encodedata = EncodeBase58Check(sk);
//    CRaqcoinSecret secret;
//    EXPECT_TRUE(secret.SetString(encodedata));
	
}


TEST(base58Test , test)
{
   // CKey key;
	//key.MakeNewKey();
	//CPrivKey sk;
	//sk = key.GetPrivKey();
	//std::string longstr = HexStr(sk);
	//std::vector<unsigned char> skhex = ParseHex(longstr);

	//int64 nStart = GetTimeMicros();
	//std::string encodedata = EncodeBase58Check(sk);
    //int64 nTime = GetTimeMicros() - nStart;
	//std::cout<<"encodeTime: "<<nTime<<std::endl;
	//std::vector<unsigned char> result;
	//nStart = GetTimeMicros();
	//EXPECT_TRUE(DecodeBase58(encodedata, result));
	//nTime = GetTimeMicros() - nStart;
	//std::cout<<"decodeTime: "<<nTime<<std::endl;
	//CRaqcoinSecret secret;
	//bool b = secret.SetString(encodedata);

}
#endif
