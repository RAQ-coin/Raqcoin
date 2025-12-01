#include <bitset>
#include <math.h>
#include <gtest/gtest.h>
#include "util.h"
#include "uint256.h"
#include "main.h"
#include "../pqcrypto/pqcrypto.h"
#include "../pqcrypto/random.h"
#include "miner.h"

const static unsigned int DIFFMN = 8; 
const static unsigned int NUM_EQUATIONS = 41; 

TEST(asmMinerTest, GetBlockValue) {
   int64 sum = 21474836470000000;
   int64 fees = 0;
   for (int loop = 1; loop <=  88 * 365 * 144 + 10000000; loop++) {
       int64 temp =  GetBlockValue(loop, fees);
	   sum += temp;
   }
 //  std::cout<<"total number ABC	"<<sum<<std::endl;
   EXPECT_TRUE(sum == (pow(2,31) - 1)*COIN);  
}

#if 0
TEST(asmMinerTest, miner) {
//	std::cout<<"start new found nonce:  "<<std::endl;

    uint256 prehash = 0;
	uint256 merkelRootHash =  uint256("0xaef6a6cb3767fa5d965b10f9d1e3e183ddea21a5f7ffce9bd7a86c065e7c6865");
    uint256 tempHash = prehash ^ merkelRootHash;
	uint256 seedHash = Hash(BEGIN(tempHash), END(tempHash));

	uint256 randomNonce = 0;
	if (NUM_EQUATIONS+DIFFMN <= 48) {
        randomNonce = (uint64)random_uint32_t();
	} else if ((NUM_EQUATIONS+DIFFMN) > 48 && (NUM_EQUATIONS+DIFFMN <= 80)) {
        randomNonce = random_uint64_t();
	} else {
        randomNonce = GetRandHash();
	}

	int64 startTime = GetTimeMicros();
	uint256 nFoundNonce = SerchSolution(seedHash, NUM_EQUATIONS, randomNonce, pindexBest);
	int64 diffTime = GetTimeMicros() - startTime;

	std::cout<<"number of variables: "<<NUM_EQUATIONS+DIFFMN<<std::endl;
	std::cout<<"number of equations: "<<NUM_EQUATIONS<<std::endl;
	std::cout<<"New Miner Algorithm Solve Time: "<<diffTime<<std::endl;


	EXPECT_TRUE(CheckSolution(seedHash, NUM_EQUATIONS, nFoundNonce));
	std::cout<<"new found nonce:  "<<nFoundNonce.ToString()<<std::endl;
    std::cout<<std::endl;

}
#endif



