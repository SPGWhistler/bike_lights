#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char* ssid = "whistler";
const char* password = "365-94-626512";

AsyncWebServer server(80);

void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  AsyncElegantOTA.begin(&server);
  server.begin();
}

void wifiLoop() {
  AsyncElegantOTA.loop();
}