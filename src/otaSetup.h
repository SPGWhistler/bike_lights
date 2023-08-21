#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "common.h"

const char* ssid = "whistler";
const char* password = "365-94-626512";
const char* ssidAlt = "SpiralGalaxy";
const char* passwordAlt = "SuperNova";
const uint8_t STATE_NOTSETUP = 0;
const uint8_t STATE_WAITING = 1;
const uint8_t STATE_RUNNING = 2;
const uint8_t STATE_ERROR = 3;

void otaStatus() {
  Serial.println("OTA Status:");
  Serial.print("Current Millis: ");
  Serial.println(millis());
  Serial.print("state: ");
  Serial.println(otaState);
  Serial.print("progress: ");
  Serial.println(otaProgress);
  Serial.println("");
}

void otaSetup(Print& p, u_int8_t wifiToUse = 0) {
  if (otaState > 0) {
    return;
  }

  WiFi.mode(WIFI_STA);
  if (wifiToUse = 0) {
    WiFi.begin(ssid, password);
  } else {
    WiFi.begin(ssidAlt, passwordAlt);
  }
  while (WiFi.waitForConnectResult(5000) != WL_CONNECTED) {
    //Serial.println("Connection Failed!");
    p.println("connection failed");
    WiFi.printDiag(p);
	  return;
  }
  WiFi.printDiag(p);
  p.println(WiFi.localIP());

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
      otaProgress = (progress / (total / 100));
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
  return;
}

void otaLoop() {
	if (otaState > 0) {
		ArduinoOTA.handle();
  }
}