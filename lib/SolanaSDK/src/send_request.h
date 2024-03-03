#ifndef SEND_REQUEST_H
#define SEND_REQUEST_H

#include <WiFi.h>
#include <HTTPClient.h>

bool sendHttpRequest(const char *url, const String &requestData, String &response);

#endif // SEND_REQUEST_H