#ifndef BPF_LOADER_DEPRECATED_H
#define BPF_LOADER_DEPRECATED_H

#include <optional>
#include "../public_key.h"

class BPFLoaderDeprecated
{
public:
  static PublicKey id()
  {
    // std::optional<PublicKey> pubkey = PublicKey::fromString("BPFLoader1111111111111111111111111111111111");

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

#endif // BPF_LOADER_DEPRECATED_H