// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "validationinterface.h"

// For Boost 1.60+, need to explicitly use placeholders namespace
using namespace boost::placeholders;

static CMainSignals g_signals;

CMainSignals& GetMainSignals()
{
    return g_signals;
}

void RegisterValidationInterface(CValidationInterface* pwalletIn) {
    g_signals.UpdatedBlockTip.connect(boost::bind(&CValidationInterface::UpdatedBlockTip, pwalletIn, _1, _2, _3));
    g_signals.TransactionAddedToMempool.connect(boost::bind(&CValidationInterface::TransactionAddedToMempool, pwalletIn, _1));
    g_signals.BlockConnected.connect(boost::bind(&CValidationInterface::BlockConnected, pwalletIn, _1));
    g_signals.BlockDisconnected.connect(boost::bind(&CValidationInterface::BlockDisconnected, pwalletIn, _1));
}

void UnregisterValidationInterface(CValidationInterface* pwalletIn) {
    g_signals.BlockDisconnected.disconnect(boost::bind(&CValidationInterface::BlockDisconnected, pwalletIn, _1));
    g_signals.BlockConnected.disconnect(boost::bind(&CValidationInterface::BlockConnected, pwalletIn, _1));
    g_signals.TransactionAddedToMempool.disconnect(boost::bind(&CValidationInterface::TransactionAddedToMempool, pwalletIn, _1));
    g_signals.UpdatedBlockTip.disconnect(boost::bind(&CValidationInterface::UpdatedBlockTip, pwalletIn, _1, _2, _3));
}

void UnregisterAllValidationInterfaces() {
    g_signals.BlockDisconnected.disconnect_all_slots();
    g_signals.BlockConnected.disconnect_all_slots();
    g_signals.TransactionAddedToMempool.disconnect_all_slots();
    g_signals.UpdatedBlockTip.disconnect_all_slots();
}