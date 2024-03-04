#include <cstdint>
#include <vector>
#include <array>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <Arduino.h>
#include <sodium/crypto_sign_ed25519.h>
#include "keypair.h"
#include "public_key.h"
#include "signature.h"

Signature::Signature(const std::vector<uint8_t> &signatureSlice)
{
    // Find the first non-zero byte
    auto first_non_zero = std::find_if(signatureSlice.begin(), signatureSlice.end(), [](int byte)
                                       { return byte != 0; });

    // Create a new vector without the initial zeros
    std::vector<uint8_t> trimmedSlice(first_non_zero, signatureSlice.end());

    // Check if the size of the trimmed signature is correct
    if (trimmedSlice.size() != SIGNATURE_BYTES)
    {
        throw std::invalid_argument("Wrong size for signature");
    }

    // Copy the trimmed signature to the value member
    std::copy(trimmedSlice.begin(), trimmedSlice.end(), value.begin());
}

Signature Signature::newUnique()
{
    Signature signature;
    for (auto &byte : signature.value)
    {
        byte = rand() % 256; // Replace with a better random generator if needed
    }
    return signature;
}

void Signature::verify(const std::vector<uint8_t> &pubkeyBytes, const std::vector<uint8_t> &messageBytes)
{
    this->verifyVerbose(pubkeyBytes, messageBytes);
}

std::string Signature::toString() const
{
    std::vector<uint8_t> result;
    std::copy(value.begin(), value.end(), std::back_inserter(result));
    return Base58::trimEncode(result);
}

Signature Signature::fromString(const std::string &s)
{
    std::string s_trimmed = s.substr(s.find_first_not_of('1')); // Skip initial '1's
    if (s_trimmed.size() > MAX_BASE58_SIGNATURE_LEN)
    {
        throw std::invalid_argument("Wrong size for signature");
    }
    std::vector<uint8_t> decoded = Base58::decode(s);
    std::vector<uint8_t> signatureSlice(decoded.begin(), decoded.end());
    return Signature(signatureSlice);
}

void Signature::verifyVerbose(const std::vector<uint8_t> &publicKeyBytes, const std::vector<uint8_t> &messageBytes)
{
    if (crypto_sign_ed25519_verify_detached(this->value.data(), messageBytes.data(), messageBytes.size(), publicKeyBytes.data()) != 0)
    {
        throw std::runtime_error("Signature verify failed");
    }
}

std::vector<uint8_t> Signature::serialize()
{
    std::vector<uint8_t> result(this->value.size());
    std::copy(this->value.begin(), this->value.end(), result.begin());
    return result;
}

Signature Signature::deserialize(const std::vector<uint8_t> &signatureSlice)
{
    Signature signature;
    if (signatureSlice.size() != SIGNATURE_BYTES)
    {
        throw std::invalid_argument("Invalid size for signature");
    }
    std::copy(signatureSlice.begin(), signatureSlice.end(), signature.value.begin());
    return signature;
}

std::ostream &operator<<(std::ostream &os, const Signature &signature)
{
    return os << signature.toString();
}
