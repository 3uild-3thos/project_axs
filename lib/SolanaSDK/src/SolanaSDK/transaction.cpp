#include <vector>
#include <optional>
#include <utility>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include "transaction.h"
#include "signature.h"
#include "message.h"
#include "instruction.h"
#include "compiled_keys.h"
#include "signer.h"

// Create an unsigned transaction from a Message.
Transaction::Transaction(Message message)
    : message(message), signatures(message.header.numRequiredSignatures, Signature()) {}

void Transaction::sanitize()
{
  if (this->message.header.numRequiredSignatures > this->signatures.size())
  {
    throw std::out_of_range("Number of required signatures exceeds the number of signatures");
  }
  if (this->signatures.size() > this->message.accountKeys.size())
  {
    throw std::out_of_range("Number of signatures exceeds the number of account keys");
  }
  this->message.sanitize();
}

// Create an unsigned transaction from a Message.
Transaction Transaction::newUnsigned(Message message)
{
  return Transaction(message);
}

// Create a fully-signed transaction from a Message
Transaction Transaction::create(std::vector<Signer> &fromKeypairs, Message message, Hash recent_blockhash)
{
  Transaction tx = this->newUnsigned(message);
  // TODO: add sign
  return tx;
}

// Create an unsigned transaction from a list of Instructions.
Transaction Transaction::newWithPayer(std::vector<Instruction> &instructions, std::optional<PublicKey> &payer)
{
  Message message = Message(instructions, payer);
  return this->newUnsigned(message);
}

// Create a fully-signed transaction from a list of Instructions
Transaction Transaction::createSignedWithPayer(std::vector<Instruction> &instructions, std::optional<PublicKey> &payer, std::vector<Signer> &signingKeypairs, Hash recentBlockhash)
{
  Message message = Message(instructions, payer);
  return this->create(signingKeypairs, message, recentBlockhash);
}

// Create a fully-signed transaction from pre-compiled instructions.
Transaction Transaction::createWithCompiledInstructions(std::vector<Signer> &fromKeypairs, std::vector<PublicKey> &keys, Hash recentBlockhash, std::vector<PublicKey> programIds, std::vector<CompiledInstruction> instructions)
{
  std::vector<PublicKey> accountKeys;

  for (auto keypair : fromKeypairs)
  {
    accountKeys.push_back(keypair.publicKey());
  }
  size_t fromKeypairsLen = accountKeys.size();

  for (auto key : keys)
  {
    accountKeys.push_back(key);
  }

  for (auto programId : programIds)
  {
    accountKeys.push_back(programId);
  }

  Message message = Message::newWithCompiledInstructions(fromKeypairsLen, 0, programIds.size(), accountKeys, Hash(), instructions);
  return Transaction::create(fromKeypairs, message, recentBlockhash);
}

// Get the data for an instruction at the given index.
std::vector<uint8_t> Transaction::data(size_t instructionIndex)
{
  return this->message.instructions[instructionIndex].data;
}

std::optional<size_t> Transaction::keyIndex(size_t instructionIndex, size_t accountsIndex)
{
  CompiledInstruction ix = this->message.instructions.at(instructionIndex);
  size_t accountKeysIndex = ix.accounts.at(accountsIndex);
  return accountKeysIndex;
}

// Get the PublicKey of an account required by one of the instructions in
// the transaction.
std::optional<PublicKey> Transaction::key(size_t instructionIndex, size_t accountsIndex)
{
  std::optional<size_t> accountKeysIndex = this->keyIndex(instructionIndex, accountsIndex);
  return this->message.accountKeys.at(accountKeysIndex.value());
}

// Get the PublicKey of a signing account required by one of the
// instructions in the transaction.
std::optional<PublicKey> Transaction::signerKey(size_t instructionIndex, size_t accountsIndex)
{
  if (this->keyIndex(instructionIndex, accountsIndex).has_value())
  {
    size_t signatureIndex = this->keyIndex(instructionIndex, accountsIndex).value();
    if (signatureIndex >= this->signatures.size())
    {
      return std::nullopt;
    }
    return this->message.accountKeys.at(signatureIndex);
  }
  else
  {
    return std::nullopt;
  }
}

// Return the message containing all data that should be signed.
Message Transaction::getMessage()
{
  return this->message;
}

// Return the serialized message data to sign.
std::vector<uint8_t> Transaction::messageData()
{
  return this->getMessage().serialize();
}

// Sign the transaction.
Transaction Transaction::sign(Signers &keypairs, Hash recentBlockhash)
{
  try
  {
    this->trySign(keypairs, recentBlockhash);
  }
  catch (std::exception &e)
  {
    throw std::runtime_error("Transaction::sign failed with error " + std::string(e.what()));
  }
  return *this;
}

// Sign the transaction with a subset of required keys.
void Transaction::partialSign(Signers &keypairs, Hash recentBlockhash)
{
  try
  {
    this->tryPartialSign(keypairs, recentBlockhash);
  }
  catch (std::exception &e)
  {
    throw std::runtime_error("Transaction::partialSign failed with error " + std::string(e.what()));
  }
}

// Sign the transaction, returning any errors.
void Transaction::trySign(Signers &keypairs, Hash recentBlockhash)
{
  this->tryPartialSign(keypairs, recentBlockhash);

  if (!this->isSigned())
  {
    throw std::runtime_error("Not enough signers");
  }
}

