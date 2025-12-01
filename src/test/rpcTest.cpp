#include <gtest/gtest.h>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "raqcoinrpc.h"
#include <boost/algorithm/string.hpp>
#include "boost/filesystem.hpp"

#include "init.h"
#include "db.h"
#if 0
json_spirit::Value CallRPCTest(std::string args)
{
    std::vector<std::string> vArgs;
    boost::split(vArgs, args, boost::is_any_of(" \t"));
    if(vArgs.empty())
    {
        throw std::runtime_error("empty args");
    }

    try
    {
        const json_spirit::Array params = RPCConvertValues(vArgs[0], std::vector<std::string>(vArgs.begin() + 1, vArgs.end()));
        const CRPCCommand *pcmd = tableRPC[vArgs[0]];
        json_spirit::Value result = pcmd->actor(params, false);

        return result;
    }
    catch (json_spirit::Object& objError)
    {
        throw std::runtime_error(find_value(objError, "message").get_str());
    }
    catch (std::exception& e)
    {
        throw std::runtime_error(e.what());
    }
}

void setupWalletTest()
{
    //will use import key to add key to wallet.dat, if re-run, key will conflict in wallet.dat, 
    //so rename wallet.dat to insure this test case would be OK each time runing it
    if (boost::filesystem::exists(GetDataDir() / "wallet_test.dat"))
    {
        std::string fileRename("wallet_test.dat.bk");
        fileRename += std::to_string(GetTime());
        boost::filesystem::rename(GetDataDir()/"wallet_test.dat", GetDataDir()/fileRename.c_str());
    }

    bool fFirstRun = true;
    pwalletMain = new CWallet("wallet_test.dat");
    DBErrors nLoadWalletRet = pwalletMain->LoadWallet(fFirstRun);
    ASSERT_EQ(nLoadWalletRet,DB_LOAD_OK);
}

TEST(rpcTest, rawtransaction)
{
    ASSERT_THROW(CallRPCTest("getrawtransaction"), std::runtime_error);
    ASSERT_THROW(CallRPCTest("getrawtransaction not_hex"), std::runtime_error);
    ASSERT_THROW(CallRPCTest("getrawtransaction a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff150ed not_int"), std::runtime_error);

    ASSERT_THROW(CallRPCTest("createrawtransaction"), std::runtime_error);
    ASSERT_THROW(CallRPCTest("createrawtransaction null null"), std::runtime_error);
    ASSERT_THROW(CallRPCTest("createrawtransaction not_array"), std::runtime_error);
    ASSERT_THROW(CallRPCTest("createrawtransaction {} {}"), std::runtime_error);
    ASSERT_NO_THROW(CallRPCTest("createrawtransaction [] {}"));
    ASSERT_THROW(CallRPCTest("createrawtransaction [] {} extra"), std::runtime_error);

    ASSERT_THROW(CallRPCTest("decoderawtransaction"), std::runtime_error);
    ASSERT_THROW(CallRPCTest("decoderawtransaction null"), std::runtime_error);
    ASSERT_THROW(CallRPCTest("decoderawtransaction DEADBEEF"), std::runtime_error);
    std::string rawtx = "01000000010b30d2994962b03b48cfceb18b4bdc519974d3d29d6cf6ed31854de85b6cc9e50000000000ffffffff0100e40b54020000002576aa20d1ac643e112b6f1d9539bb943104d00b932808a3073783108490051635f976ea88ac00000000";
    json_spirit::Value result = CallRPCTest(std::string("decoderawtransaction ")+rawtx);
    ASSERT_EQ(find_value(result.get_obj(), "version").get_int(), 1);
    ASSERT_EQ(find_value(result.get_obj(), "locktime").get_int(), 0);
    ASSERT_THROW(CallRPCTest(std::string("decoderawtransaction ")+rawtx+" extra"), std::runtime_error);
    ASSERT_THROW(CallRPCTest(std::string("decoderawtransaction ")+rawtx+" false"), std::runtime_error);
    ASSERT_THROW(CallRPCTest(std::string("decoderawtransaction ")+rawtx+" false extra"), std::runtime_error);

    // Only check failure cases for sendrawtransaction, there's no network to send to...
    ASSERT_THROW(CallRPCTest("sendrawtransaction"), std::runtime_error);
    ASSERT_THROW(CallRPCTest("sendrawtransaction null"), std::runtime_error);
    ASSERT_THROW(CallRPCTest("sendrawtransaction DEADBEEF"), std::runtime_error);
    ASSERT_THROW(CallRPCTest(std::string("sendrawtransaction ")+rawtx+" extra"), std::runtime_error);
}

