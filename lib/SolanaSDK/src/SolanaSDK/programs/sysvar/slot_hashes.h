#ifndef SLOT_HASHES_H
#define SLOT_HASHES_H

#include <optional>
#include "SolanaSDK/public_key.h"

class SlotHashes
{
public:
  static PublicKey id()
  {
    // std::optional<PublicKey> pubkey = PublicKey::fromString("SysvarS1otHashes111111111111111111111111111");

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

#endif // SLOT_HASHES_H