#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

void btStatus() {
  Serial.println("BT Status:");
  Serial.print("Current Millis: ");
  Serial.println(millis());
  Serial.print("Has Client: ");
  Serial.println(SerialBT.hasClient());
  Serial.println("");
}

void btSetup() {
  SerialBT.begin("bike_lights");
  SerialBT.setTimeout(50);
}

bool btLoop(byte* bytes) {
	if (SerialBT.available()) {
		SerialBT.readBytes( bytes, 4);
    //SerialBT.readBytesUntil('\0', )
	}
  if (SerialBT.hasClient()) {
    return true;
  } else {
    return false;
  }
}