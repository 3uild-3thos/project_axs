#include <cstdint>
#include <vector>
#include <algorithm>
#include <mutex>
#include <list>
#include <array>
#include <iterator>
#include <iostream>
#include <sstream>
#include "public_key.h"
#include "hash.h"
#include "instruction.h"
#include "compiled_keys.h"
#include "message.h"
#include "programs/sysvar/sysvar.h"
#include "programs/bpf_loader.h"
#include "programs/bpf_loader_deprecated.h"
#include "programs/bpf_loader_upgradeable.h"
#include "programs/system_program.h"

std::array<PublicKey, 10> BUILTIN_PROGRAMS_KEYS;
std::array<bool, 256> MAYBE_BUILTIN_KEY_OR_SYSVAR;

void initialize()
{
  static std::once_flag flag;
  std::call_once(flag, []
                 {
        BUILTIN_PROGRAMS_KEYS = {
            PublicKey::fromString("Config1111111111111111111111111111111111111").value(),
            PublicKey::fromString("Feature111111111111111111111111111111111111").value(),
            PublicKey::fromString("NativeLoader1111111111111111111111111111111").value(),
            PublicKey::fromString("Stake11111111111111111111111111111111111111").value(),
            PublicKey::fromString("StakeConfig11111111111111111111111111111111").value(),
            PublicKey::fromString("Vote111111111111111111111111111111111111111").value(),
            SystemProgram::id(),
            BPFLoader::id(),
            BPFLoaderDeprecated::id(),
            BPFLoaderUpgradeable::id()
        };

        for (const auto& key : BUILTIN_PROGRAMS_KEYS) {
            MAYBE_BUILTIN_KEY_OR_SYSVAR[key[0]] = true;
        }

        for (const auto& key : sysvar::ALL_IDS) {
            MAYBE_BUILTIN_KEY_OR_SYSVAR[key[0]] = true;
        } });
}

bool isBuiltinKeyOrSysvar(const PublicKey &key)
{
  initialize();

  if (MAYBE_BUILTIN_KEY_OR_SYSVAR[key[0]])
  {
    return sysvar::isSysvarId(key) || std::find(BUILTIN_PROGRAMS_KEYS.begin(), BUILTIN_PROGRAMS_KEYS.end(), key) != BUILTIN_PROGRAMS_KEYS.end();
    return true;
  }

  return false;
}

void Message::sanitize()
{
  // signing area and read--only on-signing area should no overlap
  if (this->header.numRequiredSignatures + this->header.numReadonlyUnsignedAccounts > this->accountKeys.size())
  {
    throw std::out_of_range("Number of required signatures plus read-only unsigned accounts exceeds total number of account keys");
  }

  // there should be at least 1 RW fee-payer account
  if (this->header.numReadonlySignedAccounts >= this->header.numRequiredSignatures)
  {
    throw std::out_of_range("Need at least 1 read-only signed account");
  }

  // Check address lookup tables
  size_t totalAccountKeys = this->accountKeys.size();
  size_t maxAccountIndex = totalAccountKeys > 0 ? totalAccountKeys - 1 : 0;

  for (const auto &lookup : this->addressTableLookups)
  {
    size_t numLookupIndexes = lookup.writableIndexes.size() + lookup.readonlyIndexes.size();
    if (numLookupIndexes == 0)
    {
      throw std::out_of_range("Each lookup table must be used to load at least one account");
    }

    totalAccountKeys += numLookupIndexes;
    if (totalAccountKeys > 256)
    {
      throw std::out_of_range("The combined number of static and dynamic account keys must be <= 256");
    }

    size_t maxAccountIndexLookup = totalAccountKeys > 0 ? totalAccountKeys - 1 : 0;

    for (const auto &ai : lookup.writableIndexes)
    {
      if (ai > maxAccountIndexLookup)
      {
        throw std::out_of_range("Writable index out of bounds");
      }
    }

    for (const auto &ai : lookup.readonlyIndexes)
    {
      if (ai > maxAccountIndexLookup)
      {
        throw std::out_of_range("Read-only index out of bounds");
      }
    }
  }

  for (const auto &ci : this->instructions)
  {
    if (ci.programIdIndex >= this->accountKeys.size())
    {
      throw std::out_of_range("Invalid program id index");
    }

    // A program cannot be a payer
    if (ci.programIdIndex == 0)
    {
      throw std::out_of_range("Program cannot be a payer");
    }

    for (const auto &ai : ci.accounts)
    {
      if (ai >= this->accountKeys.size())
      {
        throw std::out_of_range("Invalid account keys size");
      }
    }
  }

  for (auto &ak : this->accountKeys)
  {
    ak.sanitize();
  }

  this->recentBlockhash.sanitize();

  for (auto &ix : this->instructions)
  {
    ix.sanitize();
  }
}

