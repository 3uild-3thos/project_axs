#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <optional>
#include "SolanaSDK/public_key.h"

class Instructions
{
public:
  static PublicKey id()
  {
    // std::optional<PublicKey> pubkey = PublicKey::fromString("Sysvar1nstructions1111111111111111111111111");

    // if (pubkey.has_value())
    // {
    //   return pubkey.value();
    // }
    // else
    // {
    // }
    return PublicKey();
  }
};

#endif // INSTRUCTION_H