#include <Arduino.h>
#include "btSetup.h"
#include "ledSetup.h"

void setup(void) {
  Serial.begin(115200);
  btSetup();
  ledSetup();
}

void loop(void) {
	byte bytes[4] = {0, 0, 0, 0};
	btLoop(bytes);
	if (bytes[0]) {
		switch (bytes[0]) {
			case 0x28:
				Serial.println("show color");
				show_color(bytes);
				break;
			case 0x30:
				Serial.println("black");
				black();
				break;
			case 0x31:
				Serial.println("red");
				red();
				break;
			case 0x32:
				Serial.println("green");
				green();
				break;
			case 0x33:
				Serial.println("blue");
				blue();
				break;
			case 0x34:
				Serial.println("white");
				white();
				break;
			case 0x35:
				Serial.println("rainbow");
				rainbow();
				break;
			case 0x36:
				Serial.println("sparkle");
				sparkle();
				break;
			case 0x37:
				Serial.println("set brightness");
				setBrightness(bytes);
				break;
			default:
				Serial.println("undefined");
				break;
		}
	}
	ledLoop();
}