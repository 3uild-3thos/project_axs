#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstdint>
#include <vector>
#include <sstream>
#include "public_key.h"
#include "hash.h"
#include "instruction.h"
#include "address_lookup_table.h"

struct MessageHeader
{
  uint8_t numRequiredSignatures;
  uint8_t numReadonlySignedAccounts;
  uint8_t numReadonlyUnsignedAccounts;
};

class Message
{
public:
  // THe message header, identifying signed and read-only accountKeys
  MessageHeader header;

  // All the account keys used by the transaction
  std::vector<PublicKey> accountKeys;

  // The id of a recent ledger entry
  Hash recentBlockhash;

  // Programs the will be executed in sequence and committed in
  // one atomic transaction if all succeed
  std::vector<CompiledInstruction> instructions;

  std::vector<AddressLookupTable> addressTableLookups;

  void sanitize();

  Message() = default;

  Message(MessageHeader header, std::vector<PublicKey> accountKeys, Hash recentBlockhash, std::vector<CompiledInstruction> instructions);

  Message(std::vector<Instruction> instructions, std::optional<PublicKey> payer);

  static Message newWithBlockhash(std::vector<Instruction> instructions, std::optional<PublicKey> payer, Hash blockhash);

  Message newWithNonce(
      std::vector<Instruction> instructions,
      std::optional<PublicKey> payer,
      PublicKey &nonceAccountPublicKey,
      PublicKey &nonceAuthorityPublicKey);

  static Message newWithCompiledInstructions(
      uint8_t numRequiredSignatures,
      uint8_t numReadonlySignedAccounts,
      uint8_t numReadonlyUnsignedAccounts,
      std::vector<PublicKey> accountKeys,
      Hash recentBlockhash,
      std::vector<CompiledInstruction> instructions);

  CompiledInstruction compileInstruction(Instruction &ix);

  std::optional<PublicKey *> programId(size_t instructionIndex);

  std::optional<size_t> programIndex(size_t instruction_index);

  std::vector<PublicKey *> programIds();

  bool isKeyPassedToProgram(size_t keyIndex);

  bool isKeyCalledAsProgram(size_t keyIndex);

  bool isNonLoaderKey(size_t keyIndex);

  std::optional<size_t> programPosition(size_t index);

  bool maybeExecutable(size_t i);

  bool demoteProgramId(size_t i);

  bool isWritable(size_t i);

  bool isSigner(size_t i);

  std::vector<PublicKey *> signerKeys();

  bool hasDuplicates();

  bool isUpgradeableLoaderPresent();

  static Hash hashRawMessage(std::vector<uint8_t> messageBytes);

  std::vector<uint8_t> serialize();

  static Message deserialize(const std::vector<uint8_t> &input);
};

#endif // MESSAGE_H
