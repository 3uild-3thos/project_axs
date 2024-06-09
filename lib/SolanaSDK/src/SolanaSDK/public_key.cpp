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
    return true;
}

PublicKey::PublicKey()
{
  std::fill(key, key + PUBLIC_KEY_LEN, 0);
}

PublicKey::PublicKey(const unsigned char value[PUBLIC_KEY_LEN])
{
  std::copy(value, value + PUBLIC_KEY_LEN, this->key);
}

PublicKey::PublicKey(const std::vector<uint8_t> &value)
{
  std::copy(value.begin(), value.end(), this->key);
}

PublicKey::PublicKey(const std::array<uint8_t, PUBLIC_KEY_LEN> &value) {
    std::copy(value.begin(), value.end(), key);
}

std::string PublicKey::toBase58()
{
  std::vector<uint8_t> keyVector(this->key, this->key + PUBLIC_KEY_LEN);
  // Ensure the vector has the correct size
  if (keyVector.size() != PUBLIC_KEY_LEN) {
    Serial.println("Error: Key vector size is incorrect.");
    return "";
  }
  return Base58::trimEncode(keyVector);
}

void PublicKey::sanitize() {}

std::optional<PublicKey> PublicKey::fromString(const std::string &s) {
    if (s.length() > PUBLIC_KEY_MAX_BASE58_LEN) {
        return std::nullopt;
    }

    try {
        // Decode Base58 string
        std::vector<uint8_t> intVec = Base58::decode(s);

        // Check the length of the decoded vector
        if (intVec.size() != PUBLIC_KEY_LEN) {
            return std::nullopt;
        }

        // Convert to unsigned char vector
        std::vector<unsigned char> publicKeyVec(intVec.begin(), intVec.end());
        return PublicKey(publicKeyVec);
    } catch (const std::exception &e) {
        return std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
}

// Serialize method
std::vector<uint8_t> PublicKey::serialize()
{
  return std::vector<uint8_t>(this->key, this->key + PUBLIC_KEY_LEN);
}

// Deserialize method
PublicKey PublicKey::deserialize(const std::vector<uint8_t> &data)
{
  if (data.size() != PUBLIC_KEY_LEN) {
    throw ParsePublickeyError("Invalid public key length");
  }

  return PublicKey(data);
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

    // Hash each seed
    for (const auto &seed : seeds) {
        hasher.hash(seed.data(), seed.size());
    }

    // Hash the program ID
    hasher.hash(programId.key, PUBLIC_KEY_LEN);

    // Hash the PDA marker
    hasher.hash(PDA_MARKER, sizeof(PDA_MARKER) - 1);

    Hash hashResult;
    hasher.result(&hashResult);

    if (bytesAreCurvePoint(hashResult.toBytes())) {
        throw ParsePublickeyError("InvalidSeeds");
    }

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

    std::vector<uint8_t> bump_seed(1, MAX_BUMP_SEED);

    for (uint8_t i = MAX_BUMP_SEED; i > 0; --i) {
        std::vector<std::vector<uint8_t>> seeds_with_bump = seeds;
        seeds_with_bump.push_back(bump_seed);
        try {
            PublicKey address = createProgramAddress(seeds_with_bump, programId);
            return std::make_pair(address, bump_seed[0]);
        } catch (const ParsePublickeyError &e) {
            if (std::string(e.what()) != "InvalidSeeds") {
                break;
            }
        }
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
