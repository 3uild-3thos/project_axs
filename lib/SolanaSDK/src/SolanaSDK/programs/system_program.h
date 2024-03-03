#ifndef SYSTEM_PROGRAM_H
#define SYSTEM_PROGRAM_H

#include <optional>
#include "../public_key.h"

class SystemProgram
{
public:
  static PublicKey id()
  {
    std::optional<PublicKey> pubkey = PublicKey::fromString("11111111111111111111111111111111");

    if (pubkey.has_value())
    {
      return pubkey.value();
    }
    else
    {
      return PublicKey();
    }
  }
};

#endif // SYSTEM_PROGRAM_H