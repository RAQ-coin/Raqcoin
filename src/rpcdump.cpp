// Copyright (c) 2012 The Bitcoin developers
// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers


#include "init.h" // for pwalletMain
#include "raqcoinrpc.h"
#include "ui_interface.h"
#include "base58.h"

#include <boost/algorithm/string.hpp>

#include <boost/lexical_cast.hpp>

#define printf OutputDebugStringF

using namespace json_spirit;
using namespace std;

class CTxDump
{
public:
    CBlockIndex *pindex;
    int64 nValue;
    bool fSpent;
    CWalletTx* ptx;
    int nOut;
    CTxDump(CWalletTx* ptx = NULL, int nOut = -1)
    {
        pindex = NULL;
        nValue = 0;
        fSpent = false;
        this->ptx = ptx;
        this->nOut = nOut;
    }
};

Value importkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "importkey <raqcoinkey> [label] [rescan=true]\n"
            "Adds a private key|public key|position (as returned by dumpkey) to your wallet, this would cost a few minutes");

    string strOneKey = params[0].get_str();

    std::vector<std::string> vArgs;
    boost::split(vArgs, strOneKey, boost::is_any_of("|"));
    if(vArgs.size() < 2)
    {
        throw std::runtime_error("invalid input!");
    }

    string strLabel = "";
    if (params.size() > 1)
        strLabel = params[1].get_str();

    // Whether to perform rescan after import
    bool fRescan = true;
    if (params.size() > 2)
        fRescan = params[2].get_bool();

    CRaqcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(vArgs[0]);

    if (!fGood) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key");

    CKey key;
    CSecret secret = vchSecret.GetSecret();
    key.SetPrivKey(secret);

    CPubKey pubKey;
    if(!DecodeBase58(vArgs[1], pubKey.vchPubKey))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid public key");

    key.SetPubKey(pubKey);
    if (!key.IsValid()) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key or public key");
    }

    CKeyID vchAddress = key.GetPubKey().GetID();
    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        pwalletMain->MarkDirty();

        if (!pwalletMain->AddKey(key))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key to wallet");

        if(vArgs.size() > 2) {
            CDiskPubKeyPos pos;
            pos<< ParseHex(vArgs[2]);
            if (!pwalletMain->AddPubKeyPos(CRaqcoinAddress(vchAddress).ToString(), pos))
                throw JSONRPCError(RPC_WALLET_ERROR, "Error adding public key position to wallet");
        }

        pwalletMain->SetAddressBookName(vchAddress, strLabel);

        if (fRescan) {
            pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
            pwalletMain->ReacceptWalletTransactions();
        }
    }

    return Value::null;
}

Value dumpkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "dumpkey <raqcoinaddress>\n"
            "Reveals the private key|public key|position corresponding to <raqcoinaddress>, this would cost a few minutes");

    string strAddress = params[0].get_str();
    CRaqcoinAddress address;
    if (!address.SetString(strAddress))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Raqcoin address");
    CKeyID keyID;
    if (!address.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
    CSecret vchSecret;
    if (!pwalletMain->GetSecret(keyID, vchSecret))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key for address " + strAddress + " is not known");

    CPubKey pubKey;
    if (!pwalletMain->GetPubKey(keyID, pubKey))
        throw JSONRPCError(RPC_WALLET_ERROR, "Address does not refer to a public key");

    CKey key;
    key.SetPrivKey(vchSecret);
    key.SetPubKey(pubKey);
    if (!key.IsValid()) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key or public key");
    }

    CDiskPubKeyPos pos;
    if (pwalletMain->GetPubKeyPos(strAddress, pos))
        return CRaqcoinSecret(vchSecret).ToString()+"|" + EncodeBase58(pubKey.vchPubKey) + "|" + HexStr(pos.ToVector());
    else
        return CRaqcoinSecret(vchSecret).ToString()+"|" + EncodeBase58(pubKey.vchPubKey);

}
