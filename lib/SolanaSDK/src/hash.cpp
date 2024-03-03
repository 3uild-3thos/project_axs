#include <atomic>
#include <sstream>
#include <iomanip>
#include "Hash.h"
#include "base58.h"

Hash::Hash()
{
    this->data.fill(0);
}

Hash::Hash(std::array<uint8_t, HASH_BYTES> &hashArray)
{
    auto firstNonZero = std::find_if(hashArray.begin(), hashArray.end(), [](int i)
                                     { return i != 0; });
    std::copy(firstNonZero, hashArray.end(), this->data.begin());
}

Hash::Hash(const std::vector<uint8_t> &hashSlice)
{
    if (hashSlice.size() != HASH_BYTES && hashSlice.size() != HASH_MAX_BASE58_LEN)
    {
        throw std::runtime_error("Invalid vector size");
    }
    auto firstNonZero = std::find_if(hashSlice.begin(), hashSlice.end(), [](uint8_t i)
                                     { return i != 0; });

    if (firstNonZero == hashSlice.end())
    {
        // All elements in the hash are zero
        std::copy(hashSlice.begin(), hashSlice.end(), this->data.begin());
    }
    else if (std::distance(firstNonZero, hashSlice.end()) != HASH_BYTES)
    {
        // Hash size is not HASH_BYTES after the first non-zero element
        throw std::runtime_error("Invalid hash size");
    }
    else
    {
        // Copy the hash starting from the first non-zero element
        std::copy(firstNonZero, hashSlice.end(), this->data.begin());
    }
}

// Constructor to initialize from an array
Hash Hash::newFromArray(std::array<uint8_t, HASH_BYTES> hashArray)
{
    return Hash(hashArray);
}

// Static method to create a unique Hash
Hash Hash::newUnique()
{
    static uint64_t I = 1;
    std::array<uint8_t, HASH_BYTES> b{};
    std::memcpy(b.data(), &I, sizeof(uint64_t));
    I++;
    return Hash(b);
}

// Method to convert the Hash to an array of bytes
std::array<uint8_t, HASH_BYTES> Hash::toBytes() const
{
    return data;
}

void Hash::sanitize() {}

// Serialize method
std::vector<uint8_t> Hash::serialize()
{
    std::vector<uint8_t> result(data.begin(), data.end());
    return result;
}

// Deserialize method
Hash Hash::deserialize(const std::vector<uint8_t> &input)
{
    if (input.size() != HASH_BYTES)
    {
        throw std::invalid_argument("Invalid hash string");
    }
    Hash hash;
    for (size_t i = 0; i < HASH_BYTES; ++i)
    {
        hash.data[i] = input[i];
    }
    return hash;
}

// Method to create a Hash from a Base58 encoded string
Hash Hash::fromString(const std::string &str)
{
    std::string s_trimmed = str.substr(str.find_first_not_of('1')); // Skip initial '1's
    const unsigned char *uchar_str = reinterpret_cast<const unsigned char *>(s_trimmed.c_str());
    std::vector<uint8_t> decoded;

    if (s_trimmed.size() == HASH_MAX_BASE58_LEN)
    {
        decoded = Base58::decode(str);
    }
    else if (s_trimmed.size() == HASH_BYTES)
    {
        decoded = Base58::decode(str);
    }
    else
    {
        throw std::invalid_argument("Invalid string length");
    }

    if (decoded.size() != HASH_BYTES)
    {
        throw std::invalid_argument("Invalid hash string");
    }

    Hash hash;
    for (size_t i = 0; i < HASH_BYTES; ++i)
    {
        hash.data[i] = decoded[i];
    }
    return hash;
}

std::string Hash::toStr()
{
    if (data.size() != HASH_BYTES)
    {
        throw std::invalid_argument("Invalid byte length for hash");
    }

    // Convert std::array to std::vector
    std::vector<uint8_t> dataVector(data.begin(), data.end());

    std::string base58Encoded = Base58::trimEncode(dataVector);
    return base58Encoded;
}

void Hasher::hash(const uint8_t *val, size_t len)
{
    this->hasher.doUpdate(val, len);
}

void Hasher::hashv(const std::vector<uint8_t *> &vals)
{
    for (auto &val : vals)
    {
        this->hash(val, std::strlen(reinterpret_cast<const char *>(val)));
    }
}

void Hasher::result(Hash *hash)
{
    this->hasher.doFinal(reinterpret_cast<byte *>(hash->data.data()));
}
