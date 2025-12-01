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
#include "miner.h"
#include "wallet.h"
#include "init.h"
#include "db.h"

using namespace std;
using namespace json_spirit;
using namespace boost::algorithm;

extern uint256 SignatureHash(CScript scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType);

static const unsigned int flags = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC;

CScript
ParseScript(std::string s)
{
    CScript result;

    static std::map<std::string, opcodetype> mapOpNames;

    if (mapOpNames.size() == 0)
    {
        for (int op = OP_NOP; op <= OP_NOP10; op++)
        {
            const char* name = GetOpName((opcodetype)op);
            if (strcmp(name, "OP_UNKNOWN") == 0)
                continue;
            std::string strName(name);
            mapOpNames[strName] = (opcodetype)op;
            // Convenience: OP_ADD and just ADD are both recognized:
            replace_first(strName, "OP_", "");
            mapOpNames[strName] = (opcodetype)op;
        }
    }

    std::vector<std::string> words;
    split(words, s, is_any_of(" \t\n"), token_compress_on);

    BOOST_FOREACH(std::string w, words)
    {
        if (all(w, is_digit()) ||
            (starts_with(w, "-") && all(std::string(w.begin()+1, w.end()), is_digit())))
        {
            // Number
            int64 n = atoi64(w);
            result << n;
        }
        else if (starts_with(w, "0x") && IsHex(std::string(w.begin()+2, w.end())))
        {
            // Raw hex data, inserted NOT pushed onto stack:
            std::vector<unsigned char> raw = ParseHex(std::string(w.begin()+2, w.end()));
            result.insert(result.end(), raw.begin(), raw.end());
        }
        else if (w.size() >= 2 && starts_with(w, "'") && ends_with(w, "'"))
        {
            // Single-quoted string, pushed as data. NOTE: this is poor-man's
            // parsing, spaces/tabs/newlines in single-quoted strings won't work.
            std::vector<unsigned char> value(w.begin()+1, w.end()-1);
            result << value;
        }
        else if (mapOpNames.count(w))
        {
            // opcode, e.g. OP_ADD or OP_1:
            result << mapOpNames[w];
        }
        else
        {
            BOOST_ERROR("Parse error: " << s);
            return CScript();
        }
    }

    return result;
}

Array
read_json(const std::string& filename)
{
    namespace fs = boost::filesystem;
    fs::path testFile = fs::current_path() / "test" / "data" / filename;

#ifdef TEST_DATA_DIR
    if (!fs::exists(testFile))
    {
        testFile = fs::path(BOOST_PP_STRINGIZE(TEST_DATA_DIR)) / filename;
    }
#endif

    ifstream ifs(testFile.string().c_str(), ifstream::in);
    Value v;
    if (!read_stream(ifs, v))
    {
        if (ifs.fail())
            BOOST_ERROR("Cound not find/open " << filename);
        else
            BOOST_ERROR("JSON syntax error in " << filename);
        return Array();
    }
    if (v.type() != array_type)
    {
        BOOST_ERROR(filename << " does not contain a json array");
        return Array();
    }

    return v.get_array();
}

CScript
sign_multisig(CScript scriptPubKey, std::vector<CKey> keys, CTransaction transaction)
{
    uint256 hash = SignatureHash(scriptPubKey, transaction, 0, SIGHASH_ALL);

    CScript result;
    //
    // NOTE: CHECKMULTISIG has an unfortunate bug; it requires
    // one extra item on the stack, before the signatures.
    // Putting OP_0 on the stack is the workaround;
    // fixing the bug would mean splitting the block chain (old
    // clients would not accept new CHECKMULTISIG transactions,
    // and vice-versa)
    //
    result << OP_0;
    BOOST_FOREACH(CKey key, keys)
    {
        vector<unsigned char> vchSig;
        // Use config_value=0 (standard Rainbow) for testing
        // Convert uint256 hash to raw bytes
        unsigned char* hashBytes = (unsigned char*)&hash;
        EXPECT_TRUE(key.Sign(0, hashBytes, 32, vchSig, false));
        vchSig.push_back((unsigned char)SIGHASH_ALL);
        result << vchSig;
    }
    return result;
}
CScript
sign_multisig(CScript scriptPubKey, CKey key, CTransaction transaction)
{
    std::vector<CKey> keys;
    keys.push_back(key);
    return sign_multisig(scriptPubKey, keys, transaction);
}


