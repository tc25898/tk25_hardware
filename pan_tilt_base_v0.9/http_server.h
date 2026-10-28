#include "web_page.h"

// Create WebServer object on port 80
WebServer server(80);

void handleRoot(){
  server.send(200, "text/html", index_html); //Send web page
}

void webCtrlServer(){
  server.on("/", handleRoot);

  server.on("/js", [](){
    String jsonCmdWebString = server.arg(0);
    jsonCmdReceive.clear();
    if (deserializeJson(jsonCmdReceive, jsonCmdWebString) || !jsonCmdReceive.is<JsonObject>()) {
      jsonCmdReceive.clear();
      server.send(400, "application/json", "{\"error\":\"Invalid JSON object\"}");
      return;
    }
    jsonCmdReceiveHandler();
    serializeJson(jsonInfoHttp, jsonFeedbackWeb);
    server.send(200, "application/json", jsonFeedbackWeb);
    jsonFeedbackWeb = "";
    jsonInfoHttp.clear();
    jsonCmdReceive.clear();
  });

  server.on("/host/wifi", HTTP_POST, []() {
    server.sendHeader("Cache-Control", "no-store");
    String body = server.arg("plain");
    if (body.length() >= UART_FRAME_BYTES) {
      server.send(413, "application/json", "{\"error\":\"Request too large\"}");
      return;
    }
    jsonCmdReceive.clear();
    DeserializationError error = deserializeJson(jsonCmdReceive, body);
    const char* invalid = error || !jsonCmdReceive.is<JsonObject>()
        ? "Invalid or oversized JSON object" : validateHostWifiRequest(jsonCmdReceive);
    if (invalid) {
      StaticJsonDocument<JSON_OBJECT_SIZE(1)> response;
      response["error"] = invalid;
      String output;
      serializeJson(response, output);
      server.send(400, "application/json", output);
    } else {
      sendHostWifiRequest(jsonCmdReceive);
      server.send(202, "application/json", "{\"state\":\"sent\"}");
    }
    jsonCmdReceive.clear();
  });

  server.on("/host/wifi/status", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    StaticJsonDocument<JSON_OBJECT_SIZE(2)> response;
    response["state"] = hostWifiReplyReceived ? "reply" : hostWifiRequestSent ? "sent" : "idle";
    response["result"] = static_cast<const char*>(hostWifiResult);
    String output;
    serializeJson(response, output);
    server.send(200, "application/json", output);
  });

  // Start server
  server.begin();
  Serial.println("Server Starts.");
}

void initHttpWebServer(){
  webCtrlServer();
}