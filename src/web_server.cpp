#include <Arduino.h>
#include <Preferences.h>
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
    <style>
        body { margin: 0; min-height: 100vh; display: grid; place-items: center; background: #101820; color: #f2f5f4; font-family: sans-serif; }
        main { padding: 2rem; text-align: center; }
        h1 { margin: 0 0 1.5rem; font-size: 1.25rem; font-weight: 500; }
        p { margin: 0; color: #aab8b8; }
        output { display: block; margin-top: .5rem; color: #8ee3a1; font-size: 4rem; font-variant-numeric: tabular-nums; }
        #status { margin-top: 1rem; font-size: .875rem; }
    </style>
</head>
<body>
    <main>
        <h1>T-Rex Console</h1>
        <p>High score</p>
        <output id="record">Loading...</output>
        <p id="status" role="status">Reading saved record</p>
    </main>
    <script>
        fetch('/api/record', { cache: 'no-store' })
            .then(response => {
                if (!response.ok) throw new Error('Request failed');
                return response.json();
            })
            .then(data => {
                document.getElementById('record').textContent = String(data.record);
                document.getElementById('status').textContent = 'Saved on this console';
            })
            .catch(() => {
                document.getElementById('record').textContent = '--';
                document.getElementById('status').textContent = 'Could not load record';
            });
    </script>
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
    server.on("/api/record", HTTP_GET, []() {
        Preferences recordPreferences;
        if (!recordPreferences.begin("trex", true)) {
            server.send(500, "application/json", "{\"error\":\"storage_unavailable\"}");
            return;
        }

        unsigned long record = recordPreferences.getULong("highscore", 0);
        recordPreferences.end();

        char response[40];
        snprintf(response, sizeof(response), "{\"record\":%lu}", record);
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "application/json", response);
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