TEST(rpcTest, network)
{
    ASSERT_NO_THROW(CallRPCTest("getpeerinfo"));
    ASSERT_THROW(CallRPCTest("getpeerinfo false"), std::runtime_error);

    json_spirit::Value result;
    result = CallRPCTest("getconnectioncount");
    ASSERT_EQ(result.get_int(), 0);

    ASSERT_NO_THROW(CallRPCTest("addnode 192.168.1.1 add"));

    result = CallRPCTest("getaddednodeinfo false");
    ASSERT_STREQ(find_value(result.get_obj(), "addednode").get_str().c_str(), "192.168.1.1");

    ASSERT_THROW(CallRPCTest("addnode 192.168.1.1 add"), std::runtime_error);
    ASSERT_NO_THROW(CallRPCTest("addnode 192.168.1.1 remove"));

}

TEST(rpcTest, mining)
{
    json_spirit::Value result;
    result = CallRPCTest("getgenerate");
    ASSERT_EQ(result.get_bool(), false);

    ASSERT_THROW(CallRPCTest("setgenerate"), std::runtime_error);
    ASSERT_NO_THROW(CallRPCTest("setgenerate true"));

    ASSERT_THROW(CallRPCTest("getgenerate 1"), std::runtime_error);
    result = CallRPCTest("getgenerate");
    ASSERT_EQ(result.get_bool(), true);

    ASSERT_THROW(CallRPCTest("getmininginfo extra"), std::runtime_error);
    result = CallRPCTest("getmininginfo");
    ASSERT_EQ(find_value(result.get_obj(), "generate").get_bool(), true);
    ASSERT_EQ(find_value(result.get_obj(), "difficulty").get_int(), 41);

    ASSERT_THROW(CallRPCTest("getwork true"), std::runtime_error);
    ASSERT_THROW(CallRPCTest("getblocktemplate true"), std::runtime_error);

    ASSERT_THROW(CallRPCTest("getwork"), std::runtime_error); //not in server mode, should receive runtime_error
    ASSERT_THROW(CallRPCTest("getblocktemplate"), std::runtime_error); //not in server mode, should receive runtime_error
}
/*
TEST(rpcTest, publickeypos)
{
    setupWalletTest();
    //an address not exist
    ASSERT_THROW(CallRPCTest("getpublickeypos 8A778HtteQY3PeZs8J9w2KJGVMd9M4jpFSv2QNZNXspaM2EGt"), std::runtime_error);

    //create a new key/address
    ASSERT_NO_THROW(CallRPCTest("getaccountaddress publickeypos"));
    json_spirit::Value result;
    result = CallRPCTest("getaddressesbyaccount publickeypos");
    ASSERT_EQ(result.type(), json_spirit::array_type);

    json_spirit::Array ar = result.get_array();
    ASSERT_EQ(ar.size(), (unsigned int)1);
    ASSERT_EQ(ar[0].type(), json_spirit::str_type);
    //position is null
    ASSERT_THROW(CallRPCTest(std::string("getpublickeypos ")+ ar[0].get_str()), std::runtime_error);

}


TEST(rpcTest, dumpRpc)
{
    setupWalletTest();

    ASSERT_THROW(CallRPCTest("dumpkey"), std::runtime_error);
    ASSERT_THROW(CallRPCTest("importkey"), std::runtime_error);

    ASSERT_NO_THROW(CallRPCTest("getaccountaddress keyaccount1"));
    json_spirit::Value result;
    result = CallRPCTest("getaddressesbyaccount keyaccount1");
    ASSERT_EQ(result.type(), json_spirit::array_type);

    json_spirit::Array ar = result.get_array();
    ASSERT_EQ(ar.size(), (unsigned int)1);
    ASSERT_EQ(ar[0].type(), json_spirit::str_type);
    result = CallRPCTest(std::string("dumpkey ")+ ar[0].get_str());

    std::vector<std::string> vRet;
    boost::split(vRet, result.get_str(), boost::is_any_of("|"));
    ASSERT_EQ(vRet.size(), (unsigned int)3);
    ASSERT_STREQ("ffffffff00000000", vRet[2].c_str());

    FILE * pFile;
    pFile = fopen ("./test/data/importkey.txt","r");
    ASSERT_TRUE(pFile!=NULL);
    fseek (pFile , 0 , SEEK_SET);
    char* buffer = (char*) malloc (sizeof(char)*344611);
    ASSERT_TRUE(buffer!=NULL);
    memset(buffer, 0, sizeof(char)*344611);
    size_t ret = fread(buffer,sizeof(char),344610,pFile);
    ASSERT_EQ(ret, (unsigned int)344610);
    ASSERT_NO_THROW(CallRPCTest(std::string(buffer)));

    result = CallRPCTest("getaddressesbyaccount win_8");
    ASSERT_EQ(result.type(), json_spirit::array_type);
    ar = result.get_array();
    ASSERT_EQ(ar.size(), (unsigned int)1);
    ASSERT_EQ(ar[0].type(), json_spirit::str_type);

    ASSERT_STREQ(ar[0].get_str().c_str(), "8348w1QCQJfSNv24Bfe9h4oXGQF14kBspusWmspq5VnCPs4c2");

    free(buffer);

}
*/
#endif
