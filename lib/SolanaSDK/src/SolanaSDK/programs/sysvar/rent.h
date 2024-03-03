#ifndef RENT_H
#define RENT_H

#include <optional>
#include "SolanaSDK/public_key.h"

class Rent
{
public:
  static PublicKey id()
  {
    // std::optional<PublicKey> pubkey = PublicKey::fromString("SysvarRent111111111111111111111111111111111");

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

#endif // RENT_H