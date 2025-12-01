#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>

#include "db.h"
#include "txdb.h"
#include "main.h"
#include "wallet.h"
#include "util.h"
#include "noui.h"
CWallet* g_walletMain;
CClientUIInterface g_uiInterface;

extern bool g_PrintToConsole;
//extern void noui_connect();

struct TestingSetup {
    CCoinsViewDB *pcoinsdbview;
    boost::filesystem::path pathTemp;
    boost::thread_group threadGroup;

    TestingSetup() {
        fPrintToDebugger = true; // don't want to write to debug.log file
        noui_connect();
        bitdb.MakeMock();
        pathTemp = GetTempPath() / strprintf("test_ Raqcoin_%lu_%i", (unsigned long)GetTime(), (int)(GetRand(100000)));
        boost::filesystem::create_directories(pathTemp);
        mapArgs["-datadir"] = pathTemp.string();
        pblocktree = new CBlockTreeDB(1 << 20, true);
        pcoinsdbview = new CCoinsViewDB(1 << 23, true);
        pcoinsTip = new CCoinsViewCache(*pcoinsdbview);
        InitBlockIndex();
        bool fFirstRun;
        g_walletMain = new CWallet("wallet.dat");
        g_walletMain->LoadWallet(fFirstRun);
        RegisterWallet(g_walletMain);
        nScriptCheckThreads = 3;
        for (int i=0; i < nScriptCheckThreads-1; i++)
            threadGroup.create_thread(&ThreadScriptCheck);
    }
    ~TestingSetup()
    {
        threadGroup.interrupt_all();
        threadGroup.join_all();
        delete g_walletMain;
        g_walletMain = NULL;
        delete pcoinsTip;
        delete pcoinsdbview;
        delete pblocktree;
        bitdb.Flush(true);
        boost::filesystem::remove_all(pathTemp);
    }
};

TEST(raqcoinTest, raqcoin) {
	TestingSetup startRaqcoin;
}

