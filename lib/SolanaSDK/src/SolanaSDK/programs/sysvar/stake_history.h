#ifndef STAKE_HISTORY
#define STAKE_HISTORY

#include <optional>
#include "SolanaSDK/public_key.h"

class StakeHistory
{
public:
  static PublicKey id()
  {
    // std::optional<PublicKey> pubkey = PublicKey::fromString("SysvarStakeHistory1111111111111111111111111");

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

#endif // STAKE_HISTORY