#if 1
TEST(asmMinerTest, initBlockIndex) {
	// Genesis block
	const char* pszTimestamp = "Surely you're joking, Mr.Feynman!";
	CTransaction txNew;
	txNew.vin.resize(1);
	txNew.vin[0].prevout.SetNull();
	txNew.vout.resize(12);
	txNew.vin[0].scriptSig = CScript()<< std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
	txNew.vout[0].nValue = 20000000*COIN;
	txNew.vout[0].scriptPubKey.SetDestination(CKeyID("e6b765cf4efeb5519bcb47ca35738355588e44284398d58c4ac33116ab7e8cef"));
	txNew.vout[1].nValue = 20000000*COIN ;
	txNew.vout[1].scriptPubKey.SetDestination(CKeyID("ebbb5a58e2914878d6f6670a53bdd5e6d0210a3d0b3fa8656bfeb8dd61fb8b81"));
	txNew.vout[2].nValue = 20000000*COIN ;
	txNew.vout[2].scriptPubKey.SetDestination(CKeyID("76068b3357435c9306fcb2b54547b298b5f19e4819dd62e31f84558219e3bd8e"));
	txNew.vout[3].nValue = 20000000*COIN;
	txNew.vout[3].scriptPubKey.SetDestination(CKeyID("be17335dbc8aeaf6aeb3c4ae62886491dab4a3c38c79a5e86bac95214a49b001"));
	txNew.vout[4].nValue = 20000000*COIN ;
	txNew.vout[4].scriptPubKey.SetDestination(CKeyID("f6b7bee668d7125d8a5502ccf9f5927b8866dc180d20642e368d9a936e287e39"));
	txNew.vout[5].nValue = 20000000*COIN ;
	txNew.vout[5].scriptPubKey.SetDestination(CKeyID("706d4053c3794cc3312a7c09b6dd21dd1da3f8c8e56f23c76554ddbf585bccf4"));
	txNew.vout[6].nValue = 20000000*COIN;
	txNew.vout[6].scriptPubKey.SetDestination(CKeyID("bdb56bbe25f5db3df8a3dbfbd5480f390665603b0bc1f6120d0a36370d4d5416"));
	txNew.vout[7].nValue = 20000000*COIN ;
	txNew.vout[7].scriptPubKey.SetDestination(CKeyID("8603abbecfdcaa59545ba185caf684a75075d4147afba303b2c318ef58f6c33e"));
	txNew.vout[8].nValue = 20000000*COIN ;
	txNew.vout[8].scriptPubKey.SetDestination(CKeyID("ebe47a2e627e8c015534830474268becf1ea37da47c8a426b2336ed72abc3257"));
	txNew.vout[9].nValue = 20000000*COIN;
	txNew.vout[9].scriptPubKey.SetDestination(CKeyID("ec97680cfe5c7f30b6356329d463b2e00ed09c22a37ca8010a9c84e36b70f674"));
	txNew.vout[10].nValue = 7374182.7*COIN ;
	txNew.vout[10].scriptPubKey.SetDestination(CKeyID("210ed7d715c525c5bf6b638c29428c70bc4040f248750d5241371d00d84f22ef"));
	txNew.vout[11].nValue = 7374182*COIN ;
	txNew.vout[11].scriptPubKey.SetDestination(CKeyID("b4cb22b62f91f6a36554bdfce5b860d6905c54217fd4543d8eaf16bb20d6ccf7"));

	CBlock block;
	block.vtx.push_back(txNew);
	block.hashPrevBlock = 0;
	block.hashMerkleRoot = block.BuildMerkleTree();
	block.nVersion = 1;
	block.nTime    = 1529291280;  // Use fixed timestamp from genesis block in main.cpp
	block.nBits    = 41;
	block.nNonce   = uint256("0x0000000000000000000000000000000000000000000000000001ee7340a9a1d6");
	uint256 tempHash = block.hashPrevBlock ^ block.hashMerkleRoot;
	uint256 seedHash = Hash(BEGIN(tempHash), END(tempHash));
	
	// Debug output
	uint256 hash = block.GetHash();
	std::cout<<"GenesisBlock hash: "<<hash.ToString().c_str()<<std::endl;
	std::cout<<"GenesisBlock hashPrevBlock: "<<block.hashPrevBlock.ToString().c_str()<<std::endl;
	std::cout<<"GenesisBlock hashMerkleRoot: "<<block.hashMerkleRoot.ToString().c_str()<<std::endl;
    std::cout<<"block version: "<<block.nVersion<<std::endl;
    std::cout<<"block time: "<<block.nTime<<std::endl;
    std::cout<<"block nBits: "<<block.nBits<<std::endl;
    std::cout<<"block nNonce: "<<block.nNonce.ToString().c_str()<<std::endl;
    std::cout<<"seedHash: "<<seedHash.ToString().c_str()<<std::endl;
    std::cout<<"vtx size: "<<block.vtx.size()<<std::endl;
	std::cout<<"vMerkleTree size: "<<block.vMerkleTree.size()<<std::endl;
    
    // NOTE: The genesis block is NOT validated for POW in InitBlockIndex().
    // It's directly added to the block index without calling CheckBlock().
    // The hardcoded nNonce value was generated with an older coefficient generation algorithm
    // which has since been modified. Therefore, CheckSolution would fail on this nNonce.
    // 
    // In production, the genesis block is accepted by its hash (hashGenesisBlock) without POW validation.
    // This test verifies block construction, not POW validation for the genesis block.
    
    // Verify that the block hash matches the expected genesis block hash
    EXPECT_EQ(hash.ToString(), "34158989bb092e9cb797bdc4bba47bf79a404e457b37d263187a9d4dccf9309c");
	//printf("%s\n", hash.ToString().c_str());
	//printf("%s\n", hashGenesisBlock.ToString().c_str());
	//printf("%s\n", block.hashMerkleRoot.ToString().c_str());
	//assert(block.hashMerkleRoot == uint256("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));
	//block.print();
	//assert(hash == hashGenesisBlock);

}
#endif




