#include <bitset>
#include <math.h>
#include <gtest/gtest.h>
#include "util.h"
#include "uint256.h"
#include "main.h"
#include "../pqcrypto/pqcrypto.h"
#include "../pqcrypto/random.h"
#include "miner.h"

const static unsigned int addN = 8;
//const static unsigned int NUM_EQUATIONS = 16;

#if 1

void PrintMatrix(uint8_t *M, int row, int col) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            std::cout<<" "<<(int)M[i*col+j];
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

void PrintArray(uint8_t *a, int len) {
    for (int i = 0; i < len; i++)
        std::cout<<" "<<(int)a[i];
    std::cout<<std::endl;
}

static std::bitset<8> ByteToBits(unsigned char byte) {
    return std::bitset<8>(byte);
}

static void Uint256ToBits(const unsigned char *bytes, std::bitset<256> &bits) {
    for (int i = 0; i < 32; i++) {
        std::bitset<8> tempbits = ByteToBits(bytes[i]);
        for (int j = 0; j < 8; j++) {
            bits[i*8+j] = tempbits[j];
        }
    }
}

static void ArrayShiftRight(uint8_t array[], int len, int nShift) {
    int i = 0;
    uint8_t temp;
    do {
        i = (i+nShift) % len;
        temp = array[i];
        array[i] = array[0];
        array[0] = temp;
    } while(i);
}

static void  GenCoeffMatrix(uint256 hash, unsigned int nBits, std::vector<uint8_t> &coeffM) {
    unsigned int mEquations = nBits;
    unsigned int nUnknowns = nBits+addN;
    unsigned int nTerms = 1 + nUnknowns*(nUnknowns+1)/2;

    //generate the first polynomial coefficients.
    unsigned char in[32], out[32];
    unsigned int count = 0, i, j ,k;
    uint8_t g[nTerms];
    std::bitset<256> bits;
    pqcSha256(hash.begin(),32,in);

    do {
         pqcSha256(in,32,out);
         Uint256ToBits(out, bits);
         for (k = 0; k < 256; k++) {
             if(count < nTerms) {
                 g[count++] = (uint8_t)bits[k];
             } else {
                 break;
             }
         }
         for (j = 0; j < 32; j++) {
             in[j] = out[j];
         }
     } while(count < nTerms);

    //generate the rest polynomials coefficients by shiftint f[0] one bit
    for (i = 0; i < mEquations ; i++) {
        ArrayShiftRight(g, nTerms, 1);
        //std::cout<<"g["<<i<<"]: "<<std::endl;
        //PrintArray(g,nTerms);
        for (j = 0; j < nTerms; j++)
            coeffM[i*nTerms+j] = g[j];
    }
    //std::cout<<"coefficients matrix: "<<std::endl;
    //PrintMatrix(coeffM.data(),mEquations,nTerms);

}

static void Uint256ToSolutionBits(uint8_t *x, unsigned int nUnknowns, uint256 nonce) {
    std::bitset<256> bitnonce;
    Uint256ToBits(nonce.begin(), bitnonce);
    if (nUnknowns < 256) {
        for (unsigned int i = 0; i < nUnknowns; i++) {
                x[i] = (uint8_t)bitnonce[i];
        }
    } else {
        for (unsigned int i = 0; i < 256; i++) {
            x[i] = (uint8_t)bitnonce[i];
        }
    }
}

uint256 TSerchSolution(uint256 hash, unsigned int nBits, uint256 randomNonce) {
    unsigned int mEquations = nBits;
    unsigned int nUnknowns = nBits + addN;
    unsigned int nTerms = 1 + (nUnknowns+1)*(nUnknowns)/2;
    std::vector<uint8_t> coeffMatrix;
    coeffMatrix.resize(mEquations*nTerms);
    GenCoeffMatrix(hash, nBits, coeffMatrix);

    //serch
    uint8_t x[nUnknowns], y[mEquations], tempbit;
    unsigned int  i, j, k, count;
    int success = 0 ;
    uint256 nonce = randomNonce;    
    //std::cout<<"coeffM: "<<std::endl;
    do {
        Uint256ToSolutionBits(x, nUnknowns, nonce);
        k = 0;
        success = 0;
        do {
            tempbit = 0;
            count = 0;
            //std::cout<<"the ["<<k<<"] time: "<<std::endl;
            for (i = 0; i < nUnknowns; i++) {
                for (j = i+1; j < nUnknowns; j++) {
                    tempbit ^= (coeffMatrix[k*nTerms+count] * x[i] * x[j]);
                   // std::cout<<(int)coeffMatrix[k*nTerms+count]<<" ";
                    count++;

                }
            }
            for (i = 0; i < nUnknowns; i++) {
                //std::cout<<(int)coeffMatrix[k*nTerms+count]<<" ";
                tempbit ^= (coeffMatrix[k*nTerms+count]* x[i]);
                count++;
            }
            tempbit ^= (coeffMatrix[k*nTerms+count]);
            //std::cout<<(int)coeffMatrix[k*nTerms+count]<<" ";
            //std::cout<<std::endl;
            y[k] = tempbit;
            if (tempbit != 0) {
                break;
            } 
            k++;
            //if ((k == mEquations)&& (tempbit == 0))
                //std::cout<<"success!"<<std::endl;
        } while((k < mEquations)&& (tempbit == 0));
        nonce = nonce + (uint256)1;
        //if (pindexPrev != pindexBest)
        //  break;
        for(k = 0; k < mEquations; k++) {
             if (y[k] != 0)
                success = -1;
        }
        if (nonce == -1)
            nonce = 0;
    } while(success == -1);
    return nonce - (uint256)1;

}


