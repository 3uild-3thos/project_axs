#ifndef PUBLIC_KEY_H
#define PUBLIC_KEY_H

#include <string>
#include <optional>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include "base58.h"

const uint8_t PUBLIC_KEY_LEN = 32;

const uint8_t PUBLIC_KEY_MAX_BASE58_LEN = 44;

class ParsePubkeyError : public std::runtime_error
{
public:
    explicit ParsePubkeyError(const std::string &arg) : std::runtime_error(arg) {}
    explicit ParsePubkeyError(const char *arg) : std::runtime_error(arg) {}
};

class PublicKey
{
public:
    unsigned char key[PUBLIC_KEY_LEN];

    // Default constructor
    PublicKey();

    // Parameterized constructor
    PublicKey(const unsigned char value[PUBLIC_KEY_LEN]);

    PublicKey(const std::vector<uint8_t> &value);

    // Convert key to base58
    std::string toBase58();

    // sanitize
    void sanitize();

    static std::optional<PublicKey> fromString(const std::string &s);

    std::vector<uint8_t> serialize();

    static PublicKey deserialize(const std::vector<uint8_t> &data);

    // Less-than operator
    bool operator<(const PublicKey &other) const
    {
        return std::lexicographical_compare(key, key + PUBLIC_KEY_LEN, other.key, other.key + PUBLIC_KEY_LEN);
    }

    // Equality operator
    bool operator==(const PublicKey &other) const
    {
        return std::equal(key, key + PUBLIC_KEY_LEN, other.key);
    }

    PublicKey &operator=(const PublicKey &other)
    {
        if (this != &other)
        {
            std::copy(other.key, other.key + PUBLIC_KEY_LEN, key);
        }
        return *this;
    }

    // Overload * operator.
    PublicKey operator*(const PublicKey &other) const
    {
        PublicKey result;
        for (int i = 0; i < PUBLIC_KEY_LEN; ++i)
        {
            result.key[i] = this->key[i] * other.key[i];
        }
        return result;
    }

    // Overload unary * operator.
    PublicKey operator*() const
    {
        // Return a copy of the object that the pointer points to.
        return *this;
    }

    // Overload [] operator.
    unsigned char &operator[](int index)
    {
        if (index < 0 || index >= PUBLIC_KEY_LEN)
        {
            throw std::out_of_range("Index out of range");
        }
        return key[index];
    }

    // Overload [] operator for const objects.
    const unsigned char &operator[](int index) const
    {
        if (index < 0 || index >= PUBLIC_KEY_LEN)
        {
            throw std::out_of_range("Index out of range");
        }
        return key[index];
    }

    // Overload << operator for output
    friend std::ostream &operator<<(std::ostream &os, const PublicKey &pk)
    {
        for (int i = 0; i < PUBLIC_KEY_LEN; ++i)
        {
            os << std::to_string(static_cast<char>(pk.key[i])) << " ";
        }
        return os;
    }

    // Overload >> operator for input
    friend std::istream &operator>>(std::istream &is, PublicKey &pk)
    {
        for (int i = 0; i < PUBLIC_KEY_LEN; ++i)
        {
            is >> pk.key[i];
        }
        return is;
    }

    // Addition operator
    PublicKey operator+(const PublicKey &other) const
    {
        PublicKey result;
        for (int i = 0; i < PUBLIC_KEY_LEN; ++i)
        {
            result.key[i] = this->key[i] + other.key[i];
        }
        return result;
    }
};

#endif // PUBLIC_KEY_H
