#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <ArduinoJson.h>
#include "hash.h"
#include "signature.h"
#include "transaction.h"

enum class Commitment
{
  processed,
  confirmed,
  finalized
};

// Overload the std::string conversion operator for Commitment enum class
std::string to_string(Commitment commitment);

struct SendOptions
{
  bool skipPreflight = false;
  Commitment preflightCommitment = Commitment::confirmed;
  int maxRetires = 5;
};

struct BlockhashWithExpiryBlockHeight
{
  Hash blockhash;
  uint64_t lastValidBlockHeight;
};

// Partial implementation of Connection
// reference from web3.js
class Connection
{
private:
  Commitment commitment;
  std::string rpcEndpoint;
  String createRequestPayload(uint16_t id, const std::string &method, JsonObject &additionalParams);
  String createRequestPayload(uint16_t id, const std::string &method, JsonArray &additionalParams);
  // TODO: Add proper commitment or config args
  BlockhashWithExpiryBlockHeight _getLatestBlockhash(Commitment commitment);
  // TODO: Add proper signer arg and
  Signature _sendTransaction(Transaction transaction, SendOptions sendOptions);

public:
  Connection(std::string endpoint, Commitment commitment);
  Connection(std::string endpoint);
  BlockhashWithExpiryBlockHeight getLatestBlockhash(Commitment commitment);
  BlockhashWithExpiryBlockHeight getLatestBlockhash();
  Signature sendTransaction(Transaction transaction, SendOptions sendOptions);
  Signature sendTransaction(Transaction transaction);
};

#endif // CONNECTION_H