// Sign the transaction with a subset of required keys, returning any errors.
void Transaction::tryPartialSign(Signers &keypairs, Hash recentBlockhash)
{
  std::vector<PublicKey> keys = keypairs.publicKeys();
  std::vector<std::optional<size_t>> positions = this->getSigningKeypairPositions(keys);
  std::vector<size_t> positionsVec;
  for (auto pos : positions)
  {
    if (!pos.has_value())
    {
      throw std::runtime_error("Keypair public key mismatch");
    }
    positionsVec.push_back(pos.value());
  }
  this->tryPartialSignUnchecked(keypairs, positionsVec, recentBlockhash);
}

// Sign the transaction with a subset of required keys, returning any
// errors.
void Transaction::tryPartialSignUnchecked(Signers &keypairs, std::vector<size_t> positions, Hash recentBlockhash)
{
  //  if you change the blockhash, you're re-signing...
  if (recentBlockhash != this->message.recentBlockhash)
  {
    this->message.recentBlockhash = recentBlockhash;

    for (auto &signature : this->signatures)
    {
      signature = Signature();
    }
  }

  std::vector<Signature> signatures = keypairs.signMessage(this->messageData());

  if (signatures.size() > 0)
  {
    for (int i = 0; i < positions.size(); i++)
    {
      this->signatures[positions[i]] = signatures[i];
    }
  }
}

// Get the positions of the public keys in accountKeys associated with signing keypairs.
std::vector<std::optional<size_t>> Transaction::getSigningKeypairPositions(std::vector<PublicKey> &publicKeys)
{
  if (this->message.accountKeys.size() < this->message.header.numRequiredSignatures)
  {
    throw std::runtime_error("Invalid account index");
  }

  std::vector<PublicKey> signedKeys(this->message.accountKeys.begin(), this->message.accountKeys.begin() + this->message.header.numRequiredSignatures);

  std::vector<std::optional<size_t>> positions;
  for (const auto &publicKey : publicKeys)
  {
    auto it = std::find(signedKeys.begin(), signedKeys.end(), publicKey);
    positions.push_back(it != signedKeys.end() ? std::optional<size_t>(std::distance(signedKeys.begin(), it)) : std::nullopt);
  }

  return positions;
}

bool Transaction::isSigned()
{
  for (auto &signature : this->signatures)
  {
    if (signature != Signature())
    {
      return true;
    }
  }

  return false;
}

// Returns a signature that is not valid for signing this transaction.
Signature Transaction::getInvalidSignature()
{
  return Signature();
}

// Verifies that all signers have signed the message.
void Transaction::verify()
{
  std::vector<uint8_t> messageBytes = this->messageData();
  std::vector<bool> verifyResults = this->_verifyWithResults(messageBytes);
  for (auto verifyResult : verifyResults)
  {
    if (!verifyResult)
    {
      throw std::runtime_error("Signature failure");
    }
  }
}

// Verify the transaction and hash its message.
Hash Transaction::verifyAndHashMessage()
{
  std::vector<uint8_t> messageData = this->messageData();
  std::vector<bool> verifyResults = this->_verifyWithResults(messageData);

  if (std::all_of(verifyResults.begin(), verifyResults.end(), [](bool v)
                  { return v; }))
  {
    return Message::hashRawMessage(messageData);
  }
  else
  {
    throw std::runtime_error("Signature failure");
  }
}

// Verifies that all signers have signed the message.
std::vector<bool> Transaction::verifyWithResults()
{
  return this->_verifyWithResults(this->messageData());
}

std::vector<bool> Transaction::_verifyWithResults(const std::vector<uint8_t> &messageBytes)
{
  std::vector<PublicKey> publicKeys = this->message.accountKeys;
  if (this->signatures.size() != publicKeys.size())
  {
    throw std::runtime_error("Mismatch between signatures and public keys");
  }

  std::vector<bool> results;
  for (size_t i = 0; i < this->signatures.size(); ++i)
  {
    try
    {
      std::vector<uint8_t> pubkeyBytes(publicKeys[i].key, publicKeys[i].key + PUBLIC_KEY_LEN);
      this->signatures[i].verify(pubkeyBytes, messageBytes);
      results.push_back(true);
    }
    catch (const std::runtime_error &e)
    {
      results.push_back(false);
    }
  }
  return results;
}

// Serialize method
std::vector<uint8_t> Transaction::serialize()
{
  std::vector<uint8_t> serializedTransaction;

  // Serialize signatures length
  uint8_t numSignatures = static_cast<uint8_t>(this->signatures.size());
  serializedTransaction.push_back(numSignatures);

  // Serialize each signature
  for (auto signature : this->signatures)
  {
    std::vector<uint8_t> serializedSignature = signature.serialize();
    serializedTransaction.insert(serializedTransaction.end(), serializedSignature.begin(), serializedSignature.end());
  }

  // Serialize message
  std::vector<uint8_t> serializedMessage = this->message.serialize();
  serializedTransaction.insert(serializedTransaction.end(), serializedMessage.begin(), serializedMessage.end());

  return serializedTransaction;
}

// Deserialize method
Transaction Transaction::deserialize(const std::vector<uint8_t> &data)
{
  size_t index = 0;

  // Deserialize message
  Message message = Message::deserialize(data);

  // Deserialize signatures length
  uint8_t numSignatures = data[index++];

  // Deserialize signatures
  std::vector<Signature> signatures;
  for (int i = 0; i < numSignatures; ++i)
  {
    Signature signature = Signature::deserialize(data);
    signatures.push_back(signature);
  }

  // TODO: Add Signatures Constructor

  return Transaction(message);
}
