#include <string>
#include <ArduinoJson.h>
#include "connection.h"
#include "hash.h"
#include "base58.h"
#include "send_request.h"

std::string to_string(Commitment commitment)
{
  switch (commitment)
  {
  case Commitment::processed:
    return "processed";
  case Commitment::confirmed:
    return "confirmed";
  case Commitment::finalized:
    return "finalized";
  }
  return ""; // Default case, should not be reached
}

String Connection::createRequestPayload(uint16_t id, const std::string &method, JsonObject &additionalParams)
{
  StaticJsonDocument<1024> doc;
  doc["id"] = id;
  doc["jsonrpc"] = "2.0";
  doc["method"] = method.c_str();

  // Create a JsonArray for params
  JsonArray paramsArray = doc.createNestedArray("params");
  paramsArray.add(additionalParams); // Add the JsonObject as an element of the array

  String requestPayload;
  serializeJson(doc, requestPayload);
  Serial.println(requestPayload);
  return requestPayload;
}

String Connection::createRequestPayload(uint16_t id, const std::string &method, JsonArray &additionalParams)
{
  StaticJsonDocument<1024> doc;
  doc["id"] = id;
  doc["jsonrpc"] = "2.0";
  doc["method"] = method.c_str();

  // Create a JsonArray for params
  doc["params"] = additionalParams;

  String requestPayload;
  serializeJson(doc, requestPayload);
  Serial.println(requestPayload);
  return requestPayload;
}

Connection::Connection(std::string endpoint, Commitment commitment)
{
  this->rpcEndpoint = endpoint;
  this->commitment = commitment;
}

Connection::Connection(std::string endpoint)
{
  this->rpcEndpoint = endpoint;
  this->commitment = Commitment::processed;
}

BlockhashWithExpiryBlockHeight Connection::_getLatestBlockhash(Commitment commitment)
{
  // Create a JSON document to hold the request payload
  DynamicJsonDocument doc(256);
  JsonObject params = doc.to<JsonObject>();

  // Add the commitment parameter to the params object
  params["commitment"] = to_string(commitment);

  // Create the request payload using createRequestPayload method
  String requestPayload = createRequestPayload(1, "getLatestBlockhash", params);

  // Send the HTTP request and get the response
  String response;
  if (sendHttpRequest(rpcEndpoint.c_str(), requestPayload, response))
  {
    // Parse the JSON response
    DynamicJsonDocument responseDoc(512); // Adjust capacity as needed
    deserializeJson(responseDoc, response);

    // Extract the blockhash string from the response
    const char *blockhashString = responseDoc["result"]["value"]["blockhash"];

    // Decode the blockhash string using Base58 decoding
    std::vector<uint8_t> recentBlockhashBytes = Base58::trimDecode(blockhashString);

    // Construct the Hash object from the decoded bytes
    Hash blockhash = Hash(recentBlockhashBytes);

    // Construct the BlockhashWithExpiryBlockHeight object
    BlockhashWithExpiryBlockHeight blockhashWithExpiryBlockHeight;
    blockhashWithExpiryBlockHeight.blockhash = blockhash;

    // Check if the response contains lastValidBlockHeight
    if (responseDoc["result"]["value"].containsKey("lastValidBlockHeight"))
    {
      uint64_t lastValidBlockHeight = responseDoc["result"]["value"]["lastValidBlockHeight"];
      blockhashWithExpiryBlockHeight.lastValidBlockHeight = lastValidBlockHeight;
    }

    return blockhashWithExpiryBlockHeight;
  }
  else
  {
    // Throw an exception or handle the error as needed
    throw std::runtime_error("Request failed");
  }
}

BlockhashWithExpiryBlockHeight Connection::getLatestBlockhash(Commitment commitment)
{
  return _getLatestBlockhash(commitment);
}

BlockhashWithExpiryBlockHeight Connection::getLatestBlockhash()
{
  return _getLatestBlockhash(commitment);
}

Signature Connection::_sendTransaction(Transaction transaction, SendOptions sendOptions)
{
  // Create a JSON document to hold the request payload
  DynamicJsonDocument doc(512);

  // Create the params array
  JsonArray params = doc.createNestedArray("params");

  // Serialize transaction and add it to the params array
  String transactionSerialized = Base58::trimEncode(transaction.serialize()).c_str();
  params.add(transactionSerialized);

  // Create options object and add parameters
  JsonObject options = doc.template add<JsonObject>();
  options["encoding"] = "base58";
  options["skipPreflight"] = sendOptions.skipPreflight;
  options["preflightCommitment"] = to_string(sendOptions.preflightCommitment);
  options["maxRetries"] = sendOptions.maxRetires;

  // Add options object to the params array
  params.add(options);

  std::string encoding = params[1]["encoding"];
  Serial.println(encoding.c_str());

  bool skipPreflight = params[1]["skipPreflight"];
  Serial.println(skipPreflight);

  std::string preflightCommitment = params[1]["preflightCommitment"];
  Serial.println(preflightCommitment.c_str());

  int maxRetries = params[1]["maxRetries"];
  Serial.println(maxRetries);

  // Create the request payload using createRequestPayload method
  String requestPayload = createRequestPayload(1, "sendTransaction", params);

  // Send the HTTP request and get the response
  String response;
  if (sendHttpRequest(rpcEndpoint.c_str(), requestPayload, response))
  {
    // Parse the JSON response
    DynamicJsonDocument responseDoc(128); // Adjust capacity as needed
    deserializeJson(responseDoc, response);

    // Extract the signature string from the response
    const char *signatureString = responseDoc["result"];

    // Decode the signature string using Base58 decoding
    std::vector<uint8_t> signatureBytes = Base58::trimDecode(signatureString);

    // Deserialize the signature
    Signature signature = Signature::deserialize(signatureBytes);

    return signature;
  }
  else
  {
    // Throw an exception or handle the error as needed
    throw std::runtime_error("Request failed");
  }
}

Signature Connection::sendTransaction(Transaction transaction, SendOptions sendOptions)
{
  return _sendTransaction(transaction, sendOptions);
}

Signature Connection::sendTransaction(Transaction transaction)
{
  SendOptions defaultSendOptions;
  return _sendTransaction(transaction, defaultSendOptions);
}