TEST(scriptTest, readJson) {
    // Read tests from test/data/script_valid.json
    // Format is an array of arrays
    // Inner arrays are [ "scriptSig", "scriptPubKey" ]
    // ... where scriptSig and scriptPubKey are stringified
    // scripts.
    Array tests = read_json("script_valid.json");

    BOOST_FOREACH(Value& tv, tests)
    {
        Array test = tv.get_array();
        string strTest = write_string(tv, false);
	//	std::cout<<strTest<<std::endl;
        if (test.size() < 2) // Allow size > 2; extra stuff ignored (useful for comments)
        {
            BOOST_ERROR("Bad test: " << strTest);
            continue;
        }

        std::string scriptSigString = test[0].get_str();
        CScript scriptSig = ParseScript(scriptSigString);
//		std::cout<<scriptSig.ToString()<<std::endl;
        std::string scriptPubKeyString = test[1].get_str();
        CScript scriptPubKey = ParseScript(scriptPubKeyString);
//		std::cout<<scriptPubKey.ToString()<<std::endl;
        int flagsNow = flags;
        if (test.size() > 3 && ("," + test[2].get_str() + ",").find(",DERSIG,") != std::string::npos) {
            flagsNow |= SCRIPT_VERIFY_DERSIG;
        }
        CTransaction tx;
		EXPECT_TRUE(VerifyScript(scriptSig, scriptPubKey, tx, 0, flagsNow, SIGHASH_NONE));
    }

}

TEST(scriptTest, GetOpName) {
    const char* name = GetOpName(opcodetype(0x00));
	EXPECT_STREQ(name, "0");
	name = GetOpName(opcodetype(0xfd));
	EXPECT_STREQ(name, "OP_PUBKEYHASH");
	name = GetOpName(opcodetype(0xfdff));
	EXPECT_STREQ(name, "OP_UNKNOWN");
}


#if 0

TEST(scriptTest, ScriptVerify) {
    CKey key1, key2;
    key1.MakeNewKey();
    key2.MakeNewKey();

    CScript scriptPubKey12;
    scriptPubKey12<<OP_1<<key1.GetPubKey()<<key2.GetPubKey()<<OP_2<<OP_CHECKMULTISIG;
   // std::cout<<scriptPubKey12.ToString()<<std::endl;
    CTransaction txFrom12;
    txFrom12.vout.resize(1);
    txFrom12.vout[0].scriptPubKey = scriptPubKey12;
	//std::cout<<txFrom12.ToString()<<std::endl;
	
    CTransaction txTo12;
    txTo12.vin.resize(1);
    txTo12.vout.resize(1);
    txTo12.vin[0].prevout.n = 0;
    txTo12.vin[0].prevout.hash = txFrom12.GetHash();
    txTo12.vout[0].nValue = 1;
	//std::cout<<txTo12.ToString()<<std::endl;
    CScript goodsig1 = sign_multisig(scriptPubKey12, key1, txTo12);
	//std::cout<<goodsig1.ToString()<<std::endl;
    EXPECT_TRUE(VerifyScript(goodsig1, scriptPubKey12, txTo12, 0, flags, 0));

}


TEST(sciptTest, multiSign) {
    CKey key1, key2, key3;
    key1.MakeNewKey();
    key2.MakeNewKey();
    key3.MakeNewKey();

    CScript scriptPubKey12;
    scriptPubKey12 << OP_1 << key1.GetPubKey() << key2.GetPubKey() << OP_2 << OP_CHECKMULTISIG;

    CTransaction txFrom12;
    txFrom12.vout.resize(1);
    txFrom12.vout[0].scriptPubKey = scriptPubKey12;

    CTransaction txTo12;
    txTo12.vin.resize(1);
    txTo12.vout.resize(1);
    txTo12.vin[0].prevout.n = 0;
    txTo12.vin[0].prevout.hash = txFrom12.GetHash();
    txTo12.vout[0].nValue = 1;

    CScript goodsig1 = sign_multisig(scriptPubKey12, key1, txTo12);
    EXPECT_TRUE(VerifyScript(goodsig1, scriptPubKey12, txTo12, 0, flags, 0));
    txTo12.vout[0].nValue = 2;
    EXPECT_TRUE(!VerifyScript(goodsig1, scriptPubKey12, txTo12, 0, flags, 0));

    CScript goodsig2 = sign_multisig(scriptPubKey12, key2, txTo12);
    EXPECT_TRUE(VerifyScript(goodsig2, scriptPubKey12, txTo12, 0, flags, 0));

    CScript badsig1 = sign_multisig(scriptPubKey12, key3, txTo12);
    EXPECT_TRUE(!VerifyScript(badsig1, scriptPubKey12, txTo12, 0, flags, 0));

}

