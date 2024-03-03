#ifndef BPF_LOADER_UPGRADEABLE_H
#define BPF_LOADER_UPGRADEABLE_H

#include <optional>
#include "../public_key.h"

class BPFLoaderUpgradeable
{
public:
  static PublicKey id()
  {
    // std::optional<PublicKey> pubkey = PublicKey::fromString("BPFLoaderUpgradeab1e11111111111111111111111");

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

#endif // BPF_LOADER_UPGRADEABLE_H