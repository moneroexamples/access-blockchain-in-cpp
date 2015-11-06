//
// Created by marcin on 5/11/15.
//

#ifndef XMREG01_TOOLS_H
#define XMREG01_TOOLS_H

#include <string>

#include "monero_headers.h"

namespace xmreg
{
    using namespace cryptonote;
    using namespace std;

    template <typename T>
    bool
    parse_str_secret_key(const string& key_str, T& secret_key);



//
//    template <typename T>
//    bool
//    parse_str_secret_key(const string& key_str, T& secret_key)
//    {
//
//        crypto::hash hash_;
//
//        if(!parse_hash256(key_str, hash_))
//        {
//            cerr << "Cant parse a key (e.g. viewkey): " << key_str << endl;
//            return false;
//        }
//
//        copy(begin(hash_.data), end(hash_.data), secret_key.data);
//
//        return true;
//    }


    bool
    get_tx_pub_key_from_str_hash(Blockchain& core_storage,
                             const string& hash_str,
                             transaction& tx);

    bool
    parse_str_address(const string& address_str,
                      account_public_address& address);


    string
    print_address(const account_public_address& address)
    {
        return get_account_address_as_str(false, address);
    }

}

#endif //XMREG01_TOOLS_H
