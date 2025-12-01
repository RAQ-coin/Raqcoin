// Copyright (c) 2012 The Bitcoin developers
// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers

#ifndef RAQCOIN_LEVELDB_H
#define RAQCOIN_LEVELDB_H

#include "serialize.h"

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

#include <boost/filesystem/path.hpp>

class leveldb_error : public std::runtime_error
{
public:
    leveldb_error(const std::string &msg) : std::runtime_error(msg) {}
};

void HandleError(const leveldb::Status &status) throw(leveldb_error);

// Batch of changes queued to be written to a CLevelDB
class CLevelDBBatch
{
    friend class CLevelDB;

private:
    leveldb::WriteBatch batch;
	size_t sizeEstimate;

public:
	explicit CLevelDBBatch() :sizeEstimate(0) { }
	void Clear() {
        batch.Clear();
        sizeEstimate = 0;
    }
    template<typename K, typename V> void Write(const K& key, const V& value) {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        leveldb::Slice slKey(&ssKey[0], ssKey.size());

        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        ssValue.reserve(ssValue.GetSerializeSize(value));
        ssValue << value;
        leveldb::Slice slValue(&ssValue[0], ssValue.size());

        batch.Put(slKey, slValue);

	    // LevelDB serializes writes as:
	    // - byte: header
	    // - varint: key length (1 byte up to 127B, 2 bytes up to 16383B, ...)
	    // - byte[]: key
	    // - varint: value length
	    // - byte[]: value
	    // The formula below assumes the key and value are both less than 16k.
	    sizeEstimate += 3 + (slKey.size() > 127) + slKey.size() + (slValue.size() > 127) + slValue.size();
	    ssKey.clear();
	    ssValue.clear();
    }

    template<typename K> void Erase(const K& key) {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        leveldb::Slice slKey(&ssKey[0], ssKey.size());

        batch.Delete(slKey);

	    // LevelDB serializes erases as:
        // - byte: header
        // - varint: key length
        // - byte[]: key
        // The formula below assumes the key is less than 16kB.
        sizeEstimate += 2 + (slKey.size() > 127) + slKey.size();
        ssKey.clear();
    }

	size_t SizeEstimate() const { return sizeEstimate; }
};

class CLevelDB
{
private:
    // custom environment this database is using (may be NULL in case of default environment)
    leveldb::Env *penv;

    // database options used
    leveldb::Options options;

    // options used when reading from the database
    leveldb::ReadOptions readoptions;

    // options used when iterating over values of the database
    leveldb::ReadOptions iteroptions;

    // options used when writing to the database
    leveldb::WriteOptions writeoptions;

    // options used when sync writing to the database
    leveldb::WriteOptions syncoptions;

    // the database itself
    leveldb::DB *pdb;

public:
    CLevelDB(const boost::filesystem::path &path, size_t nCacheSize, bool fMemory = false, bool fWipe = false);
    ~CLevelDB();

    template<typename K, typename V> bool Read(const K& key, V& value) throw(leveldb_error) {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        leveldb::Slice slKey(&ssKey[0], ssKey.size());

        std::string strValue;
        leveldb::Status status = pdb->Get(readoptions, slKey, &strValue);
        if (!status.ok()) {
            if (status.IsNotFound())
                return false;
            printf("LevelDB read failure: %s\n", status.ToString().c_str());
            HandleError(status);
        }
        try {
            CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
            ssValue >> value;
        } catch(std::exception &e) {
            return false;
        }
        return true;
    }

    template<typename K, typename V> bool Write(const K& key, const V& value, bool fSync = false) throw(leveldb_error) {
        CLevelDBBatch batch;
        batch.Write(key, value);
        return WriteBatch(batch, fSync);
    }

    template<typename K> bool Exists(const K& key) throw(leveldb_error) {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        leveldb::Slice slKey(&ssKey[0], ssKey.size());

        std::string strValue;
        leveldb::Status status = pdb->Get(readoptions, slKey, &strValue);
        if (!status.ok()) {
            if (status.IsNotFound())
                return false;
            printf("LevelDB read failure: %s\n", status.ToString().c_str());
            HandleError(status);
        }
        return true;
    }

    template<typename K> bool Erase(const K& key, bool fSync = false) throw(leveldb_error) {
        CLevelDBBatch batch;
        batch.Erase(key);
        return WriteBatch(batch, fSync);
    }

    bool WriteBatch(CLevelDBBatch &batch, bool fSync = false) throw(leveldb_error);

    // not available for LevelDB; provide for compatibility with BDB
    bool Flush() {
        return true;
    }

    bool Sync() throw(leveldb_error) {
        CLevelDBBatch batch;
        return WriteBatch(batch, true);
    }

    // not exactly clean encapsulation, but it's easiest for now
    leveldb::Iterator *NewIterator() {
        return pdb->NewIterator(iteroptions);
    }
};

#endif // RAQCOIN_LEVELDB_H
