#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <cstdint>
#include <vector>
#include <array>
#include <stdexcept>
#include <string>
#include <algorithm>
#include "keypair.h"
#include "public_key.h"

// Number of bytes in a signature
constexpr size_t SIGNATURE_BYTES = 64;

// Maximum string length of a base58 encoded signature
constexpr size_t MAX_BASE58_SIGNATURE_LEN = 88;

class Signature
{
public:
    std::array<uint8_t, SIGNATURE_BYTES> value;

    Signature() = default;
    Signature(const std::vector<uint8_t> &signatureSlice);
    static Signature newUnique();
    void verify(const std::vector<uint8_t> &pubkeyBytes, const std::vector<uint8_t> &message_bytes);
    std::string toString() const;
    static Signature fromString(const std::string &s);
    std::vector<uint8_t> serialize();
    static Signature deserialize(const std::vector<uint8_t> &signatureSlice);

    uint8_t operator*() const
    {
        return value[0];
    }

    bool operator!=(const Signature &other) const
    {
        return value != other.value;
    }

    Signature operator+(const Signature &other) const
    {
        Signature result;
        for (int i = 0; i < SIGNATURE_BYTES; i++)
        {
            result.value[i] = this->value[i] + other.value[i];
        }
        return result;
    }

    Signature operator=(const Signature &other)
    {
        if (this != &other)
        {
            std::copy(other.value.begin(), other.value.end(), value.begin());
        }
        return *this;
    }

private:
    void verifyVerbose(const std::vector<uint8_t> &publicKeyBytes, const std::vector<uint8_t> &messageBytes);
};

std::ostream &operator<<(std::ostream &os, const Signature &signature);

class Signable
{
public:
    virtual void sign(const Keypair &keypair) = 0;
    virtual bool verify() const = 0;
    virtual PublicKey pubkey() const = 0;
    virtual std::vector<uint8_t> signable_data() const = 0;
    virtual Signature get_signature() const = 0;
    virtual void set_signature(const Signature &signature) = 0;
};

#endif // SIGNATURE_H
