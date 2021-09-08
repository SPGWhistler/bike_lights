#include <Arduino.h>
#include "otaSetup.h"
#include "btSetup.h"
#include "ledSetup.h"

void setup(void) {
  Serial.begin(115200);
  btSetup();
  ledSetup();
  //TODO Remove, or add in a way to only stay active if whistler is available.
  otaSetup();
}

void loop(void) {
	byte bytes[4] = {0, 0, 0, 0};
	btLoop(bytes);
	if (bytes[0]) {
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
			case 0x35:
				SerialBT.println("rainbow");
				setActivePattern(PAT_RAINBOW);
				break;
			case 0x36:
				SerialBT.println("sparkle");
				setActivePattern(PAT_SPARKLE);
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
			default:
				SerialBT.println("undefined");
				break;
		}
	}
	ledLoop();
	otaLoop();
}