#include <vector>
#include <string>
#include <iostream>
#include "address_lookup_table.h"
#include "public_key.h"

std::vector<uint8_t> AddressLookupTable::serialize()
{
  std::vector<uint8_t> serializedData;

  // Serialize account key
  std::vector<uint8_t> accountKeyBytes = accountKey.serialize();
  serializedData.insert(serializedData.end(), accountKeyBytes.begin(), accountKeyBytes.end());

  // Serialize writable indexes
  serializedData.push_back(writableIndexes.size());
  serializedData.insert(serializedData.end(), writableIndexes.begin(), writableIndexes.end());

  // Serialize readonly indexes
  serializedData.push_back(readonlyIndexes.size());
  serializedData.insert(serializedData.end(), readonlyIndexes.begin(), readonlyIndexes.end());

  return serializedData;
}

AddressLookupTable AddressLookupTable::deserialize(const std::vector<uint8_t> &data)
{
  AddressLookupTable table;

  size_t index = 0;

  // Deserialize account key
  std::vector<uint8_t> accountKeyData(data.begin(), data.begin() + PUBLIC_KEY_LEN);
  table.accountKey = PublicKey::deserialize(accountKeyData);

  index += static_cast<size_t>(PUBLIC_KEY_LEN);

  // Deserialize writable indexes
  uint8_t writableIndexSize = data[index++];
  table.writableIndexes.resize(writableIndexSize);
  std::copy(data.begin() + index, data.begin() + index + writableIndexSize, table.writableIndexes.begin());
  index += writableIndexSize;

  // Deserialize readonly indexes
  uint8_t readonlyIndexSize = data[index++];
  table.readonlyIndexes.resize(readonlyIndexSize);
  std::copy(data.begin() + index, data.begin() + index + readonlyIndexSize, table.readonlyIndexes.begin());
  index += readonlyIndexSize;

  return table;
}
