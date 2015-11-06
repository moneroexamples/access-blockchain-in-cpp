//
// Created by mwo on 5/11/15.
//

#include "MicroCore.h"

namespace xmreg
{

    MicroCore::MicroCore():
            m_mempool(m_blockchain_storage),
            m_blockchain_storage(m_mempool)
    {}



    bool
    MicroCore::init(const string& blockchain_path)
    {
        int db_flags = 0;

        //db_flags |= MDB_RDONLY ;
        db_flags |= MDB_NOSYNC;

        BlockchainDB* db = nullptr;
        db = new BlockchainLMDB();

        try
        {
            db->open(blockchain_path, db_flags);
        }
        catch (const std::exception& e)
        {
            cerr << "Error opening database: " << e.what();
            return false;
        }

        if(!db->is_open())
        {
            return false;
        }


        return m_blockchain_storage.init(db, false);
    }

    Blockchain&
    MicroCore::get_core()
    {
        return m_blockchain_storage;
    }


    MicroCore::~MicroCore()
    {
        // call "BlockchainDB" for BlockchainDB
        // its needed to dealocate  new BlockchainDB
        // created in the MicroCore::init().
        m_blockchain_storage.deinit();
    }

}