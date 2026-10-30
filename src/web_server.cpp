#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include "web_server.h"

namespace {

constexpr char WEB_AP_SSID[] = "T-Rex-Console";
WebServer server(80);

const char HELLO_PAGE[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>T-Rex Console</title>
</head>
<body>
  <h1>Hello, world!</h1>
</body>
</html>
)rawliteral";

} // namespace

void startWebServer() {
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(WEB_AP_SSID)) {
        Serial.println("Failed to start Wi-Fi access point");
        return;
    }

    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html; charset=utf-8", HELLO_PAGE);
    });
    server.onNotFound([]() {
        server.send(404, "text/plain; charset=utf-8", "Not found");
    });
    server.begin();

    Serial.print("Wi-Fi AP: ");
    Serial.println(WEB_AP_SSID);
    Serial.print("Open http://");
    Serial.println(WiFi.softAPIP());
}

void handleWebServer() {
    server.handleClient();
}
