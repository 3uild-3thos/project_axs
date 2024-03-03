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
  StaticJsonDocument<200> doc;
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

BlockhashWithExpiryBlockHeight Connection::getLatestBlockhash(Commitment commitment)
{
  // Create a JSON document to hold the request payload
  DynamicJsonDocument doc(256);
  JsonObject params = doc.to<JsonObject>();

  // Add the commitment parameter to the params object
  params["commitment"] = to_string(commitment);

  // Create the request payload using createRequestPayload method
  String requestPayload = createRequestPayload(34, "getLatestBlockhash", params);

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

    return blockhashWithExpiryBlockHeight;
  }
  else
  {
    // Throw an exception or handle the error as needed
    throw std::runtime_error("Request failed");
  }
}

BlockhashWithExpiryBlockHeight Connection::getLatestBlockhash()
{
  // Create a JSON document to hold the request payload
  DynamicJsonDocument doc(256);
  JsonObject params = doc.to<JsonObject>();

  // Add the commitment parameter to the params object
  params["commitment"] = to_string(commitment);

  // Create the request payload using createRequestPayload method
  String requestPayload = createRequestPayload(34, "getLatestBlockhash", params);

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

    uint64_t lastValidBlockHeight = responseDoc["result"]["value"]["lastValidBlockHeight"];

    // Construct the BlockhashWithExpiryBlockHeight object
    BlockhashWithExpiryBlockHeight blockhashWithExpiryBlockHeight;
    blockhashWithExpiryBlockHeight.blockhash = blockhash;
    blockhashWithExpiryBlockHeight.lastValidBlockHeight = lastValidBlockHeight;

    return blockhashWithExpiryBlockHeight;
  }
  else
  {
    // Throw an exception or handle the error as needed
    throw std::runtime_error("Request failed");
  }
}
