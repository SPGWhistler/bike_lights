#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

void btSetup() {
  SerialBT.begin("bike_lights");
  SerialBT.setTimeout(50);
}

void btLoop(byte* bytes) {
	if (SerialBT.available()) {
		SerialBT.readBytes( bytes, 4);
	} else {
		//TODO Only for development:
		if (Serial.available()) {
			Serial.readBytes( bytes, 4);
		}
	}
}