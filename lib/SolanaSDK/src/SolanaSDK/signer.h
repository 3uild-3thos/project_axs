#ifndef SIGNER_H
#define SIGNER_H

#include <sodium/crypto_sign_ed25519.h>
#include <string>
#include <vector>
#include "keypair.h"
#include "base58.h"
#include "public_key.h"
#include "signature.h"

class Signer
{
private:
    Keypair keypair;

public:
    Signer(Keypair kp);

    std::string sign(const std::string &message);

    PublicKey publicKey();

    Signature signMessage(const std::vector<uint8_t> &message);

    // TODO: add tryPublicKeys & trySignMessage
};

// Convenience trait for working with mixed collections of Signers.
class Signers
{
public:
    std::vector<Signer> signers;

    Signers() = default;

    Signers(std::vector<Signer> &signers);

    std::vector<PublicKey> publicKeys();

    std::vector<Signature> signMessage(const std::vector<uint8_t> &message);

    // TODO: add tryPublicKeys & trySignMessage
};

#endif // SIGNER_H
