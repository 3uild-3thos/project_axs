#include <WiFi.h>
#include <HTTPClient.h>

bool sendHttpRequest(const char *url, const String &requestData, String &response)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(url);

    int httpResponseCode = http.POST(requestData);

    if (httpResponseCode > 0)
    {
      response = http.getString();
      http.end();
      return true;
    }
    else
    {
      Serial.print("HTTP error code: ");
      Serial.println(httpResponseCode);
      http.end();
      return false;
    }
  }
  else
  {
    Serial.println("WiFi not connected");
    return false;
  }
}