Message::Message(MessageHeader header, std::vector<PublicKey> accountKeys, Hash recentBlockhash, std::vector<CompiledInstruction> instructions)
    : header(header), accountKeys(accountKeys), recentBlockhash(recentBlockhash), instructions(instructions) {}

Message::Message(std::vector<Instruction> instructions, std::optional<PublicKey> payer)
{
  Message::newWithBlockhash(instructions, payer, Hash());
}

Message Message::newWithNonce(
    std::vector<Instruction> instructions,
    std::optional<PublicKey> payer,
    PublicKey &nonceAccountPublicKey,
    PublicKey &nonceAuthorityPublicKey)
{
  // TODO: add advance nonce account instruction
  return Message(instructions, payer);
}

Message Message::newWithBlockhash(std::vector<Instruction> instructions, std::optional<PublicKey> payer, Hash blockhash)
{
  CompiledKeys compiledKeys = CompiledKeys::compile(instructions, payer);
  MessageHeader header;
  std::vector<PublicKey> accountKeys;
  std::tie(header, accountKeys) = compiledKeys.tryIntoMessageComponents();

  std::vector<CompiledInstruction> ixs = compileInstructions(instructions, accountKeys);

  return Message::newWithCompiledInstructions(
      header.numRequiredSignatures,
      header.numReadonlySignedAccounts,
      header.numReadonlyUnsignedAccounts,
      accountKeys,
      blockhash,
      ixs);
}

Message Message::newWithCompiledInstructions(
    uint8_t numRequiredSignatures,
    uint8_t numReadonlySignedAccounts,
    uint8_t numReadonlyUnsignedAccounts,
    std::vector<PublicKey> accountKeys,
    Hash recentBlockhash,
    std::vector<CompiledInstruction> instructions)
{
  MessageHeader header = MessageHeader{
      numRequiredSignatures,
      numReadonlySignedAccounts,
      numReadonlyUnsignedAccounts};

  Message message = Message{
      header,
      accountKeys,
      recentBlockhash,
      instructions};
  return message;
}

CompiledInstruction Message::compileInstruction(Instruction &ix)
{
  return compileIx(ix, this->accountKeys);
}

std::optional<PublicKey *> Message::programId(size_t instructionIndex)
{
  if (instructionIndex >= instructions.size())
  {
    return std::nullopt; // equivalent to Rust's None
  }
  size_t programIdIndex = instructions[instructionIndex].programIdIndex;
  if (programIdIndex >= accountKeys.size())
  {
    return std::nullopt; // equivalent to Rust's None
  }
  return &accountKeys[programIdIndex];
}

std::optional<size_t> Message::programIndex(size_t instruction_index)
{
  if (instruction_index >= instructions.size())
  {
    return std::nullopt;
  }
  return instructions[instruction_index].programIdIndex;
}

std::vector<PublicKey *> Message::programIds()
{
  std::vector<PublicKey *> ids;
  for (const auto &ix : instructions)
  {
    ids.push_back(&accountKeys[ix.programIdIndex]);
  }
  return ids;
}

bool Message::isKeyPassedToProgram(size_t keyIndex)
{
  if (keyIndex > std::numeric_limits<uint8_t>::max())
  {
    return false;
  }
  uint8_t keyIndexU8 = static_cast<uint8_t>(keyIndex);
  for (const auto &ix : instructions)
  {
    if (std::find(ix.accounts.begin(), ix.accounts.end(), keyIndexU8) != ix.accounts.end())
    {
      return true;
    }
  }
  return false;
}

bool Message::isKeyCalledAsProgram(size_t keyIndex)
{
  if (keyIndex > std::numeric_limits<uint8_t>::max())
  {
    return false;
  }
  uint8_t keyIndexU8 = static_cast<uint8_t>(keyIndex);
  for (const auto &ix : instructions)
  {
    if (ix.programIdIndex == keyIndexU8)
    {
      return true;
    }
  }
  return false;
}

bool Message::isNonLoaderKey(size_t keyIndex)
{
  return !isKeyCalledAsProgram(keyIndex) || isKeyPassedToProgram(keyIndex);
}

std::optional<size_t> Message::programPosition(size_t index)
{
  auto programIds = Message::programIds();
  auto &accountKeys = this->accountKeys;
  auto it = std::find_if(
      programIds.begin(), programIds.end(), [*this, index](PublicKey *publicKey)
      { return *publicKey == this->accountKeys[index]; });
  if (it == programIds.end())
  {
    return std::nullopt;
  }
  return std::distance(programIds.begin(), it);
}

bool Message::maybeExecutable(size_t i)
{
  return programPosition(i).has_value();
}

bool Message::demoteProgramId(size_t i)
{
  return isKeyCalledAsProgram(i) && !isUpgradeableLoaderPresent();
}

