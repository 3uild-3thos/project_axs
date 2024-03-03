#ifndef COMPILED_KEYS_H
#define COMPILED_KEYS_H

#include <map>
#include <vector>
#include <optional>
#include "public_key.h"
#include "instruction.h"
#include "message.h"

struct CompiledKeyMeta
{
  bool isSigner = false;
  bool isWritable = false;
  bool isInvoked = false;
};

using CompileError = std::runtime_error;

class CompiledKeys
{
public:
  std::optional<PublicKey> payer;
  std::map<PublicKey, CompiledKeyMeta> keyMetaMap;

  static CompiledKeys compile(std::vector<Instruction> &instructions, std::optional<PublicKey> &payer);

  std::pair<MessageHeader, std::vector<PublicKey>> tryIntoMessageComponents();
};

#endif // COMPILED_KEYS_H
