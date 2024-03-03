#ifndef ACCOUNT_META_H
#define ACCOUNT_META_H

#include "public_key.h"

class AccountMeta
{
public:
  // An account's public key
  PublicKey publicKey;

  // True if an Instruction required a Transaction signature matching public_key
  bool isSigner;

  // True if the account data or metadata maybe mutated during program execution
  bool isWritable;

  // Construct metadata for a writable account.
  static AccountMeta *newWritable(PublicKey publicKey, bool isSigner);

  // Construct metadata for a read-only account.
  static AccountMeta *newReadonly(PublicKey publicKey, bool isSigner);

  std::vector<uint8_t> serialize();

  static AccountMeta deserialize(const std::vector<uint8_t> &input);
};

#endif // ACCOUNT_META_H