TEST(scriptTest, multiSign23) {
    CKey key1, key2, key3, key4;
    key1.MakeNewKey();
    key2.MakeNewKey();
    key3.MakeNewKey();
    key4.MakeNewKey();

    CScript scriptPubKey23;
    scriptPubKey23 << OP_2 << key1.GetPubKey() << key2.GetPubKey() << key3.GetPubKey() << OP_3 << OP_CHECKMULTISIG;

    CTransaction txFrom23;
    txFrom23.vout.resize(1);
    txFrom23.vout[0].scriptPubKey = scriptPubKey23;

    CTransaction txTo23;
    txTo23.vin.resize(1);
    txTo23.vout.resize(1);
    txTo23.vin[0].prevout.n = 0;
    txTo23.vin[0].prevout.hash = txFrom23.GetHash();
    txTo23.vout[0].nValue = 1;

    std::vector<CKey> keys;
    keys.push_back(key1); keys.push_back(key2);
    CScript goodsig1 = sign_multisig(scriptPubKey23, keys, txTo23);
    EXPECT_TRUE(VerifyScript(goodsig1, scriptPubKey23, txTo23, 0, flags, 0));

    keys.clear();
    keys.push_back(key1); keys.push_back(key3);
    CScript goodsig2 = sign_multisig(scriptPubKey23, keys, txTo23);
    EXPECT_TRUE(VerifyScript(goodsig2, scriptPubKey23, txTo23, 0, flags, 0));

    keys.clear();
    keys.push_back(key2); keys.push_back(key3);
    CScript goodsig3 = sign_multisig(scriptPubKey23, keys, txTo23);
    EXPECT_TRUE(VerifyScript(goodsig3, scriptPubKey23, txTo23, 0, flags, 0));

    keys.clear();
    keys.push_back(key2); keys.push_back(key2); // Can't re-use sig
    CScript badsig1 = sign_multisig(scriptPubKey23, keys, txTo23);
    EXPECT_TRUE(!VerifyScript(badsig1, scriptPubKey23, txTo23, 0, flags, 0));

    keys.clear();
    keys.push_back(key2); keys.push_back(key1); // sigs must be in correct order
    CScript badsig2 = sign_multisig(scriptPubKey23, keys, txTo23);
    EXPECT_TRUE(!VerifyScript(badsig2, scriptPubKey23, txTo23, 0, flags, 0));

    keys.clear();
    keys.push_back(key3); keys.push_back(key2); // sigs must be in correct order
    CScript badsig3 = sign_multisig(scriptPubKey23, keys, txTo23);
    EXPECT_TRUE(!VerifyScript(badsig3, scriptPubKey23, txTo23, 0, flags, 0));

    keys.clear();
    keys.push_back(key4); keys.push_back(key2); // sigs must match pubkeys
    CScript badsig4 = sign_multisig(scriptPubKey23, keys, txTo23);
    EXPECT_TRUE(!VerifyScript(badsig4, scriptPubKey23, txTo23, 0, flags, 0));

    keys.clear();
    keys.push_back(key1); keys.push_back(key4); // sigs must match pubkeys
    CScript badsig5 = sign_multisig(scriptPubKey23, keys, txTo23);
    EXPECT_TRUE(!VerifyScript(badsig5, scriptPubKey23, txTo23, 0, flags, 0));

    keys.clear(); // Must have signatures
    CScript badsig6 = sign_multisig(scriptPubKey23, keys, txTo23);
    EXPECT_TRUE(!VerifyScript(badsig6, scriptPubKey23, txTo23, 0, flags, 0));

}

