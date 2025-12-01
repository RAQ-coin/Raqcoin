//
// Unit tests for block-chain checkpoints
//
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <gtest/gtest.h>

#include "../checkpoints.h"
#include "../util.h"

using namespace std;

TEST(checkpointTest, sanity)
{
    uint256 p12345 = uint256("0x6a89732f27c4c4d3503fd3c641934c29c924bb2f3459f6e865312e3888fc7b3d");
	uint256 p11111 = uint256("0xae23a3406fbce0049505c3cf2d3ca0e384e27e6ccc45988c556fce4e98b70c01");
    EXPECT_TRUE(Checkpoints::CheckBlock(11111, p11111));
    EXPECT_TRUE(Checkpoints::CheckBlock(12345, p12345));
   // EXPECT_FALSE(Checkpoints::CheckBlock(11110, p11111));
    EXPECT_FALSE(Checkpoints::CheckBlock(11111, p12345));
    EXPECT_TRUE(Checkpoints::GetTotalBlocksEstimate() >= 12345);
}    



