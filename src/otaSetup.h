#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "whistler";
const char* password = "365-94-626512";
const uint8_t STATE_NOTSETUP = 0;
const uint8_t STATE_WAITING = 1;
const uint8_t STATE_RUNNING = 2;
const uint8_t STATE_ERROR = 3;
uint8_t otaState = STATE_NOTSETUP;

uint8_t otaSetup() {
  if (otaState > 0) {
    return otaState;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult(5000) != WL_CONNECTED) {
    //Serial.println("Connection Failed!");
	  return otaState;
  }

  ArduinoOTA.setPort(3232);
  ArduinoOTA.setHostname("bikelights");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
	    } else { // U_SPIFFS
        type = "filesystem";
	    }
      otaState = STATE_RUNNING;
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      //Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      //Serial.println("\nEnd");
      otaState = STATE_WAITING;
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      otaState = STATE_RUNNING;
    })
    .onError([](ota_error_t error) {
      //Serial.printf("Error[%u]: ", error);
      //if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      //else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      //else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      //else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      //else if (error == OTA_END_ERROR) Serial.println("End Failed");
      otaState = STATE_ERROR;
    });

  ArduinoOTA.begin();

  //Serial.println("Ready");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
  otaState = STATE_WAITING;
  return otaState;
}

uint8_t otaLoop() {
	if (otaState > 0) {
		ArduinoOTA.handle();
  }
  return otaState;
}