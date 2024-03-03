#include <cstdint>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <ArduinoJson.h>
#include "instruction.h"
#include "public_key.h"
#include "account_meta.h"

// Create a new instruction from a byte slice.
Instruction Instruction::newWithBytes(PublicKey programId, std::vector<uint8_t> &data, std::vector<AccountMeta> accounts)
{
    return {
        programId,
        accounts,
        data,
    };
}

// Serialize method for Instruction
std::vector<uint8_t> Instruction::serialize()
{
    std::vector<uint8_t> result;
    auto programIdBytes = this->programId.serialize();
    result.insert(result.end(), programIdBytes.begin(), programIdBytes.end());
    for (auto account : accounts)
    {
        auto accountBytes = account.serialize();
        result.insert(result.end(), accountBytes.begin(), accountBytes.end());
    }
    result.insert(result.end(), data.begin(), data.end());
    return result;
}

// Deserialize method for Instruction
Instruction Instruction::deserialize(const std::vector<uint8_t> &input)
{
    Instruction instruction;
    std::vector<uint8_t>::const_iterator it = input.begin();

    // Deserialize programId
    std::vector<uint8_t> programIdBytes(it, it + sizeof(PublicKey));
    instruction.programId = PublicKey::deserialize(programIdBytes);
    it += sizeof(PublicKey);

    // Deserialize accounts
    while (it != input.end())
    {
        std::vector<uint8_t> accountBytes(it, it + sizeof(AccountMeta));
        AccountMeta accountMeta = AccountMeta::deserialize(accountBytes);
        instruction.accounts.push_back(accountMeta);
        it += sizeof(AccountMeta);
    }

    // Deserialize data
    instruction.data.assign(it, input.end());

    return instruction;
}

template <typename T>
CompiledInstruction::CompiledInstruction(uint8_t programIdsIndex, const T &data, std::vector<uint8_t> accounts)
    : programIdIndex(programIdsIndex), accounts(std::move(accounts))
{
    this->data.insert(this->data.end(), data.begin(), data.end());
}

CompiledInstruction::CompiledInstruction(uint8_t programIdIndex, std::vector<uint8_t> data, std::vector<uint8_t> accounts)
    : programIdIndex(programIdIndex), data(std::move(data)), accounts(std::move(accounts)) {}

PublicKey CompiledInstruction::programId(const std::vector<PublicKey> &programIds) const
{
    return programIds[this->programIdIndex];
}

void CompiledInstruction::sanitize() {}

// Serialize method for CompiledInstruction
std::vector<uint8_t> CompiledInstruction::serialize()
{
    std::vector<uint8_t> result;

    // Serialize programIdIndex
    result.push_back(static_cast<uint8_t>(programIdIndex));

    // Serialize number of accounts
    uint8_t numAccounts = static_cast<uint8_t>(accounts.size());
    result.push_back(numAccounts);

    // Serialize accounts
    for (const auto &accountIndex : accounts)
    {
        result.push_back(static_cast<uint8_t>(accountIndex));
    }

    // Serialize data size
    size_t dataSize = data.size();
    result.push_back(static_cast<uint8_t>(dataSize));

    // TODO: CHECK WHAT TO US LSB/MSB
    // result.push_back(static_cast<uint8_t>((dataSize >> 0) & 0xFF)); // LSB
    // result.push_back(static_cast<uint8_t>((dataSize >> 8) & 0xFF)); // MSB

    // Serialize data
    result.insert(result.end(), data.begin(), data.end());

    return result;
}

// Deserialize method for CompiledInstruction
CompiledInstruction CompiledInstruction::deserialize(uint8_t programIdIndex, const std::vector<uint8_t> &accounts, const std::vector<uint8_t> &data, const std::vector<uint8_t> &input)
{
    CompiledInstruction instruction(programIdIndex, data, accounts);
    std::vector<uint8_t>::const_iterator it = input.begin();

    // Deserialize programIdIndex
    instruction.programIdIndex = *it++;

    // Deserialize accounts
    while (it != input.end() && *it != 0)
    {
        instruction.accounts.push_back(*it++);
    }

    // Skip the zero byte separator
    if (it != input.end())
    {
        ++it;
    }

    // Deserialize data
    instruction.data.assign(it, input.end());

    return instruction;
}

uint8_t position(const std::vector<PublicKey> &keys, const PublicKey &key)
{
    auto it = std::find(keys.begin(), keys.end(), key);
    if (it == keys.end())
    {
        throw std::runtime_error("Key not found");
    }
    return std::distance(keys.begin(), it);
}

CompiledInstruction compileIx(const Instruction &ix, const std::vector<PublicKey> &keys)
{
    std::vector<uint8_t> accounts;
    for (const auto &accountMeta : ix.accounts)
    {
        accounts.push_back(position(keys, accountMeta.publicKey));
    }
    return CompiledInstruction{position(keys, ix.programId), ix.data, accounts};
}

std::vector<CompiledInstruction> compileInstructions(const std::vector<Instruction> &ixs, const std::vector<PublicKey> &keys)
{
    std::vector<CompiledInstruction> compiled;
    for (const auto &ix : ixs)
    {
        compiled.push_back(compileIx(ix, keys));
    }
    return compiled;
}