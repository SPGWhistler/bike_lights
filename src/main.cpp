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
				btPrintln("show color");
				show_color(bytes);
				break;
			case 0x30:
				btPrintln("black");
				black();
				break;
			case 0x31:
				btPrintln("red");
				red();
				break;
			case 0x32:
				btPrintln("green");
				green();
				break;
			case 0x33:
				btPrintln("blue");
				blue();
				break;
			case 0x34:
				btPrintln("white");
				white();
				break;
			case 0x35:
				btPrintln("rainbow");
				rainbow();
				break;
			case 0x36:
				btPrintln("sparkle");
				sparkle();
				break;
			case 0x37:
				btPrintln("set brightness");
				setBrightness(bytes);
				break;
			case 0x38:
				btPrintln("sinelon");
				sinelon();
				break;
			case 0x39:
				btPrintln("otaSetup");
				otaSetup();
				break;
			case 0x40:
				btPrintln("right blinker");
				rightBlinker();
				break;
			case 0x41:
				btPrintln("left blinker");
				leftBlinker();
				break;
			default:
				btPrintln("undefined");
				break;
		}
	}
	ledLoop();
	otaLoop();
}