TEST(scriptTest, CombineSignatures) {
    // Test the CombineSignatures function
    CBasicKeyStore keystore;
    std::vector<CKey> keys;
    for (int i = 0; i < 3; i++)
    {
        CKey key;
        key.MakeNewKey();
        keys.push_back(key);
        keystore.AddKey(key);
    }

    CTransaction txFrom;
    txFrom.vout.resize(1);
    txFrom.vout[0].scriptPubKey.SetDestination(keys[0].GetPubKey().GetID());
    CScript& scriptPubKey = txFrom.vout[0].scriptPubKey;

    CTransaction txTo;
    txTo.vin.resize(1);
    txTo.vout.resize(1);
    txTo.vin[0].prevout.n = 0;
    txTo.vin[0].prevout.hash = txFrom.GetHash();

    CScript& scriptSig = txTo.vin[0].scriptSig;
    txTo.vout[0].nValue = 1;

    CScript empty;
    CScript combined = CombineSignatures(scriptPubKey, txTo, 0, empty, empty);
    EXPECT_TRUE(combined.empty());

    // Single signature case:
    SignSignature(keystore, txFrom, txTo, 0); // changes scriptSig
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSig, empty);
    EXPECT_TRUE(combined == scriptSig);
    combined = CombineSignatures(scriptPubKey, txTo, 0, empty, scriptSig);
    EXPECT_TRUE(combined == scriptSig);
    CScript scriptSigCopy = scriptSig;
    // Signing again will give a different, valid signature:
    SignSignature(keystore, txFrom, txTo, 0);
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSigCopy, scriptSig);
    EXPECT_TRUE(combined == scriptSigCopy || combined == scriptSig);

    // P2SH, single-signature case:
    CScript pkSingle; pkSingle << keys[0].GetPubKey() << OP_CHECKSIG;
    keystore.AddCScript(pkSingle);
    scriptPubKey.SetDestination(pkSingle.GetID());
    SignSignature(keystore, txFrom, txTo, 0);
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSig, empty);
    EXPECT_TRUE(combined == scriptSig);
    combined = CombineSignatures(scriptPubKey, txTo, 0, empty, scriptSig);
    EXPECT_TRUE(combined == scriptSig);
    scriptSigCopy = scriptSig;
    SignSignature(keystore, txFrom, txTo, 0);
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSigCopy, scriptSig);
    EXPECT_TRUE(combined == scriptSigCopy || combined == scriptSig);
    // dummy scriptSigCopy with placeholder, should always choose non-placeholder:
    scriptSigCopy = CScript() << OP_0 << static_cast<std::vector<unsigned char> >(pkSingle);
	std::cout<<"spk:   "<<scriptPubKey.ToString()<<std::endl;
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSigCopy, scriptSig);
    EXPECT_TRUE(combined == scriptSig);

#if 0	
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSig, scriptSigCopy);
    EXPECT_FALSE(combined == scriptSig);

    // Hardest case:  Multisig 2-of-3
    scriptPubKey.SetMultisig(2, keys);
    keystore.AddCScript(scriptPubKey);
    SignSignature(keystore, txFrom, txTo, 0);
    combined = CombineSignatures(scriptPubKey, txTo, 0, scriptSig, empty);
    EXPECT_FALSE(combined == scriptSig);
    combined = CombineSignatures(scriptPubKey, txTo, 0, empty, scriptSig);
    EXPECT_FALSE(combined == scriptSig);

    // A couple of partially-signed versions:
    std::vector<unsigned char> sig1;
    uint256 hash1 = SignatureHash(scriptPubKey, txTo, 0, SIGHASH_ALL);
    EXPECT_TRUE(keys[0].Sign(hash1, sig1));
    sig1.push_back(SIGHASH_ALL);
    std::vector<unsigned char> sig2;
    uint256 hash2 = SignatureHash(scriptPubKey, txTo, 0, SIGHASH_NONE);
    EXPECT_TRUE(keys[1].Sign(hash2, sig2));
    sig2.push_back(SIGHASH_NONE);
    std::vector<unsigned char> sig3;
    uint256 hash3 = SignatureHash(scriptPubKey, txTo, 0, SIGHASH_SINGLE);
    EXPECT_TRUE(keys[2].Sign(hash3, sig3));
    sig3.push_back(SIGHASH_SINGLE);

    // Not fussy about order (or even existence) of placeholders or signatures:
    CScript partial1a = CScript() << OP_0 << sig1 << OP_0;
    CScript partial1b = CScript() << OP_0 << OP_0 << sig1;
    CScript partial2a = CScript() << OP_0 << sig2;
    CScript partial2b = CScript() << sig2 << OP_0;
    CScript partial3a = CScript() << sig3;
    CScript partial3b = CScript() << OP_0 << OP_0 << sig3;
    CScript partial3c = CScript() << OP_0 << sig3 << OP_0;
    CScript complete12 = CScript() << OP_0 << sig1 << sig2;
    CScript complete13 = CScript() << OP_0 << sig1 << sig3;
    CScript complete23 = CScript() << OP_0 << sig2 << sig3;

    combined = CombineSignatures(scriptPubKey, txTo, 0, partial1a, partial1b);
    EXPECT_TRUE(combined == partial1a);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial1a, partial2a);
    EXPECT_TRUE(combined == complete12);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial2a, partial1a);
    EXPECT_TRUE(combined == complete12);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial1b, partial2b);
    EXPECT_TRUE(combined == complete12);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial3b, partial1b);
    EXPECT_TRUE(combined == complete13);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial2a, partial3a);
    EXPECT_TRUE(combined == complete23);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial3b, partial2b);
    EXPECT_TRUE(combined == complete23);
    combined = CombineSignatures(scriptPubKey, txTo, 0, partial3b, partial3a);
    EXPECT_TRUE(combined == partial3c);
