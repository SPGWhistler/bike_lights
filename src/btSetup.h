#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

void btSetup() {
  SerialBT.begin("bike_lights");
  SerialBT.setTimeout(50);
}

bool btLoop(byte* bytes) {
	if (SerialBT.available()) {
		SerialBT.readBytes( bytes, 4);
	}
  if (SerialBT.hasClient()) {
    return true;
  } else {
    return false;
  }
}