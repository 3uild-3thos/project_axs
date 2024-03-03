#ifndef REWARDS_H
#define REWARDS_H

#include <optional>
#include "solana/public_key.h"

class Rewards
{
public:
  static PublicKey id()
  {
    // std::optional<PublicKey> pubkey = PublicKey::fromString("SysvarRewards111111111111111111111111111111");

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

#endif // REWARDS_H