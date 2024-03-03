#include <map>
#include <vector>
#include <optional>
#include <stdexcept>
#include <variant>
#include "public_key.h"
#include "instruction.h"
#include "message.h"
#include "compiled_keys.h"

// Compiles the public keys referenced by a list of instructions and organizes
// by signer/non-signer and writable/readonly
CompiledKeys CompiledKeys::compile(std::vector<Instruction> &instructions, std::optional<PublicKey> &payer)
{
  std::map<PublicKey, CompiledKeyMeta> keyMetaMap;

  for (const Instruction &ix : instructions)
  {
    CompiledKeyMeta &meta = keyMetaMap[ix.programId];
    meta.isInvoked = true;

    for (const AccountMeta &accountMeta : ix.accounts)
    {
      CompiledKeyMeta &meta = keyMetaMap[accountMeta.publicKey];
      meta.isSigner |= accountMeta.isSigner;
      meta.isWritable |= accountMeta.isWritable;
    }
  }
  if (payer.has_value())
  {
    CompiledKeyMeta &meta = keyMetaMap[*payer];
    meta.isSigner = true;
    meta.isWritable = true;
  }

  return CompiledKeys{
      payer,
      keyMetaMap};
}

std::pair<MessageHeader, std::vector<PublicKey>> CompiledKeys::tryIntoMessageComponents()
{
  if (payer.has_value())
  {
    keyMetaMap.erase(payer.value());
  }

  std::vector<PublicKey> writableSignerKeys;
  std::vector<PublicKey> readonlySignerKeys;
  std::vector<PublicKey> writableNonSignerKeys;
  std::vector<PublicKey> readonlyNonSignerKeys;

  for (const auto &[key, meta] : keyMetaMap)
  {
    if (meta.isSigner && meta.isWritable)
    {
      writableSignerKeys.push_back(key);
    }
    else if (meta.isSigner && !meta.isWritable)
    {
      readonlySignerKeys.push_back(key);
    }
    else if (!meta.isSigner && meta.isWritable)
    {
      writableNonSignerKeys.push_back(key);
    }
    else
    {
      readonlyNonSignerKeys.push_back(key);
    }
  }

  if (payer.has_value())
  {
    writableSignerKeys.insert(writableSignerKeys.begin(), payer.value());
  }

  size_t signersLen = writableSignerKeys.size() + readonlySignerKeys.size();

  if (signersLen > 255 || readonlySignerKeys.size() > 255 || readonlyNonSignerKeys.size() > 255)
  {
    throw CompileError("AccountIndexOverflow");
  }

  MessageHeader header = {
      static_cast<uint8_t>(signersLen),
      static_cast<uint8_t>(readonlySignerKeys.size()),
      static_cast<uint8_t>(readonlyNonSignerKeys.size()),
  };

  std::vector<PublicKey> staticAccountKeys;
  staticAccountKeys.insert(staticAccountKeys.end(), writableSignerKeys.begin(), writableSignerKeys.end());
  staticAccountKeys.insert(staticAccountKeys.end(), readonlySignerKeys.begin(), readonlySignerKeys.end());
  staticAccountKeys.insert(staticAccountKeys.end(), writableNonSignerKeys.begin(), writableNonSignerKeys.end());
  staticAccountKeys.insert(staticAccountKeys.end(), readonlyNonSignerKeys.begin(), readonlyNonSignerKeys.end());

  return {header, staticAccountKeys};
}
