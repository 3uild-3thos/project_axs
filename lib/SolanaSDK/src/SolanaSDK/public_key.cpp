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

PublicKey::PublicKey() {
    std::fill(key, key + PUBLIC_KEY_LEN, 0);
}

PublicKey::PublicKey(const unsigned char value[PUBLIC_KEY_LEN]) {
    std::copy(value, value + PUBLIC_KEY_LEN, this->key);
}

PublicKey::PublicKey(const std::vector<uint8_t> &value) {
    std::copy(value.begin(), value.end(), this->key);
}

PublicKey::PublicKey(const std::array<uint8_t, PUBLIC_KEY_LEN> &value) {
    std::copy(value.begin(), value.end(), key);
}

std::string PublicKey::toBase58() {
    std::vector<uint8_t> keyVector(this->key, this->key + PUBLIC_KEY_LEN);
    // Ensure the vector has the correct size
    if (keyVector.size() != PUBLIC_KEY_LEN) {
        Serial.println("Error: Key vector size is incorrect.");
        return "";
    }
    return Base58::trimEncode(keyVector);
}

void PublicKey::sanitize() {
    // Implementation for sanitizing the public key if needed
}

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
std::vector<uint8_t> PublicKey::serialize() {
    return std::vector<uint8_t>(this->key, this->key + PUBLIC_KEY_LEN);
}

// Deserialize method
PublicKey PublicKey::deserialize(const std::vector<uint8_t> &data) {
    if (data.size() != PUBLIC_KEY_LEN) {
        throw ParsePublickeyError("Invalid public key length");
    }

    return PublicKey(data);
}

bool PublicKey::isOnCurve(const std::string &s) {
    std::optional<PublicKey> publicKey = fromString(s);

    if (publicKey.has_value()) {
        std::array<uint8_t, crypto_core_ed25519_BYTES> keyArray;
        std::copy(publicKey->key, publicKey->key + PUBLIC_KEY_LEN, keyArray.begin());
        return bytesAreCurvePoint(keyArray);
    } else {
        return false;
    }
}

// Create a valid [program derived address][pda] without searching for a bump seed.
std::optional<PublicKey> PublicKey::createProgramAddress(const std::vector<std::vector<uint8_t>> &seeds, const PublicKey &programId) {
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

    if (bytesAreCurvePoint(hashResult.toBytes())) {
        return std::nullopt;
    }

    return PublicKey(hashResult.toBytes());
}

// Find a valid [program derived address][pda] and its corresponding bump seed.
std::optional<std::pair<PublicKey, uint8_t>> PublicKey::tryFindProgramAddress(const std::vector<std::vector<uint8_t>> &seeds, const PublicKey &programId) {
    std::vector<uint8_t> bump_seed(1, MAX_BUMP_SEED);

    for (uint8_t i = MAX_BUMP_SEED; i > 0; --i) {
        std::vector<std::vector<uint8_t>> seeds_with_bump = seeds;
        seeds_with_bump.push_back(bump_seed);
        std::optional<PublicKey> address = createProgramAddress(seeds_with_bump, programId);
        if (address.has_value()) {
            return std::make_pair(address.value(), bump_seed[0]);
        } else {
            bump_seed[0] -= 1;
        }
    }
    return std::nullopt;
}


std::pair<PublicKey, uint8_t> PublicKey::findProgramAddress(const std::vector<std::vector<uint8_t>> &seeds, const PublicKey &programId) {
    auto result = tryFindProgramAddress(seeds, programId);
    if (!result.has_value()) {
        throw ParsePublickeyError("Unable to find a viable program address bump seed");
    }
    return result.value();
}