#endif
}
#endif
#if 0
// Helpers:
static std::vector<unsigned char>
Serialize(const CScript& s)
{
    std::vector<unsigned char> sSerialized(s);
    return sSerialized;
}

static bool
Verify(const CScript& scriptSig, const CScript& scriptPubKey, bool fStrict)
{
    // Create dummy to/from transactions:
    CTransaction txFrom;
    txFrom.vout.resize(1);
    txFrom.vout[0].scriptPubKey = scriptPubKey;

    CTransaction txTo;
    txTo.vin.resize(1);
    txTo.vout.resize(1);
    txTo.vin[0].prevout.n = 0;
    txTo.vin[0].prevout.hash = txFrom.GetHash();
    txTo.vin[0].scriptSig = scriptSig;
    txTo.vout[0].nValue = 1;

    return VerifyScript(scriptSig, scriptPubKey, txTo, 0, fStrict ? SCRIPT_VERIFY_P2SH : SCRIPT_VERIFY_NONE, 0);
}

bool VerifySignature(const CCoins& txFrom, CTransaction* txTo, unsigned int nIn, unsigned int flags, int nHashType)
{
    return CScriptCheck(txFrom, txTo, nIn, flags, nHashType)();
}




TEST(scriptTest, pay2sh) {
    // Pay-to-script-hash looks like this:
    // scriptSig:    <sig> <sig...> <serialized_script>
    // scriptPubKey: HASH256 <hash> EQUAL

    // Test SignSignature() (and therefore the version of Solver() that signs transactions)
    CBasicKeyStore keystore;
	pwalletMain = new CWallet("wallet_test.dat");

    CKey key[4];
    for (int i = 0; i < 4; i++)
    {
        key[i].MakeNewKey();
        keystore.AddKey(key[i]);
    }

    // 8 Scripts: checking all combinations of
    // different keys, straight/P2SH, pubkey/pubkeyhash
    CScript standardScripts[4];
    standardScripts[0] << key[0].GetPubKey() << OP_CHECKSIG;
    standardScripts[1].SetDestination(key[1].GetPubKey().GetID());
    standardScripts[2] << key[1].GetPubKey() << OP_CHECKSIG;
    standardScripts[3].SetDestination(key[2].GetPubKey().GetID());
    CScript evalScripts[4];
    for (int i = 0; i < 4; i++)
    {
        keystore.AddCScript(standardScripts[i]);
        evalScripts[i].SetDestination(standardScripts[i].GetID());
    }

    CTransaction txFrom;  // Funding transaction:
    txFrom.vout.resize(8);
    for (int i = 0; i < 4; i++)
    {
        txFrom.vout[i].scriptPubKey = evalScripts[i];
        txFrom.vout[i].nValue = COIN;
        txFrom.vout[i+4].scriptPubKey = standardScripts[i];
        txFrom.vout[i+4].nValue = COIN;
    }

   EXPECT_TRUE(txFrom.IsStandard());

    CTransaction txTo[8]; // Spending transactions
    for (int i = 0; i < 8; i++)
    {
        txTo[i].vin.resize(1);
        txTo[i].vout.resize(1);
        txTo[i].vin[0].prevout.n = i;
        txTo[i].vin[0].prevout.hash = txFrom.GetHash();
        txTo[i].vout[0].nValue = 1;
        EXPECT_TRUE(IsMine(keystore, txFrom.vout[i].scriptPubKey));
    }
    for (int i = 0; i < 8; i++)
    {
        EXPECT_TRUE(SignSignature(keystore, txFrom, txTo[i], 0));
    }
    // All of the above should be OK, and the txTos have valid signatures
    // Check to make sure signature verification fails if we use the wrong ScriptSig:
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        {
            CScript sigSave = txTo[i].vin[0].scriptSig;
            txTo[i].vin[0].scriptSig = txTo[j].vin[0].scriptSig;
            bool sigOK = VerifySignature(CCoins(txFrom, 0), &txTo[i], 0, SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_STRICTENC, 0);
            if (i == j)
                EXPECT_TRUE(sigOK);
            else
                EXPECT_TRUE(!sigOK);
            txTo[i].vin[0].scriptSig = sigSave;
        }

}
#endif


