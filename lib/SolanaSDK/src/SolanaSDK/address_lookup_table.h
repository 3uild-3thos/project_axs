#ifndef ADDRESS_LOOKUP_TABLE_H
#define ADDRESS_LOOKUP_TABLE_H

#include <vector>
#include <string>
#include "public_key.h"

class AddressLookupTable
{
public:
  // Address lookup table account key
  PublicKey accountKey;

  // List of indexes used to load writable accounts addresses
  std::vector<uint8_t> writableIndexes;

  // List of indexes used to load readonly account addresses
  std::vector<uint8_t> readonlyIndexes;

  std::vector<uint8_t> serialize();

  static AddressLookupTable deserialize(const std::vector<uint8_t> &data);
};

#endif // ADDRESS_LOOKUP_TABLE_H
