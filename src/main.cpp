#include <Arduino.h>
#include "btSetup.h"
#include "ledSetup.h"

void setup(void) {
  Serial.begin(115200);
  btSetup();
  ledSetup();
}

void loop(void) {
	byte val = btLoop();
	switch (val) {
		case 0x30:
			black();
			break;
		case 0x31:
			red();
			break;
		case 0x32:
			green();
			break;
		case 0x33:
			blue();
			break;
		case 0x34:
			white();
			break;
		case 0x35:
			rainbow();
			break;
	}
}