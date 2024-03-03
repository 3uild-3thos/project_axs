#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <vector>
#include <optional>
#include <utility>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include "signature.h"
#include "message.h"
#include "instruction.h"
#include "compiled_keys.h"
#include "signer.h"

class Transaction
{
public:
  // A set of signatures of a serialized Message, signed by the first
  // keys of the Message's accountKeys, where the number of signatures
  // is equal to numRequiredSignatures of the Message's MessageHeader
  std::vector<Signature> signatures;

  // the message to sign
  Message message;

  Transaction(Message message);
  void sanitize();

  static Transaction newUnsigned(Message message);

  Transaction create(std::vector<Signer> &fromKeypairs, Message message, Hash recent_blockhash);

  Transaction newWithPayer(std::vector<Instruction> &instructions, std::optional<PublicKey> &payer);

  Transaction createSignedWithPayer(std::vector<Instruction> &instructions, std::optional<PublicKey> &payer, std::vector<Signer> &signingKeypairs, Hash recentBlockhash);

  Transaction createWithCompiledInstructions(std::vector<Signer> &fromKeypairs, std::vector<PublicKey> &keys, Hash recentBlockhash, std::vector<PublicKey> programIds, std::vector<CompiledInstruction> instructions);

  std::vector<uint8_t> data(size_t instructionIndex);

  std::optional<size_t> keyIndex(size_t instructionIndex, size_t accountsIndex);

  std::optional<PublicKey> key(size_t instructionIndex, size_t accountsIndex);

  std::optional<PublicKey> signerKey(size_t instructionIndex, size_t accountsIndex);

  Message getMessage();

  std::vector<uint8_t> messageData();

  Transaction sign(Signers &keypairs, Hash recentBlockhash);

  void partialSign(Signers &keypairs, Hash recentBlockhash);

  void trySign(Signers &keypairs, Hash recentBlockhash);

  void tryPartialSign(Signers &keypairs, Hash recentBlockhash);

  void tryPartialSignUnchecked(Signers &keypairs, std::vector<size_t> positions, Hash recentBlockhash);

  std::vector<std::optional<size_t>> getSigningKeypairPositions(std::vector<PublicKey> &publicKeys);

  bool isSigned();

  Signature getInvalidSignature();

  void verify();

  Hash verifyAndHashMessage();

  std::vector<bool> verifyWithResults();

  std::vector<uint8_t> serialize();

  static Transaction deserialize(const std::vector<uint8_t> &data);

private:
  std::vector<bool> _verifyWithResults(const std::vector<uint8_t> &messageBytes);

  // TODO: get_nonce_pubkey_from_instruction, uses_durable_nonce,
  // replace_signatures, verify_precompiles,
};

#endif // TRANSACTION_H
