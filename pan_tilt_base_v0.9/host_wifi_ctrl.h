#pragma once

// Latest Host message, shared by all browsers; never contains submitted credentials.
char hostWifiResult[UART_FRAME_BYTES] = {};
bool hostWifiRequestSent = false;
bool hostWifiReplyReceived = false;

bool isHostWifiMessage(JsonDocument& message) {
  JsonVariant type = message["T"];
  return (type.is<int>() || type.is<double>()) &&
         type.as<double>() == CMD_HOST_WIFI_CONFIGURE;
}

// Called only for UART input. Consume management messages before command echo
// and vendor dispatch so replies can never be sent back as requests.
bool handleHostWifiReply(JsonDocument& message) {
  if (!isHostWifiMessage(message)) return false;
  if (message["result"].is<const char*>()) {
    JsonString result = message["result"].as<JsonString>();
    if (result.size() < sizeof(hostWifiResult) &&
        strlen(result.c_str()) == result.size()) {
      memcpy(hostWifiResult, result.c_str(), result.size() + 1);
      hostWifiReplyReceived = true;
    }
  }
  return true;
}

const char* validateHostWifiRequest(JsonDocument& message) {
  if (!isHostWifiMessage(message) || message.containsKey("result") ||
      !message["ssid"].is<const char*>() ||
      !message["password"].is<const char*>())
    return "Expected T:-1001, ssid and password strings";

  JsonString ssid = message["ssid"].as<JsonString>();
  JsonString password = message["password"].as<JsonString>();
  if (ssid.size() < 1 || ssid.size() > 32 || strlen(ssid.c_str()) != ssid.size())
    return "SSID must contain 1..32 UTF-8 bytes without NUL";
  bool hexPsk = password.size() == 64;
  for (size_t i = 0; i < password.size(); ++i) {
    unsigned char c = password.c_str()[i];
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
          (c >= 'A' && c <= 'F'))) hexPsk = false;
    if (c < 0x20 || c > 0x7e)
      return "Password must contain printable ASCII only";
  }
  if (!hexPsk && (password.size() < 8 || password.size() > 63))
    return "Password must be 8..63 ASCII characters or a 64-digit hex PSK";
  return nullptr;
}

void sendHostWifiRequest(JsonDocument& message) {
  // Rebuild only the registered fields. Never echo or persist credentials.
  StaticJsonDocument<JSON_OBJECT_SIZE(3)> request;
  request["T"] = CMD_HOST_WIFI_CONFIGURE;
  request["ssid"] = message["ssid"].as<const char*>();
  request["password"] = message["password"].as<const char*>();
  hostWifiResult[0] = '\0';
  hostWifiReplyReceived = false;
  hostWifiRequestSent = true;
  serializeJson(request, Serial);
  Serial.write('\n');
}
