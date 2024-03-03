#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <ArduinoJson.h>
#include "hash.h"

enum class Commitment
{
  processed,
  confirmed,
  finalized
};

// Overload the std::string conversion operator for Commitment enum class
std::string to_string(Commitment commitment);

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

public:
  Connection(std::string endpoint, Commitment commitment);
  Connection(std::string endpoint);
  // TODO: Add proper commitment or config args
  BlockhashWithExpiryBlockHeight getLatestBlockhash(Commitment commitment);
  BlockhashWithExpiryBlockHeight getLatestBlockhash();
};

#endif // CONNECTION_H