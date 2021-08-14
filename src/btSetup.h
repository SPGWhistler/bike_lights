#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

void btSetup() {
  SerialBT.begin("bike_lights");
}

byte btLoop() {
	if (SerialBT.available()) {
		return SerialBT.read();
	}
	return 0;
}