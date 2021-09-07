#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

void btSetup() {
  SerialBT.begin("bike_lights");
  SerialBT.setTimeout(50);
}

void btPrintln(const String &s) {
	SerialBT.println(s);
}

void btLoop(byte* bytes) {
	if (SerialBT.available()) {
		SerialBT.readBytes( bytes, 4);
	}
}