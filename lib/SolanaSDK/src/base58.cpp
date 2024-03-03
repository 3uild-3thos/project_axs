#include "Base58.h"
#include <Arduino.h>
#include <iostream>
#include <iomanip>

const std::string Base58::ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

Base58::Base58() {}

void Base58::printArray(const std::vector<unsigned char> &arr)
{
    for (const auto &el : arr)
    {
        Serial.print(static_cast<int>(el));
        Serial.print(",");
    }
    Serial.println();
}

std::string Base58::encode(const std::vector<uint8_t> &input)
{
    std::vector<char> output(input.size() * 2, 0);
    int c, tmp;
    for (const auto &in : input)
    {
        c = in;
        for (int j = output.size() - 1; j >= 0; --j)
        {
            tmp = output[j] * 256 + c;
            c = tmp / 58;
            output[j] = tmp % 58;
        }
    }
    for (auto &out : output)
    {
        out = ALPHABET[out];
    }
    return std::string(output.begin(), output.end());
}

std::vector<uint8_t> Base58::decode(const std::string &addr)
{
    std::vector<uint8_t> buf(addr.size(), 0);
    int c, tmp;
    for (const auto &a : addr)
    {
        c = ALPHABET.find(a);
        for (int j = buf.size() - 1; j >= 0; --j)
        {
            tmp = buf[j] * 58 + c;
            c = (tmp & (~0xff)) >> 8;
            buf[j] = tmp & 0xff;
        }
    }
    return buf;
}

std::string Base58::trimEncode(const std::vector<uint8_t> &input)
{
    std::string encoded = encode(input);
    size_t first = encoded.find_first_not_of('1');
    if (std::string::npos == first)
    {
        return "";
    }
    size_t last = encoded.find_last_not_of('1');
    return encoded.substr(first, (last - first + 1));
}

std::vector<uint8_t> Base58::trimDecode(const std::string &addr)
{
    std::vector<uint8_t> decoded = decode(addr);
    auto start = std::find_if(decoded.begin(), decoded.end(), [](int i)
                              { return i != 0; });
    return std::vector<uint8_t>(start, decoded.end());
}
