#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

#include "leveldb.h"
#include "uint256.h"
#include "util.h"
#include "pqcrypto/random.h"


// Test if a string consists entirely of null characters
bool is_null_key(const std::vector<unsigned char>& key) {
    bool isnull = true;

    for (unsigned int i = 0; i < key.size(); i++)
        isnull &= (key[i] == '\x00');

    return isnull;
}


TEST(leveldbTest, basic) {
    // Perform tests .
    boost::filesystem::path ph = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    CLevelDB dbw(ph, (1 << 20), true, false);
    char key = 'k';
    uint256 in = GetRandHash();
    uint256 res;
    EXPECT_TRUE(dbw.Write(key, in));
    EXPECT_TRUE(dbw.Read(key, res));
    EXPECT_EQ(res.ToString(), in.ToString());
	EXPECT_TRUE(dbw.Exists(key));
	EXPECT_TRUE(dbw.Erase(key));
	EXPECT_FALSE(dbw.Exists(key));
}

TEST(leveldbTest, writebatch) {
    boost::filesystem::path ph = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    CLevelDB dbw(ph, (1 << 20), true, false);
	char key = 'i';
	uint256 in = GetRandHash();
	char key2 = 'j';
	uint256 in2 = GetRandHash();
	char key3 = 'k';
	uint256 in3 = GetRandHash();
	
	uint256 res;
	CLevelDBBatch batch;
	
	batch.Write(key, in);
	batch.Write(key2, in2);
	batch.Write(key3, in3);

	// Remove key3 before it's even been written
	batch.Erase(key3);
	
	dbw.WriteBatch(batch);
	EXPECT_TRUE(dbw.Read(key, res));
	EXPECT_EQ(res.ToString(), in.ToString());
	EXPECT_TRUE(dbw.Read(key2, res));
	EXPECT_EQ(res.ToString(), in2.ToString());
	
	// key3 should've never been written
	EXPECT_FALSE(dbw.Read(key3, res));
	//std::cout<<"size: "<<batch.SizeEstimate()<<std::endl;

}

