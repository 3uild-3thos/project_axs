#include <string>
#include <optional>
#include <ArduinoJson.h>
#include <sodium.h>
#include "public_key.h"
#include "base58.h"
#include "hash.h"

bool bytesAreCurvePoint(const std::array<uint8_t, HASH_BYTES> &bytes) {
    unsigned char decompressed_point[crypto_core_ed25519_BYTES];
    if (crypto_core_ed25519_from_uniform(decompressed_point, bytes.data()) != 0) {
        return false;
    }
    crypto_core_ed25519_from_uniform(decompressed_point, bytes.data());
    return true;
}

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

PublicKey::PublicKey(const std::array<uint8_t, PUBLIC_KEY_LEN> &value) {
    std::copy(value.begin(), value.end(), key);
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
    std::vector<uint8_t> publicKeyVec;
    try
    {
        std::vector<uint8_t> intVec;
        if (s.length() == PUBLIC_KEY_MAX_BASE58_LEN)
        {
            // Decode the base58 string with padding
            intVec = Base58::decode(s);
        }
        else if (s.length() == PUBLIC_KEY_LEN)
        {
            // Decode the base58 string without padding
            intVec = Base58::decode(s);
        }
        else
        {
            throw ParsePublickeyError("Invalid length");
        }

        publicKeyVec = std::vector<unsigned char>(intVec.begin(), intVec.end());
    }
    catch (...)
    {
        throw ParsePublickeyError("Invalid base58 encoding");
    }

    // If the decoded vector is shorter than the expected length,
    // pad it with zeros
    if (publicKeyVec.size() < PUBLIC_KEY_LEN)
    {
        publicKeyVec.insert(publicKeyVec.begin(), PUBLIC_KEY_LEN - publicKeyVec.size(), 0);
    }
    // If the decoded vector is longer than the expected length,
    // trim the extra bytes
    else if (publicKeyVec.size() > PUBLIC_KEY_LEN)
    {
        publicKeyVec.erase(publicKeyVec.begin() + PUBLIC_KEY_LEN, publicKeyVec.end());
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
    throw ParsePublickeyError("Invalid");
  }
}

// Create a valid [program derived address][pda] without searching for a bump seed.
//
// [pda]: https://solana.com/docs/core/cpi#program-derived-addresses
//
// Because this function does not create a bump seed, it may unpredictably
// return an error for any given set of seeds and is not generally suitable
// for creating program derived addresses.
//
// However, it can be used for efficiently verifying that a set of seeds plus
// bump seed generated by [`findProgramAddress`] derives a particular
// address as expected. See the example for details.
//
// See the documentation for [`findProgramAddress`] for a full description
// of program derived addresses and bump seeds.
//
// [`findProgramAddress`]: Pubkey::findProgramAddress
//
PublicKey PublicKey::createProgramAddress(const std::vector<std::vector<uint8_t>> &seeds, const PublicKey &programId) {
    if (seeds.size() > MAX_SEEDS) {
        throw ParsePublickeyError("MaxSeedLengthExceeded");
    }
    for (const auto &seed : seeds) {
        if (seed.size() > MAX_SEED_LEN) {
            throw ParsePublickeyError("MaxSeedLengthExceeded");
        }
    }

     Hasher hasher;
    for (const auto &seed : seeds) {
        hasher.hash(seed.data(), seed.size());
    }
    hasher.hash(programId.key, PUBLIC_KEY_LEN);
    hasher.hash(PDA_MARKER, sizeof(PDA_MARKER) - 1); // Subtract 1 to exclude the null terminator

    Hash hashResult;
    hasher.result(&hashResult);

    // if (bytesAreCurvePoint(hashResult.toBytes())) {
    //     throw ParsePublickeyError("InvalidSeeds");
    // }

    return PublicKey(hashResult.toBytes());
}

// Find a valid [program derived address][pda] and its corresponding bump seed.
//
// [pda]: https://solana.com/docs/core/cpi#program-derived-addresses
//
// The only difference between this method and [`findProgramAddress`]
// is that this one returns `None` in the statistically improbable event
// that a bump seed cannot be found; or if any of `findProgramAddress`'s
// preconditions are violated.
//
// See the documentation for [`findProgramAddress`] for a full description.
//
// [`findProgramAddress`]: Pubkey::findProgramAddress
std::optional<std::pair<PublicKey, uint8_t>> PublicKey::tryFindProgramAddress(
    const std::vector<std::vector<uint8_t>> &seeds, 
    const PublicKey &programId) {

    std::vector<std::vector<uint8_t>> seeds_with_bump = seeds;
    std::vector<uint8_t> bump_seed(1, MAX_BUMP_SEED);

    for (uint8_t i = MAX_BUMP_SEED; i > 0; --i) {
        seeds_with_bump.push_back(bump_seed);
        try {
            PublicKey address = createProgramAddress(seeds_with_bump, programId);
            return std::make_pair(address, bump_seed[0]);
        } catch (const ParsePublickeyError &e) {
            if (std::string(e.what()) != "InvalidSeeds") {
                break;
            }
        }
        seeds_with_bump.pop_back();
        bump_seed[0] -= 1;
    }
    return std::nullopt;
}

std::pair<PublicKey, uint8_t> PublicKey::findProgramAddress(
    const std::vector<std::vector<uint8_t>> &seeds, 
    const PublicKey &programId) {
    
    auto result = tryFindProgramAddress(seeds, programId);
    if (!result.has_value()) {
        throw ParsePublickeyError("Unable to find a viable program address bump seed");
    }
    return result.value();
}