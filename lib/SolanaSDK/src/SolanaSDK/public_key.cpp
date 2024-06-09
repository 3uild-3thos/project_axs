#include <string>
#include <optional>
#include <ArduinoJson.h>
#include <sodium.h>
#include "public_key.h"
#include "base58.h"
#include "hash.h"
#include "crypto.h"

bool bytesAreCurvePoint(const std::array<uint8_t, crypto_core_ed25519_BYTES> &bytes) {
    return crypto_core_ed25519_is_valid_point(bytes.data()) != 0;
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
  return Base58::trimEncode(keyVector);
}

void PublicKey::sanitize() {
  // Implement sanitization logic if needed
}

std::optional<PublicKey> PublicKey::fromString(const std::string &s) {
    if (s.length() > PUBLIC_KEY_MAX_BASE58_LEN) {
        return std::nullopt;
    }

    try {
        std::vector<uint8_t> intVec = Base58::decode(s);

        if (intVec.size() != PUBLIC_KEY_LEN) {
            return std::nullopt;
        }

        return PublicKey(intVec);
    } catch (const std::exception &e) {
        return std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
}

std::vector<uint8_t> PublicKey::serialize()
{
  return std::vector<uint8_t>(this->key, this->key + PUBLIC_KEY_LEN);
}

PublicKey PublicKey::deserialize(const std::vector<uint8_t> &data)
{
  if (data.size() != PUBLIC_KEY_LEN) {
    throw ParsePublickeyError("Invalid public key length");
  }

  return PublicKey(data);
}

bool PublicKey::isOnCurve(const std::string &s) {
    auto publicKey = fromString(s);

    if (publicKey.has_value()) {
        std::array<uint8_t, crypto_core_ed25519_BYTES> keyArray;
        std::copy(publicKey->key, publicKey->key + PUBLIC_KEY_LEN, keyArray.begin());
        return bytesAreCurvePoint(keyArray);
    } else {
        return false;
    }
}

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

    PublicKey publicKey = PublicKey(hashResult.toBytes());

    if (!isOnCurve(publicKey.toBase58())) {
        throw ParsePublickeyError("InvalidSeeds");
    }

    return publicKey;
}

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
