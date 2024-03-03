#ifndef SLOT_HISTORY_H
#define SLOT_HISTORY_H

#include <optional>
#include "SolanaSDK/public_key.h"

class SlotHistory
{
public:
  static PublicKey id()
  {
    // std::optional<PublicKey> pubkey = PublicKey::fromString("SysvarS1otHistory11111111111111111111111111");

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

#endif // SLOT_HISTORY_H