bool Message::isWritable(size_t i)
{
  return (i < (header.numRequiredSignatures - header.numReadonlySignedAccounts) || (i >= header.numRequiredSignatures && i < accountKeys.size() - header.numReadonlyUnsignedAccounts)) && !isBuiltinKeyOrSysvar(accountKeys[i]) && !demoteProgramId(i);
}

bool Message::isSigner(size_t i)
{
  return i < header.numRequiredSignatures;
}

std::vector<PublicKey *> Message::signerKeys()
{
  size_t last_key = std::min(accountKeys.size(), static_cast<size_t>(header.numRequiredSignatures));
  std::vector<PublicKey *> keys;
  for (size_t i = 0; i < last_key; ++i)
  {
    keys.push_back(&accountKeys[i]);
  }
  return keys;
}

bool Message::hasDuplicates()
{
  for (size_t i = 1; i < accountKeys.size(); ++i)
  {
    if (std::find(accountKeys.begin() + i, accountKeys.end(), accountKeys[i - 1]) != accountKeys.end())
    {
      return true;
    }
  }
  return false;
}

bool Message::isUpgradeableLoaderPresent()
{
  return std::any_of(accountKeys.begin(), accountKeys.end(), [](PublicKey key)
                     { return key == BPFLoaderUpgradeable::id(); });
}

Hash Message::hashRawMessage(std::vector<uint8_t> messageBytes)
{
  Hash hash = Hash();
  std::array<uint8_t, 32> messageArr;
  std::copy(messageBytes.data(), messageBytes.data() + 32, messageArr.begin());
  return hash.newFromArray(messageArr);
}

// Serialize method for Message
std::vector<uint8_t> Message::serialize()
{
  std::vector<uint8_t> result;

  // Set transaction version byte
  // Adding support for just versioned transactions
  const uint8_t firstBit = 128;
  // TODO: get the transaction version on more standardized way
  result.push_back(firstBit);

  // Serialize header
  result.push_back(header.numRequiredSignatures);
  result.push_back(header.numReadonlySignedAccounts);
  result.push_back(header.numReadonlyUnsignedAccounts);

  // Serialize number of accounts
  uint8_t numAccounts = static_cast<uint8_t>(accountKeys.size());
  result.push_back(numAccounts);

  // Serialize accountKeys
  for (auto publicKey : accountKeys)
  {
    result.insert(result.end(), publicKey.key, publicKey.key + PUBLIC_KEY_LEN);
  }

  // Serialize recentBlockhash
  auto recentBlockhashBytes = this->recentBlockhash.serialize();
  result.insert(result.end(), recentBlockhashBytes.begin(), recentBlockhashBytes.end());

  // Serialize number of instructions
  uint8_t numInstructions = static_cast<uint8_t>(instructions.size());
  result.push_back(numInstructions);

  // Serialize instructions
  for (auto instruction : instructions)
  {
    auto instructionBytes = instruction.serialize();
    result.insert(result.end(), instructionBytes.begin(), instructionBytes.end());
  }

  // Serialize number of address table
  uint8_t numAddressTableLookups = static_cast<uint8_t>(addressTableLookups.size());
  result.push_back(numAddressTableLookups);

  for (auto addressTableLookup : addressTableLookups)
  {
    auto addressTableLookupBytes = addressTableLookup.serialize();
    result.insert(result.end(), addressTableLookupBytes.begin(), addressTableLookupBytes.end());
  }

  return result;
}

// Deserialize method for Message
Message Message::deserialize(const std::vector<uint8_t> &input)
{
  Message message;
  std::vector<uint8_t>::const_iterator it = input.begin();

  // Deserialize header
  message.header.numRequiredSignatures = *it++;
  message.header.numReadonlySignedAccounts = *it++;
  message.header.numReadonlyUnsignedAccounts = *it++;

  // Deserialize accountKeys
  while (it != input.end())
  {
    auto keyBytes = std::vector<uint8_t>(it, it + sizeof(PublicKey));
    PublicKey key = PublicKey::deserialize(keyBytes);
    message.accountKeys.push_back(key);
    it += sizeof(PublicKey);
  }

  // Deserialize recentBlockhash
  auto recentBlockhashBytes = std::vector<uint8_t>(it, it + sizeof(Hash));
  message.recentBlockhash = Hash::deserialize(recentBlockhashBytes);
  it += sizeof(Hash);

  // Deserialize instructions
  while (it != input.end())
  {
    // Extract the necessary parameters for CompiledInstruction::deserialize
    uint8_t programIdIndex = *it++;
    std::vector<uint8_t> accounts;
    while (it != input.end() && *it != 0)
    {
      accounts.push_back(*it++);
    }
    // Skip the zero byte separator
    if (it != input.end())
    {
      ++it;
    }
    std::vector<uint8_t> data(it, input.end());

    CompiledInstruction instruction = CompiledInstruction::deserialize(programIdIndex, accounts, data, input);
    message.instructions.push_back(instruction);
    it += sizeof(CompiledInstruction);
  }

  return message;
}
