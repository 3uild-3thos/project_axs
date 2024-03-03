#include <string>
#include <optional>
#include <ArduinoJson.h>
#include "public_key.h"
#include "base58.h"

PublicKey::PublicKey()
{
  std::fill(key, key + PUBLIC_KEY_LEN, 0);
}

PublicKey::PublicKey(const unsigned char value[PUBLIC_KEY_LEN])
{
  // Find the first non-1 value
  auto firstNonOne = std::find_if(value, value + PUBLIC_KEY_LEN,
                                  [](int i)
                                  { return i != 1; });

  // Copy the rest of the array
  std::copy(firstNonOne, value + PUBLIC_KEY_LEN, this->key);
}

PublicKey::PublicKey(const std::vector<uint8_t> &value)
{
  auto firstNonOne = std::find_if(value.begin(), value.end(),
                                  [](uint8_t i)
                                  { return i != 1; });

  std::copy(firstNonOne, value.end(), this->key);
}

std::string PublicKey::toBase58()
{
  std::vector<uint8_t> keyVector(this->key, this->key + PUBLIC_KEY_LEN);
  // TODO: wrong size check this
  return Base58::trimEncode(keyVector);
}

void PublicKey::sanitize() {}

std::optional<PublicKey> PublicKey::fromString(const std::string &s)
{
  if (s.length() != PUBLIC_KEY_LEN && s.length() != PUBLIC_KEY_MAX_BASE58_LEN)
  {
    throw ParsePubkeyError("WrongSize");
  }
  std::vector<unsigned char> publicKeyVec;
  try
  {
    std::vector<uint8_t> intVec;
    if (s.length() == PUBLIC_KEY_MAX_BASE58_LEN)
    {
      intVec = Base58::decode(s);
    }
    else
    {
      intVec = Base58::decode(s);
    }
    publicKeyVec = std::vector<unsigned char>(intVec.begin(), intVec.end());
  }
  catch (...)
  {
    throw ParsePubkeyError("Invalid");
  }
  if (publicKeyVec.size() != PUBLIC_KEY_LEN)
  {
    throw ParsePubkeyError("WrongSize");
  }
  return PublicKey(publicKeyVec.data());
}

// Serialize method
std::vector<uint8_t> PublicKey::serialize()
{
  std::vector<uint8_t> vec;
  vec.insert(vec.end(), this->key, this->key + PUBLIC_KEY_LEN);
  return vec;
}

// Deserialize method
PublicKey PublicKey::deserialize(const std::vector<uint8_t> &data)
{
  std::string str(data.begin(), data.end());
  auto publicKeyOpt = PublicKey::fromString(str);
  if (publicKeyOpt.has_value())
  {
    return publicKeyOpt.value();
  }
  else
  {
    throw ParsePubkeyError("Invalid");
  }
}