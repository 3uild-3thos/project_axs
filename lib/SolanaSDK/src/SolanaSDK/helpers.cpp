#include <vector>
#include <string>
#include <algorithm>
#include "helpers.h"

Helpers::Helpers() {};

std::vector<uint8_t> Helpers::buffer_to_hex_uint8_vector(std::vector<uint8_t> buffer) {
    std::vector<uint8_t> result;
    for (size_t i = 0; i < buffer.size(); ++i) {
        result.push_back(static_cast<uint8_t>(buffer[i]));
    }
    return result;
}

std::vector<uint8_t> Helpers::uint64_to_hex_uint8_vector(uint64_t number) {
    std::vector<uint8_t> amount_hex;
    std::vector<uint8_t> transferData = {
        0x02, 0x00, 0x00, 0x00};
    
    // Extract individual bytes
    amount_hex.push_back((number >> 56) & 0xFF);
    amount_hex.push_back((number >> 48) & 0xFF);
    amount_hex.push_back((number >> 40) & 0xFF);
    amount_hex.push_back((number >> 32) & 0xFF);
    amount_hex.push_back((number >> 24) & 0xFF);
    amount_hex.push_back((number >> 16) & 0xFF);
    amount_hex.push_back((number >> 8) & 0xFF);
    amount_hex.push_back(number & 0xFF);
    std::reverse(amount_hex.begin(), amount_hex.end());
    transferData.insert(transferData.end(), amount_hex.begin(), amount_hex.end());

    return transferData;
}