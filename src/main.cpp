#include <Arduino.h>
#include "otaSetup.h"
#include "btSetup.h"
#include "ledSetup.h"

#ifndef LED_BUILTIN
    #define LED_BUILTIN 2
#endif

uint8_t lastOtaState = 255;
uint8_t curOtaState = 0;
bool lastHasClient = false;
bool curHasClient = false;

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  btSetup();
  ledSetup();
}

void loop(void) {
	byte bytes[4] = {0, 0, 0, 0};
	curHasClient = btLoop(bytes);
	if (bytes[0]) { //Only enter here if we've received bytes
		switch (bytes[0]) {
			case 0x28:
				SerialBT.println("solid color");
				solidColor(bytes);
				break;
			case 0x30:
				SerialBT.println("turn off");
				turnOff();
				break;
			case 0x31:
				SerialBT.println("test pattern");
				setActivePattern(PAT_TEST);
				break;
			case 0x32:
				SerialBT.println("fire");
				setActivePattern(PAT_FIRE);
				break;
			case 0x33:
				SerialBT.println("rainbow");
				setActivePattern(PAT_RAINBOW);
				break;
			case 0x34:
				SerialBT.println("sparkle");
				setActivePattern(PAT_SPARKLE);
				break;
			case 0x35:
				SerialBT.println("decrease brightness");
				decreaseBrightness();
        SerialBT.println(FastLED.getBrightness());
				break;
			case 0x36:
				SerialBT.println("increase brightness");
				increaseBrightness();
        SerialBT.println(FastLED.getBrightness());
				break;
			case 0x37:
				SerialBT.println("set brightness");
				setBrightness(bytes);
				break;
			case 0x38:
				SerialBT.println("sinelon");
				setActivePattern(PAT_MARQUE);
				break;
			case 0x39:
				SerialBT.println("otaSetup");
				otaSetup();
				break;
			case 0x40:
				SerialBT.println("right blinker");
				rightBlinker();
				break;
			case 0x41:
				SerialBT.println("left blinker");
				leftBlinker();
				break;
			case 0x42:
				SerialBT.println("enable OTA");
        otaSetup();
				break;
			default:
				//SerialBT.println("undefined");
				break;
		}
	}
	ledLoop();
  curOtaState = otaLoop();
  if (curOtaState != lastOtaState) { //If ota state changes
    lastOtaState = curOtaState;
    switch (curOtaState) {
      case 3: //OTA in error
        setBuiltInLedFlashRate(25);
        SerialBT.println("ota error");
        break;
      case 2: //OTA is running
        setBuiltInLedFlashRate(100);
        SerialBT.println("ota running");
        break;
      case 1: //OTA is waiting for update
        setBuiltInLedFlashRate(500);
        SerialBT.println("ota waiting");
        break;
      default: //OTA not setup
        SerialBT.println("ota not setup");
        if (curHasClient) {
          //Normal state when riding bike.
          turnOnBuiltInLed();
        } else {
          //BT Not Connected and OTA not setup
          turnOffBuiltInLed();
        }
        break;
    }
  }
  if (curHasClient != lastHasClient) { //If bt connection changes
    lastHasClient = curHasClient;
    if (curOtaState == 0) { //Only show status when ota is not setup
      if (curHasClient) {
        //BT Connected
        //Normal state when riding bike.
        turnOnBuiltInLed();
      } else {
        //BT Not Connected and OTA not setup
        turnOffBuiltInLed();
      }
    }
  }
}