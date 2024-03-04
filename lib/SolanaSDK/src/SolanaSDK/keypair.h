#ifndef KEYPAIR_H
#define KEYPAIR_H

#include <sodium/crypto_sign_ed25519.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include "public_key.h"
#include "base58.h"

constexpr std::size_t SECRET_KEY_LEN = 64;

class Keypair
{
private:
    unsigned char secretKey[SECRET_KEY_LEN];

public:
    PublicKey publicKey;

    // Default constructor for zero-initialized keys
    Keypair();

    // Generate keypair from a secure seed
    Keypair(const std::vector<unsigned char> &seed);

    // Generate keypair from a secure seed (convenience for C arrays)
    Keypair(const unsigned char seed[SECRET_KEY_LEN]);

    // Access secret key with authorization
    const unsigned char *getSecretKey();

    // Destructor to securely clear secret key
    ~Keypair();

    // Generate a new Keypair with a random seed
    static Keypair generate();
};

#endif // KEYPAIR_H