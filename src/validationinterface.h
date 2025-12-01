// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_VALIDATIONINTERFACE_H
#define BITCOIN_VALIDATIONINTERFACE_H

#include <boost/signals2/signal.hpp>
#include <boost/shared_ptr.hpp>

class CBlock;
class CBlockIndex;
class CTransaction;
class CValidationInterface;

// These functions dispatch to one or all registered wallets

/** Register a wallet to receive updates from core */
void RegisterValidationInterface(CValidationInterface* pwalletIn);
/** Unregister a wallet from core */
void UnregisterValidationInterface(CValidationInterface* pwalletIn);
/** Unregister all wallets from core */
void UnregisterAllValidationInterfaces();

class CValidationInterface {
protected:
    virtual void UpdatedBlockTip(const CBlockIndex *pindexNew, const CBlockIndex *pindexFork, bool fInitialDownload) {}
    virtual void TransactionAddedToMempool(const CTransaction &tx) {}
    virtual void BlockConnected(const CBlock *block) {}
    virtual void BlockDisconnected(const CBlock *block) {}

    friend void ::RegisterValidationInterface(CValidationInterface*);
    friend void ::UnregisterValidationInterface(CValidationInterface*);
    friend void ::UnregisterAllValidationInterfaces();
};

struct CMainSignals {
    boost::signals2::signal<void (const CBlockIndex *, const CBlockIndex *, bool)> UpdatedBlockTip;
    boost::signals2::signal<void (const CTransaction &)> TransactionAddedToMempool;
    boost::signals2::signal<void (const CBlock *)> BlockConnected;
    boost::signals2::signal<void (const CBlock *)> BlockDisconnected;
};

CMainSignals& GetMainSignals();

#endif // BITCOIN_VALIDATIONINTERFACE_H
