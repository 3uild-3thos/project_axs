#include <iostream>
#include <sstream>
#include "public_key.h"
#include "account_meta.h"

// Construct metadata for a writable account.
AccountMeta *AccountMeta::newWritable(PublicKey publicKey, bool isSigner)
{
    AccountMeta *accountMeta = new AccountMeta();
    accountMeta->publicKey = publicKey;
    accountMeta->isSigner = isSigner;
    accountMeta->isWritable = true;
    return accountMeta;
}

// Construct metadata for a read-only account.
AccountMeta *AccountMeta::newReadonly(PublicKey publicKey, bool isSigner)
{
    AccountMeta *accountMeta = new AccountMeta();
    accountMeta->publicKey = publicKey;
    accountMeta->isSigner = isSigner;
    accountMeta->isWritable = false;
    return accountMeta;
}

// Serialize the AccountMeta object to a vector of bytes
std::vector<uint8_t> AccountMeta::serialize()
{
    std::vector<uint8_t> result;
    auto publicKeyBytes = this->publicKey.serialize();
    result.insert(result.end(), publicKeyBytes.begin(), publicKeyBytes.end());
    result.push_back(this->isSigner);
    result.push_back(this->isWritable);
    return result;
}

// Deserialize the AccountMeta object from a vector of bytes
AccountMeta AccountMeta::deserialize(const std::vector<uint8_t> &input)
{
    if (input.size() < PUBLIC_KEY_LEN + 2)
    {
        throw std::runtime_error("Invalid input vector for deserialization");
    }
    AccountMeta accountMeta;
    std::vector<uint8_t> publicKeyBytes(input.begin(), input.begin() + PUBLIC_KEY_LEN);

    accountMeta.publicKey = PublicKey::deserialize(publicKeyBytes);
    accountMeta.isSigner = input[PUBLIC_KEY_LEN];
    accountMeta.isWritable = input[PUBLIC_KEY_LEN + 1];
    return accountMeta;
}