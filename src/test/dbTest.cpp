#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include "db.h"
#include "util.h"
#include "main.h"
#include "serialize.h"

#include <map>
#include <string>
#include <vector>

#include <db_cxx.h>

CDBEnv testdb;

boost::filesystem::path GetDefaultTestDataDir()
{
    boost::filesystem::path testFile = boost::filesystem::current_path()/"test"/"testdata";
	//boost::filesystem::create_directories(testFile);
    return testFile;
}

bool OpenTestDB() {
    if (!testdb.Open(GetDefaultTestDataDir())) {
       std::cout<<"open test db error!"<<std::endl;
	   return false;
    }
	return true;
}

void CloseTestDB() {
    testdb.Close();
}


class CTDB
{
protected:
    Db* pdb;
    std::string strFile;
    DbTxn *activeTxn;
    bool fReadOnly;

    explicit CTDB(const char* pszFile, const char* pszMode="r+")
	    : pdb(NULL)
		, activeTxn(NULL) {
		int ret;
		if (pszFile == NULL)
			return;
	
		fReadOnly = (!strchr(pszMode, '+') && !strchr(pszMode, 'w'));
		bool fCreate = strchr(pszMode, 'c');
		unsigned int nFlags = DB_THREAD;
		if (fCreate)
			nFlags |= DB_CREATE;
	
		{
			LOCK(testdb.cs_db);
			if (!testdb.Open(GetDefaultTestDataDir()))
				throw std::runtime_error("env open failed");
	
			strFile = pszFile;
			++testdb.mapFileUseCount[strFile];
			pdb = testdb.mapDb[strFile];
			if (pdb == NULL)
			{
				pdb = new Db(&testdb.dbenv, 0);
				ret = pdb->open(NULL,	   // Txn pointer
								pszFile,	// Filename
							    "testmain", // Logical db name
								DB_BTREE,  // Database type
								nFlags,    // Flags
								0);
	
				if (ret != 0)
				{
					delete pdb;
					pdb = NULL;
					--testdb.mapFileUseCount[strFile];
					strFile = "";
					throw std::runtime_error(strprintf("CTDB() : can't open database file %s, error %d", pszFile, ret));
				}
	
				if (fCreate && !Exists(std::string("version")))
				{
					bool fTmp = fReadOnly;
					fReadOnly = false;
					WriteVersion(CLIENT_VERSION);
					fReadOnly = fTmp;
				}
	
				testdb.mapDb[strFile] = pdb;
			}
		}
	}

    ~CTDB() {  }
public:
    void Flush() {
		if (activeTxn)
			return;
		
		// Flush database activity from memory pool to disk log
		unsigned int nMinutes = 0;
		if (fReadOnly)
			nMinutes = 1;
		
		testdb.dbenv.txn_checkpoint(nMinutes ? GetArg("-dblogsize", 100)*1024 : 0, nMinutes, 0);
	}

    void Close()
	{
		if (!pdb)
			return;
		if (activeTxn)
			activeTxn->abort();
		activeTxn = NULL;
		pdb = NULL;
		
		Flush();
		
		{
			LOCK(testdb.cs_db);
			--testdb.mapFileUseCount[strFile];
		}
	}

private:
    CTDB(const CTDB&);
    void operator=(const CTDB&);

protected:
    template<typename K, typename T>
    bool Read(const K& key, T& value)
    {
        if (!pdb)
            return false;

        // Key
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(1000);
        ssKey << key;
        Dbt datKey(&ssKey[0], ssKey.size());

        // Read
        Dbt datValue;
        datValue.set_flags(DB_DBT_MALLOC);
        int ret = pdb->get(activeTxn, &datKey, &datValue, 0);
        memset(datKey.get_data(), 0, datKey.get_size());
        if (datValue.get_data() == NULL)
            return false;

        // Unserialize value
        try {
            CDataStream ssValue((char*)datValue.get_data(), (char*)datValue.get_data() + datValue.get_size(), SER_DISK, CLIENT_VERSION);
            ssValue >> value;
        }
        catch (std::exception &e) {
            return false;
        }

        // Clear and free memory
        memset(datValue.get_data(), 0, datValue.get_size());
        free(datValue.get_data());
        return (ret == 0);
    }

    template<typename K, typename T>
    bool Write(const K& key, const T& value, bool fOverwrite=true)
    {
        if (!pdb)
            return false;
        if (fReadOnly)
            assert(!"Write called on database in read-only mode");

        // Key
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(1000);
        ssKey << key;
        Dbt datKey(&ssKey[0], ssKey.size());

        // Value
        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        ssValue.reserve(10000);
        ssValue << value;
        Dbt datValue(&ssValue[0], ssValue.size());

        // Write
        int ret = pdb->put(activeTxn, &datKey, &datValue, (fOverwrite ? 0 : DB_NOOVERWRITE));

        // Clear memory in case it was a private key
        memset(datKey.get_data(), 0, datKey.get_size());
        memset(datValue.get_data(), 0, datValue.get_size());
        return (ret == 0);
    }

