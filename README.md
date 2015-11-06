# access-blockchain-in-cpp

How to develop on top of monero? One way is to use josn-rpc calls from
any language capable of this, for example, as
[shown in python](http://moneroexamples.github.io/python-json-rpc/). Another way
is to use pubilc api of existing monero services such as
 [moneroblocks](http://moneroblocks.eu/api). These to approaches can be useful,
 but they have limitations in that they allow for only a fraction of what
 monero can do. The reason is that most functionality of monero is available
 through C++11 libraries. Those can be difficult to navigate and use.

For this reason this example was created. To show can one can use c++ monero
libraries to write something that uses the wealthy of monero libraries and tools.

# Objective
To check which transaction's outputs in a given block belong to a given
address. This is already possible using
 [XMR test](http://xmrtests.llcoins.net/checktx.html) website. And this is good,
 since it allows us to very results obtained this C++11 code with those
 provided by XMR test.


# Pre-requsits

Everthing here was done and tested on
Ubuntu 14.04 x86_64 and Ubuntu 15.10 x86_64.

For monero node is using `lmdbq`, thus I use this database only in this example.
I could not test on `berkeleydb`, thus its not included here.

## Dependencies


```bash
# refresh ubuntu's repository
sudo apt-get update

#install git
sudo apt-get install git

# install dependencies
sudo apt-get install build-essential cmake libboost1.55-all-dev miniupnpc libunbound-dev graphviz doxygen libdb5.1++-dev
```

## Monero compilation


```bash
# download the latest bitmonero source code from github
git clone https://github.com/monero-project/bitmonero.git

# go into bitmonero folder
cd bitmonero/

make # or make -j number_of_threads, e.g., make -j 2
```

## Monero static libraries
When the compilation finishes, bunch of static monero libraries
should be generated. We will need to link against.

They will be spread around different subfolders of the `./build/` folder.
So it is easier to just copy them into one folder. I assume that
 `/opt/bitmonero-dev/libs` will be our folder where we are going to keep them.

```bash
# create the folder
sudo mkdir -p /opt/bitmonero-dev/libs

# find the static libraries files (i.e., those with extension of *.a)
# and copy them to /opt/bitmonero-dev/libs
# assuming you are still in bitmonero/ folder which got downloaded from
# github
sudo find ./build/ -name '*.a' -exec cp {} /opt/bitmonero-dev/libs  \;
 ```

## Monero headers

Now we need to get headers, as this is our interface to the
monero libraries.

```bash
# create the folder
sudo mkdir -p /opt/bitmonero-dev/headers

# find the header files (i.e., those with extension of *.h)
# and copy them to /opt/bitmonero-dev/headers
# but this time the structure of directions is important
# so rsync is used to find and copy the headers files
sudo rsync -zarv --include="*/" --include="*.h" --exclude="*" --prune-empty-dirs ./ /opt/bitmonero-dev/headers
 ```

## cmake confing files
`CMakeLists.txt` files and the structure of this project are can be studied at github. I wont be discussing them
here. I tried to put comments in `CMakeLists.txt` to clarify what is there.

The location of the Monero's headers and static libraries must be correctly
indicated in  `CMakeLists.txt`. So if you put them in different folder
that in this example, please change the root `CMakeLists.txt` file
to reflect this.

# C++11 code
The two most interesting C++11 source files in this example are `MicroCore.cpp` and `main.cpp`. Therefore, I will focus on them here.
Full source code is on github.

## MicroCore.cpp

`MicroCore` class is a micro version of [cryptonode::core](https://github.com/monero-project/bitmonero/blob/master/src/cryptonote_core/cryptonote_core.h) class. The core class is the main
class with the access to the blockchain that the monero daemon is using. In the `cryptonode::core` class, the most important method (at least for this example), is the [init](https://github.com/monero-project/bitmonero/blob/master/src/cryptonote_core/cryptonote_core.cpp#L206) method. The main goal of the `init` function
is to create instance of [Blockchain](https://github.com/monero-project/bitmonero/blob/master/src/cryptonote_core/blockchain.h) class. The `Blockchain` is the high level interface to blockchain. The low level one is through `BlockchainLMDB` in our case, which
can also be accessed through `Blockchain` object.

The original class does a lot of things, which we dont need here, such as reading program options, checking
file

```c++
#include "MicroCore.h"

namespace xmreg
{
    /**
     * The constructor is interesting, as
     * m_mempool and m_blockchain_storage depend
     * on each other.
     *
     * So basically m_mempool initialized with
     * reference to Blockchain (i.e., Blockchain&)
     * and m_blockchain_storage is initialized with
     * reference to m_mempool (i.e., tx_memory_pool&)
     *
     * The same is done in cryptonode::core.
     */
    MicroCore::MicroCore():
            m_mempool(m_blockchain_storage),
            m_blockchain_storage(m_mempool)
    {}


    /**
     * Initialized the MicroCore object.
     *
     * Create BlockchainLMDB on the heap.
     * Open database files located in blockchain_path.
     * Initialize m_blockchain_storage with the BlockchainLMDB object.
     */
    bool
    MicroCore::init(const string& blockchain_path)
    {
        int db_flags = 0;

        // MDB_RDONLY will result in
        // m_blockchain_storage.deinit() producing
        // error messages.

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

    /**
    * Get m_blockchain_storage.
    * Initialize m_blockchain_storage with the BlockchainLMDB object.
    */
    Blockchain&
    MicroCore::get_core()
    {
        return m_blockchain_storage;
    }


    /**
     * De-initialized Blockchain.
     *
     * Its needed to mainly deallocate
     * new BlockchainDB object
     * created in the MicroCore::init().
     *
     * It also tries to synchronize the blockchain.
     * And this is the reason when, if MDB_RDONLY
     * is set, we are getting error messages. Because
     * blockchain is readonly and we try to synchronize it.
     */
    MicroCore::~MicroCore()
    {
        m_blockchain_storage.deinit();
    }

}
```

## main.cpp

```c++

#include "src/MicroCore.h"
#include "src/tools.h"


using namespace std;

unsigned int epee::g_test_dbg_lock_sleep = 0;

int main() {

    // enable basic monero log output
    uint32_t log_level = 0;
    epee::log_space::get_set_log_detalisation_level(true, log_level);
    epee::log_space::log_singletone::add_logger(LOGGER_CONSOLE, NULL, NULL); //LOGGER_NULL

    // location of the lmdb blockchain
    string blockchain_path {"/home/mwo/.bitmonero/lmdb"};

    // input data: public address, private view key and tx hash
    // they are hardcoded here, as I dont want to unnecessary
    // bloat the code with parsing input arguments
    string address_str {"48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU"};
    string viewkey_str {"1ddabaa51cea5f6d9068728dc08c7ffaefe39a7a4b5f39fa8a976ecbe2cb520a"};
    string tx_hash_str {"66040ad29f0d780b4d47641a67f410c28cce575b5324c43b784bb376f4e30577"};


    // our micro cryptonote core
    xmreg::MicroCore mcore;

    if (!mcore.init(blockchain_path))
    {
        cerr << "Error accessing blockchain." << endl;
        return 1;
    }

    // get the high level cryptonote::Blockchain object to interact
    // with the blockchain lmdb database
    cryptonote::Blockchain& core_storage = mcore.get_core();

    // get the current blockchain height. Just to check
    // if it reads ok.
    uint64_t height = core_storage.get_current_blockchain_height();

    cout << "Current blockchain height: " << height << endl;



    // parse string representing of monero address
    cryptonote::account_public_address address;

    if (!xmreg::parse_str_address(address_str,  address))
    {
        cerr << "Cant parse string address: " << address_str << endl;
        return 1;
    }


    // parse string representing of our private viewkey
    crypto::secret_key prv_view_key;
    if (!xmreg::parse_str_secret_key(viewkey_str, prv_view_key))
    {
        cerr << "Cant parse view key: " << viewkey_str << endl;
        return 1;
    }


    // we also need tx public key, rather than tx hash.
    // to get it first, we obtained transaction object tx
    // and then we get its public key from tx's extras.
    cryptonote::transaction tx;

    if (!xmreg::get_tx_pub_key_from_str_hash(core_storage, tx_hash_str, tx))
    {
        cerr << "Cant find transaction with hash: " << tx_hash_str << endl;
        return 1;
    }


    crypto::public_key pub_tx_key = cryptonote::get_tx_pub_key_from_extra(tx);

    if (pub_tx_key == cryptonote::null_pkey)
    {
        cerr << "Cant get public key of tx with hash: " << tx_hash_str << endl;
        return 1;
    }


    // public transaction key is combined with our view key
    // to get so called, derived key.
    crypto::key_derivation derivation;

    if (!generate_key_derivation(pub_tx_key, prv_view_key, derivation))
    {
        cerr << "Cant get dervied key for: " << "\n"
             << "pub_tx_key: " << prv_view_key << " and "
             << "prv_view_key" << prv_view_key << endl;
        return 1;
    }


    // lets check our keys
    cout << "\n"
         << "address          : <" << xmreg::print_address(address) << ">\n"
         << "private view key : "  << prv_view_key << "\n"
         << "tx hash          : <" << tx_hash_str << ">\n"
         << "public tx key    : "  << pub_tx_key << "\n"
         << "dervied key      : "  << derivation << "\n" << endl;


    // each tx that we (or the adddress we are checking) received
    // contains a number of outputs.
    // some of them are ours, some not. so we need to go through
    // all of them in a given tx block, to check with outputs are ours.

    // get the total number of outputs in a transaction.
    size_t output_no = tx.vout.size();

    // sum amount of xmr sent to us
    // in the given transaction
    uint64_t money_transfered {0};

    // loop through outputs in the given tx
    // to check which outputs our ours, we compare outputs
    // public keys, with the public key that would had been
    // generated for us.
    for (size_t i = 0; i < output_no; ++i)
    {
        // get the tx output public key
        // that normally would be generated for us,
        // if someone send us some xrm
        crypto::public_key pubkey;

        crypto::derive_public_key(derivation,
                                  i,
                                  address.m_spend_public_key,
                                  pubkey);


        // get tx output public key
        const cryptonote::txout_to_key tx_out_to_key
                = boost::get<cryptonote::txout_to_key>(tx.vout[i].target);


        cout << "Output no: " << i << ", " << tx_out_to_key.key;

        // check if the output's public key is ours
        if (tx_out_to_key.key == pubkey)
        {
            // if so, than add the xmr amount to the money_transfered
            money_transfered += tx.vout[i].amount;
            cout << ", mine key: " << cryptonote::print_money(tx.vout[i].amount) << endl;
        }
        else
        {
            cout << ", not mine key " << endl;
        }

    }

    cout << "\nTotal xmr received: " << cryptonote::print_money(money_transfered) << endl;


    cout << "\nEnd of program." << endl;

    return 0;
}
```

# Output
The main output is as follows:
```bash
address          : <48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU>
private view key : <1ddabaa51cea5f6d9068728dc08c7ffaefe39a7a4b5f39fa8a976ecbe2cb520a>
tx hash          : <66040ad29f0d780b4d47641a67f410c28cce575b5324c43b784bb376f4e30577>
public tx key    : <0851f2ec7477b82618e028238164a9080325fe299dcf5f70f868729b50d00284>
dervied key      : <8017f9944635b7b2e4dc2ddb9b81787e49b384dcb2abd474355fe62bee79fdd7>

Output no: 0, <c65ee61d95480988c1fd70f6078afafd4d90ef730fc3c4df59951d64136e911f>, not mine key
Output no: 1, <67a5fd7e06640942f0d869e494fc9d297d5087609013cd3531d0da55de19045b>, not mine key
Output no: 2, <a9e0f19422e68ed328315e92373388a3ebb418204a36d639bd1f2e870f4bc919>, mine key: 0.800000000000
Output no: 3, <849b56538f199f0a7522fcd0b132e53eec4a822e9b70b0e7e6c9e2632f1328db>, mine key: 4.000000000000
Output no: 4, <aba2e362f8ae0d79a4f33f9e4e27eecf79ad9c53eae86c27aa0281fb29aa6fdc>, not mine key
Output no: 5, <2602e4ac211216571ab1afe631aae1f905f252a1150cb8c4e5f34b820d0d6b4a>, not mine key

Total xmr received: 4.800000000000
```
These results agree with those obtained using [XMR test](http://xmrtests.llcoins.net/checktx.html).

## How can you help?

Constructive criticism, code and website edits are always good. They can be made through github.

Some Monero are also welcome:
```
48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU
```
