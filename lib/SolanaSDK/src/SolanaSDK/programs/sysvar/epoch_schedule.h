#ifndef EPOCH_SCHEDULE_H
#define EPOCH_SCHEDULE_H

#include <optional>
#include "SolanaSDK/public_key.h"

class EpochSchedule
{
public:
  static PublicKey id()
  {
    // std::optional<PublicKey> pubkey = PublicKey::fromString("SysvarEpochSchedu1e111111111111111111111111");

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

#endif // EPOCH_SCHEDULE_H