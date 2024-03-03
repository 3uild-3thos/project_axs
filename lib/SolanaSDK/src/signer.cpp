#include <sodium/crypto_sign_ed25519.h>
#include <string>
#include <vector>
#include <Arduino.h>
#include "signer.h"
#include "keypair.h"
#include "base58.h"
#include "public_key.h"
#include "signature.h"

Signer::Signer(Keypair kp)
{
  this->keypair = kp;
}

std::string Signer::sign(const std::string &message)
{
  std::vector<uint8_t> signature(crypto_sign_ed25519_BYTES);
  std::string signature_base58;

  // Sign the message using the private key
  crypto_sign_ed25519_detached(signature.data(), NULL, reinterpret_cast<const unsigned char *>(message.c_str()), message.size(), keypair.getSecretKey());

  // Convert the signature to base58
  signature_base58 = Base58::encode(signature);

  return signature_base58;
}

PublicKey Signer::publicKey()
{
  return this->keypair.publicKey;
}

Signature Signer::signMessage(const std::vector<uint8_t> &message)
{
  std::string message_str(message.begin(), message.end());
  std::string sig = this->sign(message_str);
  return Signature::fromString(sig);
}

Signers::Signers(std::vector<Signer> &signers)
{
  this->signers = signers;
}

std::vector<PublicKey> Signers::publicKeys()
{
  std::vector<PublicKey> keys;
  for (auto signer : this->signers)
  {
    keys.push_back(signer.publicKey());
  }
  return keys;
}

std::vector<Signature> Signers::signMessage(const std::vector<uint8_t> &message)
{
  std::vector<Signature> signatures;
  for (auto signer : this->signers)
  {
    Signature sig = signer.signMessage(message);
    sig.verify(signer.publicKey().serialize(), message);
    signatures.push_back(sig);
  }
  return signatures;
}