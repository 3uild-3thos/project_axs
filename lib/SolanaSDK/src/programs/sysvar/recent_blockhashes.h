#ifndef RECENT_BLOCKHASHES_H
#define RECENT_BLOCKHASHES_H

#include <optional>
#include "solana/public_key.h"

class RecentBlockhashes
{
public:
  static PublicKey id()
  {
    // std::optional<PublicKey> pubkey = PublicKey::fromString("SysvarRecentB1ockHashes11111111111111111111");

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

#endif // RECENT_BLOCKHASHES_H