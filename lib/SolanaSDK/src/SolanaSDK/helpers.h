#ifndef BUFFER_TO_HEX_UINT8_VEC_H
#define BUFFER_TO_HEX_UINT8_VEC_H

#include <vector>
#include <string>

class Helpers {
    Helpers();
    public:
        std::vector<uint8_t> Helpers::buffer_to_hex_uint8_vector(std::vector<uint8_t> buffer);
        std::vector<uint8_t> Helpers::uint64_to_hex_uint8_vector(uint64_t number);
};


#endif