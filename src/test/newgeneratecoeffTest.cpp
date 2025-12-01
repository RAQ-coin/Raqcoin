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
const static unsigned int NUM_EQUATIONS = 16;


static void PrintMatrix(uint8_t *M, int row, int col) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            std::cout<<" "<<(int)M[i*col+j];
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

static void PrintArray(uint8_t *a, int len) {
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

static void  TNewGenCoeffMatrix(uint256 hash, unsigned int nBits, std::vector<uint8_t> &coeffM) {
    unsigned int mEquations = nBits;
    unsigned int nUnknowns = nBits+8;
    unsigned int nTerms = 1 + (nUnknowns+1)*(nUnknowns)/2;
	//printf("\n **********************NewGenCoeffMatrix ***********************\n");

    //generate the first polynomial coefficients.
    unsigned char in[32], out[32];
    unsigned int count = 0, i, j ,k;
    uint8_t g[nTerms];
    std::bitset<256> bits;
    pqcSha256(hash.begin(),32,in);

	for (i = 0; i < mEquations ; i++) {
	    count = 0;
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
        for (j = 0; j < nTerms; j++) {
            coeffM[i*nTerms+j] = g[j];
        }
    }
   // std::cout<<"coefficients matrix: "<<std::endl;
   // PrintMatrix(coeffM.data(),mEquations,nTerms);

}


TEST(newgenratecoeff, NewGenerateCoeff) {
    uint256 prehash = GetRandHash();
    uint256 merkelRootHash =  GetRandHash();
    uint256 tempHash = prehash ^ merkelRootHash;
    uint256 seedHash = Hash(BEGIN(tempHash), END(tempHash));

    unsigned int mEquations = 8;
    unsigned int nUnknowns = 8 + 8;
    unsigned int nTerms = 1 + (nUnknowns+1)*(nUnknowns)/2;
    std::vector<uint8_t> coeffMatrix;
    coeffMatrix.resize(mEquations*nTerms);

	TNewGenCoeffMatrix(seedHash, mEquations, coeffMatrix);
}
