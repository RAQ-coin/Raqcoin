// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers

#ifndef RAQCOIN_INIT_H
#define RAQCOIN_INIT_H

#include "wallet.h"

extern CWallet* pwalletMain;

void StartShutdown();
bool ShutdownRequested();
void Shutdown();
bool AppInit2(boost::thread_group& threadGroup);
bool AppInit(int argc, char* argv[]);
std::string HelpMessage();

#endif
