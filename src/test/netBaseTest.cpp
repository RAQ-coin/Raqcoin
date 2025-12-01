// Copyright (c) 2012-2017 The Bitcoin Core developers
// Copyright (c) 2018 The AbcmintCore developers

#include <gtest/gtest.h>
#include "netbase.h"
#include <string>
#include "pqcrypto/pqcrypto.h"

static CNetAddr ResolveIP(const char* ip)
{
    CNetAddr addr;
    LookupHost(ip, addr, false);
    return addr;
}

static CNetAddr CreateInternal(const char* host)
{
    CNetAddr addr;
    addr.SetInternal(host);
    return addr;
}

TEST(netBaseTest, netBaseNetworks) {
    EXPECT_TRUE(ResolveIP("127.0.0.1").GetNetwork()                              == NET_UNROUTABLE);
    EXPECT_TRUE(ResolveIP("::1").GetNetwork()                                    == NET_UNROUTABLE);
    EXPECT_TRUE(ResolveIP("8.8.8.8").GetNetwork()                                == NET_IPV4);
    EXPECT_TRUE(ResolveIP("2001::8888").GetNetwork()                             == NET_IPV6);
    EXPECT_TRUE(ResolveIP("FD87:D87E:EB43:edb1:8e4:3588:e546:35ca").GetNetwork() == NET_TOR);
	//std::string msg("raqcoin");
	//unsigned char out[32] = {};
	//pqcSha256((const unsigned char*)msg.data(),msg.size(), out);
	//for (int i = 0; i < 32; i++)
	  //  std::cout<<std::hex<<(unsigned int)out[i]<<"  ";
	//std::cout<<std::endl;
}

TEST(netBaseTest, netBaseProperties) {
    EXPECT_TRUE(ResolveIP("127.0.0.1").IsIPv4());
    EXPECT_TRUE(ResolveIP("::FFFF:192.168.1.1").IsIPv4());
    EXPECT_TRUE(ResolveIP("::1").IsIPv6());
    EXPECT_TRUE(ResolveIP("10.0.0.1").IsRFC1918());
    EXPECT_TRUE(ResolveIP("192.168.1.1").IsRFC1918());
    EXPECT_TRUE(ResolveIP("172.31.255.255").IsRFC1918());
    EXPECT_TRUE(ResolveIP("2001:0DB8::").IsRFC3849());
    EXPECT_TRUE(ResolveIP("169.254.1.1").IsRFC3927());
    EXPECT_TRUE(ResolveIP("2002::1").IsRFC3964());
    EXPECT_TRUE(ResolveIP("FC00::").IsRFC4193());
    EXPECT_TRUE(ResolveIP("2001::2").IsRFC4380());
    EXPECT_TRUE(ResolveIP("2001:10::").IsRFC4843());
    EXPECT_TRUE(ResolveIP("FE80::").IsRFC4862());
    EXPECT_TRUE(ResolveIP("64:FF9B::").IsRFC6052());
    EXPECT_TRUE(ResolveIP("FD87:D87E:EB43:edb1:8e4:3588:e546:35ca").IsTor());
    EXPECT_TRUE(ResolveIP("127.0.0.1").IsLocal());
    EXPECT_TRUE(ResolveIP("::1").IsLocal());
    EXPECT_TRUE(ResolveIP("8.8.8.8").IsRoutable());
    EXPECT_TRUE(ResolveIP("2001::1").IsRoutable());
    EXPECT_TRUE(ResolveIP("127.0.0.1").IsValid());

}

TEST(netBaseTest, CNetAddr_Constructor) {
   CNetAddr addr("127.0.0.1");
   EXPECT_EQ("127.0.0.1", addr.ToString());
//   std::cout<<"CNetAddr()"<<addr.ToString().c_str()<<std::endl;   
//   std::cout<<"CNetAddr()"<<addr.ToStringIP().c_str()<<std::endl;
}


bool static TestSplitHost(std::string test, std::string host, int port)
{
    std::string hostOut;
    int portOut = -1;
    SplitHostPort(test, portOut, hostOut);
    return hostOut == host && port == portOut;
}

TEST(netBaseTest, netBaseSplithost)
{
    EXPECT_TRUE(TestSplitHost("www.abcmint.org", "www.abcmint.org", -1));
    EXPECT_TRUE(TestSplitHost("[www.abcmint.org]", "www.abcmint.org", -1));
    EXPECT_TRUE(TestSplitHost("www.abcmint.org:80", "www.abcmint.org", 80));
    EXPECT_TRUE(TestSplitHost("[www.abcmint.org]:80", "www.abcmint.org", 80));
    EXPECT_TRUE(TestSplitHost("127.0.0.1", "127.0.0.1", -1));
    EXPECT_TRUE(TestSplitHost("127.0.0.1:8888", "127.0.0.1", 8888));
    EXPECT_TRUE(TestSplitHost("[127.0.0.1]", "127.0.0.1", -1));
    EXPECT_TRUE(TestSplitHost("[127.0.0.1]:8888", "127.0.0.1", 8888));
    EXPECT_TRUE(TestSplitHost("::ffff:127.0.0.1", "::ffff:127.0.0.1", -1));
    EXPECT_TRUE(TestSplitHost("[::ffff:127.0.0.1]:8888", "::ffff:127.0.0.1", 8888));
    EXPECT_TRUE(TestSplitHost("[::]:8888", "::", 8888));
    EXPECT_TRUE(TestSplitHost("::8888", "::8888", -1));
    EXPECT_TRUE(TestSplitHost(":8888", "", 8888));
    EXPECT_TRUE(TestSplitHost("[]:8888", "", 8888));
    EXPECT_TRUE(TestSplitHost("", "", -1));
}

bool static TestParse(std::string src, std::string canon)
{
    CService addr;
    if (!LookupNumeric(src.c_str(), addr, 65535))
        return canon == "";
    return canon == addr.ToString();
}

TEST(netBaseTest, netBaseLookupNumeric)
{
    EXPECT_TRUE(TestParse("127.0.0.1", "127.0.0.1:65535"));
    EXPECT_TRUE(TestParse("127.0.0.1:8888", "127.0.0.1:8888"));
    EXPECT_TRUE(TestParse("::ffff:127.0.0.1", "127.0.0.1:65535"));
    EXPECT_TRUE(TestParse("::", "[::]:65535"));
    EXPECT_TRUE(TestParse("[::]:8888", "[::]:8888"));
    EXPECT_TRUE(TestParse("[127.0.0.1]", "127.0.0.1:65535"));
    EXPECT_TRUE(TestParse(":::", ""));
}

TEST(netBaseTest, onioncatTest)
{
    // values from https://web.archive.org/web/20121122003543/http://www.cypherpunk.at/onioncat/wiki/OnionCat
    CNetAddr addr1("5wyqrzbvrdsumnok.onion");
    CNetAddr addr2("FD87:D87E:EB43:edb1:8e4:3588:e546:35ca");
    EXPECT_TRUE(addr1 == addr2);
    EXPECT_TRUE(addr1.IsTor());
    EXPECT_TRUE(addr1.ToStringIP() == "5wyqrzbvrdsumnok.onion");
    EXPECT_TRUE(addr1.IsRoutable());
}


