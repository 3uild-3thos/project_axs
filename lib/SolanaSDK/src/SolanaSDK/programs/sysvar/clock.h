#ifndef CLOCK_H
#define CLOCK_H

#include <optional>
#include "SolanaSDK/public_key.h"

class Clock
{
public:
  static PublicKey id()
  {
    // std::optional<PublicKey> pubkey = PublicKey::fromString("SysvarC1ock11111111111111111111111111111111");

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

#endif // CLOCK_H