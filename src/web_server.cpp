#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include "web_server.h"

namespace {

constexpr char WEB_AP_SSID[] = "T-Rex-Console";
constexpr uint8_t RESULT_CAPACITY = 30;
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

void handleResultsRequest() {
    Preferences recordPreferences;
    if (!recordPreferences.begin("trex", true)) {
        server.send(500, "application/json", "{\"error\":\"storage_unavailable\"}");
        return;
    }

    unsigned long results[RESULT_CAPACITY] = {};
    uint8_t writeIndex = recordPreferences.getUChar("resultsIndex", 0);
    uint8_t resultCount = recordPreferences.getUChar("resultCount", 0);
    size_t resultsSize = recordPreferences.getBytesLength("results");

    if (writeIndex >= RESULT_CAPACITY || resultCount > RESULT_CAPACITY
        || (resultCount > 0 && resultsSize != sizeof(results))
        || (resultsSize == sizeof(results)
            && recordPreferences.getBytes("results", results, sizeof(results)) != sizeof(results))) {
        recordPreferences.end();
        server.send(500, "application/json", "{\"error\":\"invalid_history\"}");
        return;
    }

    recordPreferences.end();

    char response[400];
    size_t responseLength = snprintf(response, sizeof(response), "{\"count\":%u,\"results\":[", resultCount);
    size_t firstIndex = (writeIndex + RESULT_CAPACITY - resultCount) % RESULT_CAPACITY;

    for (uint8_t i = 0; i < resultCount; i++) {
        size_t index = (firstIndex + i) % RESULT_CAPACITY;
        int written = snprintf(response + responseLength, sizeof(response) - responseLength,
                               "%s%lu", i == 0 ? "" : ",", results[index]);
        if (written < 0 || static_cast<size_t>(written) >= sizeof(response) - responseLength) {
            server.send(500, "application/json", "{\"error\":\"response_too_large\"}");
            return;
        }
        responseLength += static_cast<size_t>(written);
    }

    snprintf(response + responseLength, sizeof(response) - responseLength, "]}");
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", response);
}

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
    server.on("/api/results", HTTP_GET, handleResultsRequest);
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
