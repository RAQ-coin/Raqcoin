// Copyright (c) 2012-2017 The Bitcoin Core developers
// Copyright (c) 2018 The Abcmint Core Developers
// Copyright (c) 2025 The Raqcoin Core developers

#include <gtest/gtest.h>
#include <string>
#include <db_cxx.h>

#include "addrman.h"
#include "serialize.h"
#include "net.h"
#include "netbase.h"
#include "util.h"
#include "hash.h"
#include "pqcrypto/pqcrypto.h"

/** Access to the (IP) address database (peers.dat) */
class CTAddrDB
{
public:
    CTAddrDB();
    bool Write(const CAddrMan& addr);
    bool Read(CAddrMan& addr);
	boost::filesystem::path pathAddr;
};

CTAddrDB::CTAddrDB()
{
    pathAddr =  boost::filesystem::current_path()/"test"/"testpeers.dat";
}

bool CTAddrDB::Write(const CAddrMan& addr)
{
    // Generate random temporary filename
    unsigned short randv = 0;
    getRandBytes((unsigned char *)&randv, sizeof(randv));
    std::string tmpfn = strprintf("testpeers.dat.%04x", randv);

    // serialize addresses, checksum data up to that point, then append csum
    CDataStream ssPeers(SER_DISK, CLIENT_VERSION);
    ssPeers << FLATDATA(pchMessageStart);
    ssPeers << addr;
    uint256 hash = Hash(ssPeers.begin(), ssPeers.end());
    ssPeers << hash;

    // open temp output file, and associate with CAutoFile
    boost::filesystem::path pathTmp = boost::filesystem::current_path()/"test"/ tmpfn;
    FILE *file = fopen(pathTmp.string().c_str(), "wb");
    CAutoFile fileout = CAutoFile(file, SER_DISK, CLIENT_VERSION);
    if (!fileout)
        return error("CAddrman::Write() : open failed");

    // Write and commit header, data
    try {
        fileout << ssPeers;
    }
    catch (std::exception &e) {
        return error("CAddrman::Write() : I/O error");
    }
    FileCommit(fileout);
    fileout.fclose();

    // replace existing peers.dat, if any, with new testpeers.dat.XXXX
    if (!RenameOver(pathTmp, pathAddr))
        return error("CAddrman::Write() : Rename-into-place failed");

    return true;
}

bool CTAddrDB::Read(CAddrMan& addr)
{
    // open input file, and associate with CAutoFile
    FILE *file = fopen(pathAddr.string().c_str(), "rb");
    CAutoFile filein = CAutoFile(file, SER_DISK, CLIENT_VERSION);
    if (!filein)
        return error("CAddrman::Read() : open failed");

    // use file size to size memory buffer
    int fileSize = GetFilesize(filein);
    int dataSize = fileSize - sizeof(uint256);
    //Don't try to resize to a negative number if file is small
    if ( dataSize < 0 ) dataSize = 0;
    std::vector<unsigned char> vchData;
    vchData.resize(dataSize);
    uint256 hashIn;

    // read data and checksum from file
    try {
        filein.read((char *)&vchData[0], dataSize);
        filein >> hashIn;
    }
    catch (std::exception &e) {
        return error("CAddrman::Read() 2 : I/O error or stream data corrupted");
    }
    filein.fclose();

    CDataStream ssPeers(vchData, SER_DISK, CLIENT_VERSION);

    // verify stored checksum matches input data
    uint256 hashTmp = Hash(ssPeers.begin(), ssPeers.end());
    if (hashIn != hashTmp)
        return error("CAddrman::Read() : checksum mismatch; data corrupted");

    unsigned char pchMsgTmp[4];
    try {
        // de-serialize file header (pchMessageStart magic number) and
        ssPeers >> FLATDATA(pchMsgTmp);

        // verify the network matches ours
        if (memcmp(pchMsgTmp, pchMessageStart, sizeof(pchMsgTmp)))
            return error("CAddrman::Read() : invalid network magic number");

        // de-serialize address data into one CAddrMan object
        ssPeers >> addr;
    }
    catch (std::exception &e) {
        return error("CAddrman::Read() : I/O error or stream data corrupted");
    }

    return true;
}


CDataStream AddrmanToStream(CAddrMan& _addrman) {
    CDataStream ssPeersIn(SER_DISK, CLIENT_VERSION);
    ssPeersIn << FLATDATA(pchMessageStart);
    ssPeersIn << _addrman;
    std::string str = ssPeersIn.str();
    std::vector<unsigned char> vchData(str.begin(), str.end());
    return CDataStream(vchData, SER_DISK, CLIENT_VERSION);
}

TEST(netTest, GetListenPort) {
    // test default
    unsigned short port = GetListenPort();
    EXPECT_TRUE(port == GetDefaultPort());
    // test set port
    unsigned short altPort = 12345;
    EXPECT_TRUE(SoftSetArg("-port", std::to_string(altPort)));
    port = GetListenPort();
    EXPECT_TRUE(port == altPort);
}

TEST(netTest, Add) {
	CAddrMan testaddrman;
    CService addr1, addr2, addr3;
    Lookup("250.7.1.1", addr1, 8888, false);
    Lookup("250.7.2.2", addr2, 9999, false);
    Lookup("250.7.3.3", addr3, 9999, false);
	
    // Add three addresses to new table.
    CService source;
    Lookup("252.5.1.1", source, 8888, false);
    testaddrman.Add(CAddress(addr1, 0), source);
    testaddrman.Add(CAddress(addr2, 0), source);
    testaddrman.Add(CAddress(addr3, 0), source);
	
    // Test that the de-serialization does not throw an exception.
    CDataStream ssPeers1 = AddrmanToStream(testaddrman);
    bool exceptionThrown = false;
    CAddrMan addrman1;

    EXPECT_TRUE(addrman1.size() == 0);
    try {
    	unsigned char pchMsgTmp[4];
	    ssPeers1 >> FLATDATA(pchMsgTmp);
	    ssPeers1 >> addrman1;
    } catch (const std::exception& e) {
	    exceptionThrown = true;
    }
    EXPECT_TRUE(addrman1.size() == 3);
    EXPECT_TRUE(exceptionThrown == false);

    // Test that CAddrDB::Read creates an addrman with the correct number of addrs.
    CDataStream ssPeers2 = AddrmanToStream(testaddrman);
    
    CAddrMan addrman2;
    CTAddrDB adb;
	adb.Write(testaddrman);
    EXPECT_TRUE(addrman2.size() == 0);
    adb.Read(addrman2);
    EXPECT_TRUE(addrman2.size() == 3);

    if (boost::filesystem::exists(adb.pathAddr)) {
        boost::filesystem::remove(adb.pathAddr);
	}
}


