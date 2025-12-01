#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/foreach.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/test/unit_test.hpp>
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"

#include "main.h"
#include "wallet.h"

using namespace std;
using namespace json_spirit;
using namespace boost::algorithm;

TEST(publicKeyPosTest, positionSerialize) {
    CDiskPubKeyPos pos((unsigned int)0, (unsigned int)693534);

    std::vector<unsigned char> v;
    v = pos.ToVector();
    
    CDiskPubKeyPos pos2;
    pos2<<v;
    EXPECT_TRUE(pos==pos2);

}
