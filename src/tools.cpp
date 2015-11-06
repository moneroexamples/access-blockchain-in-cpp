//
// Created by marcin on 5/11/15.
//

#include "tools.h"

namespace xmreg
{


    template <typename T>
    bool
    parse_str_secret_key(const string& key_str, T& secret_key)
    {

        crypto::hash hash_;

        if(!parse_hash256(key_str, hash_))
        {
            cerr << "Cant parse a key (e.g. viewkey): " << key_str << endl;
            return false;
        }

        copy(begin(hash_.data), end(hash_.data), secret_key.data);

        return true;
    }

    // explicit instantiations of get template function
    template bool parse_str_secret_key<crypto::secret_key>(const string& key_str, crypto::secret_key& secret_key);
    template bool parse_str_secret_key<crypto::public_key>(const string& key_str, crypto::public_key& secret_key);


    bool
    get_tx_pub_key_from_str_hash(Blockchain& core_storage, const string& hash_str, transaction& tx)
    {
        crypto::hash tx_hash;
        parse_hash256(hash_str, tx_hash);

        try
        {
            tx = core_storage.get_db().get_tx(tx_hash);
        }
        catch (const TX_DNE& e)
        {
            cerr << e.what() << endl;
            return false;
        }

        return true;
    }

    bool
    parse_str_address(const string& address_str, account_public_address& address)
    {

        if (!get_account_address_from_str(address, false, address_str))
        {
            cerr << "Error getting address: " << address_str << endl;
            return false;
        }

        return true;
    }



}