bool TCheckSolution(uint256 hash, unsigned int nBits, uint256 nNonce) {
    unsigned int mEquations = nBits;
    unsigned int nUnknowns = nBits+addN;
    unsigned int nTerms = 1 + (nUnknowns+1)*(nUnknowns)/2;
    std::vector<uint8_t> coeffMatrix;
    coeffMatrix.resize(mEquations*nTerms);
    GenCoeffMatrix(hash, nBits, coeffMatrix);
    unsigned int i, j, k, count;
    uint8_t x[nUnknowns], tempbit;
    Uint256ToSolutionBits(x, nUnknowns, nNonce);

    for (k = 0; k < mEquations; k++) {
        tempbit = 0;
        count = 0;
        for (i = 0; i < nUnknowns; i++) {
            for (j = i+1; j < nUnknowns; j++) {
                tempbit ^= (coeffMatrix[k*nTerms+count] * x[i] * x[j]);
                count++;
            }
        }
        for (i = 0; i < nUnknowns; i++) {
            tempbit ^= (coeffMatrix[k*nTerms+count]* x[i]);
            count++;
        }
        tempbit ^= (coeffMatrix[k*nTerms+count]);
        if (tempbit != 0) {
            return false;
        }
    }
    return true;

}


#endif

#if 0
TEST(mineTest, minerTest) {
    uint256 prehash = 0;
    uint256 merkelRootHash =  uint256("0xb9b3a70856505ff00676080f83dccb11085be368dcb9cb8b205638d7039bf07f");
    uint256 tempHash = prehash ^ merkelRootHash;
    uint256 seedHash = Hash(BEGIN(tempHash), END(tempHash));
    int64 timeRecord[100];
    int index = 0;
for (unsigned int NUM_EQUATIONS = 20; NUM_EQUATIONS < 30; NUM_EQUATIONS++) {
    uint256 randomNonce = 0;
    if (NUM_EQUATIONS+addN <= 48) {
        randomNonce = (uint64)random_uint32_t();
    } else if ((NUM_EQUATIONS+addN) > 48 && (NUM_EQUATIONS+addN <= 80)) {
        randomNonce = random_uint64_t();
    } else {
        randomNonce = GetRandHash();
    }
    int64 startTime = GetTimeMicros();
    uint256 nFoundNonce = TSerchSolution(seedHash, NUM_EQUATIONS,randomNonce);
    int64 diffTime = GetTimeMicros() - startTime;
    timeRecord[index++] = diffTime;
    std::cout<<"number of variables: "<<NUM_EQUATIONS+addN<<std::endl;
    std::cout<<"number of equations: "<<NUM_EQUATIONS<<std::endl;
    std::cout<<"Old Miner Algorithm Solve Time: "<<diffTime<<std::endl;
    EXPECT_TRUE(TCheckSolution(seedHash, NUM_EQUATIONS, nFoundNonce));
    std::cout<<"found nonce:  "<<nFoundNonce.ToString()<<std::endl;
}
for (int i = 0; i < 32; i++) {
    std::cout<<timeRecord[i]<<" ";
}
std::cout<<std::endl;

}
#endif

#if 0
TEST(mineTest, initBlockIndex) {
    // Genesis block
    const char* pszTimestamp = "It was the best of times, it was the worst of times, it was the age of wisdom, it was the age of foolishness, it was the epoch of belief, it was the epoch of incredulity, it was the season of Light, it was the season of Darkness, it was the spring of hope, it was the winter of despair, we had everything before us, we had nothing before us, we were all going direct to Heaven, we were all going direct the other way...";
    //uint256 initHash("0x3141592653589793238462643383279502884197169399375105820974944592");
    CTransaction txNew;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() <<ParseHex("3141592653589793238462643383279502884197169399375105820974944592")<< std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = 100 * COIN;
    txNew.vout[0].scriptPubKey = CScript() << ParseHex("b430960cb823bde0c483093ed702a544e029c536408e73823d690e41ca470ff6") << OP_CHECKSIG;
    CBlock block;
    block.vtx.push_back(txNew);
    block.hashPrevBlock = 0;
    block.hashMerkleRoot = block.BuildMerkleTree();
    block.nVersion = 1;
    block.nTime    =  GetTime();
    block.nBits    = 28;
    block.nNonce   = uint256("0x0000000000000000000000000000000000000000000000000000000009408ba1");
        
    //// debug print
    uint256 hash = block.GetHash();
    std::cout<<"GenesisBlock hash: "<<hash.ToString().c_str()<<std::endl;
    std::cout<<"GenesisBlock hashPrevBlock: "<<block.hashPrevBlock.ToString().c_str()<<std::endl;
    std::cout<<"GenesisBlock hashMerkleRoot: "<<block.hashMerkleRoot.ToString().c_str()<<std::endl;
    std::cout<<"block version: "<<block.nVersion<<std::endl;
    std::cout<<"block time: "<<block.nTime<<std::endl;
    std::cout<<"block nBits: "<<block.nBits<<std::endl;
    std::cout<<"block nNonce: "<<block.nNonce.ToString().c_str()<<std::endl;
    std::cout<<"vtx size"<<block.vtx.size()<<std::endl;
    std::cout<<"vMerkleTree size"<<block.vMerkleTree.size()<<std::endl;
    //printf("%s\n", hash.ToString().c_str());
    //printf("%s\n", hashGenesisBlock.ToString().c_str());
    //printf("%s\n", block.hashMerkleRoot.ToString().c_str());
    //assert(block.hashMerkleRoot == uint256("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));
    //block.print();
    //assert(hash == hashGenesisBlock);

}
#endif


