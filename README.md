# Access blockchain information using C++

How to develop on top of [Monero](https://getmonero.org/)? One way is to use json-rpc calls from
any language capable of this, for example, as
[shown in python](http://moneroexamples.github.io/python-json-rpc/). Another way
is to use pubilc api of existing Monero services such as
 [moneroblocks](http://moneroblocks.eu/api). Some Monero functions are even
 available in JavaScript if you look at the source code of [mymonero.com](https://mymonero.com/#/).
 This  allows to develop some web applications with only HTML and JavaScript, such as,
[XMR test](http://xmrtests.llcoins.net/checktx.html).

The other way, presented here, is to directly tap into Monero C++ libraries.
However, there are no tutorials or any information how to do it.

For this reason this example was created, i.e.,  to show how to access Monero C++ libraries
and do something with them.

Note: The tool was adapted to work with current monero development version (0.10.1), but was not adapted 
to work with ringct txs. For handing ringct txs, please check newer examples, for example
[onion monero blockchain explorer](https://github.com/moneroexamples/onion-monero-blockchain-explorer)

# Aim: check which transaction's outputs belong to a given address

There's been a lot of talk in Monero about viewkeys. But how do you actually use them?
Well, they can be used to to check which transaction's outputs in a given block belong to a given
address. Without the private viewkey associated with the given Monero address, it is not
possible to check how much Monero there are in that address. The same, if we know that
a given transaction was sent to a specific address, it is not possible to check
in a blockchain which outputs of that transaction actually were meant to belong to that address without
the private viewkey of that address. The viewkey allows to filter out outputs not bind to the address.

Checking if any of a transaction's outputs belong to a given address
with the private viewkey of that address is already possible using
 [XMR test](http://xmrtests.llcoins.net/checktx.html) website. This is very good,
 since it allows us to verify the results obtained using this example with those
 provided by that website.


# Prerequisites

The example uses Monero C++ libraries and headers. 
 Instructions how to download source files and compile Monero,
 setup header and library files are presented here:

- https://github.com/moneroexamples/compile-monero-09-on-ubuntu-16-04 (Ubuntu 16.04)
- https://github.com/moneroexamples/compile-monero-09-on-arch-linux (Arch Linux)


# C++ code

The two most interesting C++ files in this example are `MicroCore.cpp` and `main.cpp`.
Therefore, I will present only these to files here. Full source code is
at [github](https://github.com/moneroexamples/access-blockchain-in-cpp). The source code can
also slightly vary with the code here, as it can be updated more frequently than 
the code presented here. So for the latest version
of this example, please check the github repository directly.


## main.cpp
This is the main file of the example. For the program to work, four
input values are required:

 - `address` - Monero address.
 - `viewkey` - private view key associated with the address provided.
 - `txhash`  - transaction id (i.e., hash) which outputs we want to check.
 - `bc-path` - a path to lmdb folder with the blockchain.

To run the program, at least correct `bc-path` is required. All other
options have default values which work.

```c++
int main(int ac, const char* av[]) {

    // get command line options
    xmreg::CmdLineOptions opts {ac, av};

    auto help_opt = opts.get_option<bool>("help");

    // if help was chosen, display help text and finish
    if (*help_opt)
    {
        return 0;
    }

    // get other options
    auto address_opt = opts.get_option<string>("address");
    auto viewkey_opt = opts.get_option<string>("viewkey");
    auto tx_hash_opt = opts.get_option<string>("txhash");
    auto bc_path_opt = opts.get_option<string>("bc-path");


    // default path to monero folder
    // on linux this is /home/<username>/.bitmonero
    string default_monero_dir = tools::get_default_data_dir();

    // the default folder of the lmdb blockchain database
    // is therefore as follows
    string default_lmdb_dir   = default_monero_dir + "/lmdb";

    // get the program command line options, or
    // some default values for quick check
    string address_str = address_opt ? *address_opt : "48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU";
    string viewkey_str = viewkey_opt ? *viewkey_opt : "1ddabaa51cea5f6d9068728dc08c7ffaefe39a7a4b5f39fa8a976ecbe2cb520a";
    string tx_hash_str = tx_hash_opt ? *tx_hash_opt : "66040ad29f0d780b4d47641a67f410c28cce575b5324c43b784bb376f4e30577";
    path blockchain_path = bc_path_opt ? path(*bc_path_opt) : path(default_lmdb_dir);


    if (!is_directory(blockchain_path))
    {
        cerr << "Given path \"" << blockchain_path   << "\" "
             << "is not a folder or does not exist" << " "
             << endl;
        return 1;
    }

    blockchain_path = xmreg::remove_trailing_path_separator(blockchain_path);

    cout << "Blockchain path: " << blockchain_path << endl;

    // set  monero log output level
    uint32_t log_level = 0;
    mlog_configure("", true);


    // create instance of our MicroCore
    // and make pointer to the Blockchain
    xmreg::MicroCore mcore;
    cryptonote::Blockchain* core_storage;

    // initialize mcore and core_storage
    if (!xmreg::init_blockchain(blockchain_path.string(),
                                mcore, core_storage))
    {
        cerr << "Error accessing blockchain." << endl;
        return EXIT_FAILURE;
    }

    // get the current blockchain height. Just to check
    // if it reads ok.
    uint64_t height = core_storage->get_current_blockchain_height();

    cout << "Current blockchain height: " << height << endl;


    // parse string representing given monero address
    cryptonote::account_public_address address;

    if (!xmreg::parse_str_address(address_str,  address))
    {
        cerr << "Cant parse string address: " << address_str << endl;
        return 1;
    }


    // parse string representing given private viewkey
    crypto::secret_key prv_view_key;
    if (!xmreg::parse_str_secret_key(viewkey_str, prv_view_key))
    {
        cerr << "Cant parse view key: " << viewkey_str << endl;
        return 1;
    }


    // we also need tx public key, but we have tx hash only.
    // to get the key, first, we obtained transaction object tx
    // and then we get its public key from tx's extras.
    // this is done using get_tx_pub_key_from_str_hash function
    cryptonote::transaction tx;

    if (!xmreg::get_tx_pub_key_from_str_hash(*core_storage, tx_hash_str, tx))
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


    // public transaction key is combined with our viewkey
    // to create, so called, derived key.
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


    // each tx that we (or the address we are checking) received
    // contains a number of outputs.
    // some of them are ours, some not. so we need to go through
    // all of them in a given tx block, to check which outputs are ours.

    // get the total number of outputs in a transaction.
    size_t output_no = tx.vout.size();

    // sum amount of xmr sent to us
    // in the given transaction
    uint64_t money_transfered {0};

    // loop through outputs in the given tx
    // to check which outputs our ours. we compare outputs'
    // public keys with the public key that would had been
    // generated for us if we had gotten the outputs.
    // not sure this is the case though, but that's my understanding.
    for (size_t i = 0; i < output_no; ++i)
    {
        // get the tx output public key
        // that normally would be generated for us,
        // if someone had sent us some xmr.
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
            // if so, then add the xmr amount to the money_transfered
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

# Output example 1
Executing the program as follows:

```bash
./xmreg01 --address 48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU --viewkey 1ddabaa51cea5f6d9068728dc08c7ffaefe39a7a4b5f39fa8a976ecbe2cb520a --txhash 66040ad29f0d780b4d47641a67f410c28cce575b5324c43b784bb376f4e30577
```

Results in the following output:

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

# Output example 2

Executing the program as follows:

```bash
./xmreg01 --address 41vEA7Ye8Bpeda6g59v5t46koWrVn2PNgEKgzquJjmiKCFTsh9gajr8J3pad49rqu581TAtFGCH9CYTCkYrCpuWUG9GkgeB --viewkey fed77158ec692fe9eb951f6aeb22c3bda16fe8926c1aac13a5651a9c27f34309 --txhash ba807a90792f9202638e7288eff05949ccffbc54fd6a108571b65b963fee573a
```
Results in the following output:

```bash
address          : <41vEA7Ye8Bpeda6g59v5t46koWrVn2PNgEKgzquJjmiKCFTsh9gajr8J3pad49rqu581TAtFGCH9CYTCkYrCpuWUG9GkgeB>
private view key : <fed77158ec692fe9eb951f6aeb22c3bda16fe8926c1aac13a5651a9c27f34309>
tx hash          : <ba807a90792f9202638e7288eff05949ccffbc54fd6a108571b65b963fee573a>
public tx key    : <70dd2b3a54dc153ab367d7bab9287db1e153053e02a5a273084da00344e11e59>
dervied key      : <dad78e5ec247b91101cad60b98c09866cbcc46a3039c49a0de13be6e0645d8da>

Output no: 0, <460de8a6a3afea230c1a68db55054a67d2b33d80482cc5df645fed631d3b9c6e>, not mine key
Output no: 1, <54681e67463b5baf79a145ec90969ac0c1d358595e5a32c1e37e085290ee0dfe>, mine key: 0.080000000000
Output no: 2, <eb02627448c4cb2f21b3277f2a4359984cd89d0636a327ca068fe76f8d970bad>, mine key: 0.100000000000
Output no: 3, <584df4be03187e5545c6255cd61fee33afbdfa0b70fba2d9678cd43fd23f4df7>, not mine key
Output no: 4, <673f519e5a7e874485a5b239c54fc289941c7c15ad200f02d46ad98adfbd8049>, mine key: 9.000000000000
Output no: 5, <e1956d1077a9e5310c22a6a103e32f25def9aab4a7214894ed30a76d18cde271>, not mine key

Total xmr received: 9.180000000000
```

These results agree also with those obtained using [XMR test](http://xmrtests.llcoins.net/checktx.html).

#Output example 3
Executing the program as follows:
```bash
./xmreg01 --address 48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU --viewkey 1ddabaa51cea5f6d9068728dc08c7ffaefe39a7a4b5f39fa8a976ecbe2cb520a --txhash b82fe6d1f40e71a89a0c6d517ee1e84e4403870cba4589d0e6ca341ef966e143

```
Results in the following output:

```bash
address          : <48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU>
private view key : <1ddabaa51cea5f6d9068728dc08c7ffaefe39a7a4b5f39fa8a976ecbe2cb520a>
tx hash          : <b82fe6d1f40e71a89a0c6d517ee1e84e4403870cba4589d0e6ca341ef966e143>
public tx key    : <99b7c2876e2058718c496f36656e459352d055101cb87827f60e75d60f632727>
dervied key      : <cdac177a666eba391d5edc4f835e1e0d3e8f8735e322a0476b91e4d6e3a3a7f2>

Output no: 0, <2f1cf7db49a058592b58bdbb5a2121fb067f0be4fcd167936a242ad76fd7b32c>, mine key: 0.020000000000
Output no: 1, <0c37db67fd9cab5c3ce939df0a64905278ceb4520dde102c87f0bf66b1465aaa>, mine key: 0.100000000000
Output no: 2, <dbf1aac53547cc8ad8b288bdd83ae642aec2216fc647980e64e0a776a42b4267>, mine key: 9.000000000000

Total xmr received: 9.120000000000
```

These results also agree with those obtained using [XMR test](http://xmrtests.llcoins.net/checktx.html).

#Output example 4
Executing the program as follows:

```bash
./xmreg01 --address 41vEA7Ye8Bpeda6g59v5t46koWrVn2PNgEKgzquJjmiKCFTsh9gajr8J3pad49rqu581TAtFGCH9CYTCkYrCpuWUG9GkgeB --viewkey fed77158ec692fe9eb951f6aeb22c3bda16fe8926c1aac13a5651a9c27f34309 --txhash 60b6c42f1a3bea6ecf2adb8a4f98753be5a0a3e032a98beb7bdc44a325cea7e6
```

Results in the following output:

```bash
address          : <41vEA7Ye8Bpeda6g59v5t46koWrVn2PNgEKgzquJjmiKCFTsh9gajr8J3pad49rqu581TAtFGCH9CYTCkYrCpuWUG9GkgeB>
private view key : <fed77158ec692fe9eb951f6aeb22c3bda16fe8926c1aac13a5651a9c27f34309>
tx hash          : <60b6c42f1a3bea6ecf2adb8a4f98753be5a0a3e032a98beb7bdc44a325cea7e6>
public tx key    : <c6e755802545f5444f095ab31be3c2aaca4500d0b016d34cc0953b37cb198a1d>
dervied key      : <a6e57d6eed1c17cdb1da1b43ee0a4f49c2bfd2138d8da237f354f324712a67ba>

Output no: 0, <d58986ca9258cd4e4f7b0ef9baf3f61a49869364f5a9ec4a3d9186382a6b5110>, mine key: 0.020000000000
Output no: 1, <50fc47f931dfa393e369ab84a967c82dd866fd02205d656866dde9c2650329ca>, not mine key
Output no: 2, <55ea815f82a1552af2cf003b70f023bcde201ce3ad1ae8453ce9291b788d1cac>, mine key: 0.100000000000
Output no: 3, <e0d26e4d31022197d0831d3ba08b6b43e57672ccb9229880b29404468fd5984a>, not mine key
Output no: 4, <1ba3063bd57752537ca7ccd47ff2c0a72b7dcd8d8510fc222737636f208174cc>, mine key: 9.000000000000

Total xmr received: 9.120000000000
```
These results agree also with those obtained using [XMR test](http://xmrtests.llcoins.net/checktx.html).

## Compile this example
The dependencies are same as those for Monero, so I assume Monero compiles
correctly. If so then to download and compile this example, the following
steps can be executed:

```bash
# download the source code
git clone https://github.com/moneroexamples/access-blockchain-in-cpp.git

# enter the downloaded sourced code folder
cd access-blockchain-in-cpp

# create the makefile
cmake .

# compile
make
```

After this, `xmreg01` executable file should be present in access-blockchain-in-cpp
folder. How to use it, can be seen in the above example outputs.


## How can you help?

Constructive criticism, code and website edits are always good. They can be made through github.