    template<typename K>
    bool Erase(const K& key)
    {
        if (!pdb)
            return false;
        if (fReadOnly)
            assert(!"Erase called on database in read-only mode");

        // Key
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(1000);
        ssKey << key;
        Dbt datKey(&ssKey[0], ssKey.size());

        // Erase
        int ret = pdb->del(activeTxn, &datKey, 0);

        // Clear memory
        memset(datKey.get_data(), 0, datKey.get_size());
        return (ret == 0 || ret == DB_NOTFOUND);
    }

    template<typename K>
    bool Exists(const K& key)
    {
        if (!pdb)
            return false;

        // Key
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(1000);
        ssKey << key;
        Dbt datKey(&ssKey[0], ssKey.size());

        // Exists
        int ret = pdb->exists(activeTxn, &datKey, 0);

        // Clear memory
        memset(datKey.get_data(), 0, datKey.get_size());
        return (ret == 0);
    }

    Dbc* GetCursor()
    {
        if (!pdb)
            return NULL;
        Dbc* pcursor = NULL;
        int ret = pdb->cursor(NULL, &pcursor, 0);
        if (ret != 0)
            return NULL;
        return pcursor;
    }

    int ReadAtCursor(Dbc* pcursor, CDataStream& ssKey, CDataStream& ssValue, unsigned int fFlags=DB_NEXT)
    {
        // Read at cursor
        Dbt datKey;
        if (fFlags == DB_SET || fFlags == DB_SET_RANGE || fFlags == DB_GET_BOTH || fFlags == DB_GET_BOTH_RANGE)
        {
            datKey.set_data(&ssKey[0]);
            datKey.set_size(ssKey.size());
        }
        Dbt datValue;
        if (fFlags == DB_GET_BOTH || fFlags == DB_GET_BOTH_RANGE)
        {
            datValue.set_data(&ssValue[0]);
            datValue.set_size(ssValue.size());
        }
        datKey.set_flags(DB_DBT_MALLOC);
        datValue.set_flags(DB_DBT_MALLOC);
        int ret = pcursor->get(&datKey, &datValue, fFlags);
        if (ret != 0)
            return ret;
        else if (datKey.get_data() == NULL || datValue.get_data() == NULL)
            return 99999;

        // Convert to streams
        ssKey.SetType(SER_DISK);
        ssKey.clear();
        ssKey.write((char*)datKey.get_data(), datKey.get_size());
        ssValue.SetType(SER_DISK);
        ssValue.clear();
        ssValue.write((char*)datValue.get_data(), datValue.get_size());

        // Clear and free memory
        memset(datKey.get_data(), 0, datKey.get_size());
        memset(datValue.get_data(), 0, datValue.get_size());
        free(datKey.get_data());
        free(datValue.get_data());
        return 0;
    }

public:
    bool TxnBegin()
    {
        if (!pdb || activeTxn)
            return false;
        DbTxn* ptxn = testdb.TxnBegin();
        if (!ptxn)
            return false;
        activeTxn = ptxn;
        return true;
    }

    bool TxnCommit()
    {
        if (!pdb || !activeTxn)
            return false;
        int ret = activeTxn->commit(0);
        activeTxn = NULL;
        return (ret == 0);
    }

    bool TxnAbort()
    {
        if (!pdb || !activeTxn)
            return false;
        int ret = activeTxn->abort();
        activeTxn = NULL;
        return (ret == 0);
    }

    bool ReadVersion(int& nVersion)
    {
        nVersion = 0;
        return Read(std::string("version"), nVersion);
    }

    bool WriteVersion(int nVersion)
    {
        return Write(std::string("version"), nVersion);
    }

    bool static Rewrite(const std::string& strFile, const char* pszSkip = NULL);
};



class CTestDB :public CTDB
{
public:
	
    CTestDB(std::string strFilename, const char* pszMode="wr+") 
		: CTDB(strFilename.c_str(), pszMode) 
    	{}
private:
    CTestDB(const CTestDB&);
    void operator=(const CTestDB&);
public:
    bool WriteName(int *a, int *b) {
		return Write(*a, *b);
	}
    bool ReadName(int *a, int *b) {
		return Read(*a, *b);
	}
    bool EraseName(int *a) {
		return Erase(*a);
	}
};

TEST(dbtest, opendb) {
	boost::filesystem::path stdbpath = boost::filesystem::current_path()/"test"/"testdata";
    boost::filesystem::create_directory(stdbpath);
	CTestDB ptdb("stdb.dat","cwr+");
	int a = 100, b = 200;
    EXPECT_TRUE(ptdb.WriteName(&a, &b));
	int c;
    EXPECT_TRUE(ptdb.ReadName(&a, &c));
	EXPECT_EQ(c,200);
	ptdb.Close();
    testdb.Flush(true);
	if (boost::filesystem::exists(stdbpath)) {
		if (boost::filesystem::is_empty(stdbpath))
	        boost::filesystem::remove(stdbpath);
		else
			boost::filesystem::remove_all(stdbpath);
	}

}
