#include <Arduino.h>
#include "otaSetup.h"
#include "btSetup.h"
#include "ledSetup.h"
#include "common.h"

#ifndef LED_BUILTIN
    #define LED_BUILTIN 2
#endif

uint8_t lastOtaStatus = 255;
u_int8_t otaStatus = 0;
u_int8_t otaProgress = 0;
bool lastHasClient = true;
bool curHasClient = false;

TaskHandle_t Task1;


//Task1code:
void Task1code( void * pvParameters ){
  for(;;){
	  ledLoop();
  } 
}

void loop(void) {
  //Blue Tooth Commands
	byte bytes[4] = {0, 0, 0, 0};
	curHasClient = btLoop(bytes);
	if (bytes[0]) { //Only enter here if we've received bytes
		switch (bytes[0]) {
			case 0x28:
				SerialBT.println("solid color");
				solidColor(bytes);
				break;
			case 0x30:
				SerialBT.println("set pattern");
				setActivePattern(int(bytes[1]));
				break;
			case 0x35:
				SerialBT.println("decrease brightness");
        SerialBT.println(decreaseBrightness());
				break;
			case 0x36:
				SerialBT.println("increase brightness");
        SerialBT.println(increaseBrightness());
				break;
			case 0x37:
				SerialBT.println("set brightness");
				SerialBT.println(setBrightness(int(bytes[1])));
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
				SerialBT.println("Commands (in hex):");
				SerialBT.println("28 XX XX XX: set color 3 bytes");
				SerialBT.println("30 XX: set pattern 1 byte");
				SerialBT.println("35: dec brightness");
				SerialBT.println("36: inc brightness");
				SerialBT.println("37: set brightness 1 byte");
				SerialBT.println("40: right blinker");
				SerialBT.println("41: left blinker");
				SerialBT.println("42: enable ota");
				break;
		}
	}

  //OTA Stuff
  otaLoop();
  if (otaStatus != lastOtaStatus) { //If ota state changes
    lastOtaStatus = otaStatus;
    switch (otaStatus) {
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
          //BT Connected and OTA not setup
          //Normal state when riding bike.
          turnOnBuiltInLed();
        } else {
          //BT Not Connected and OTA not setup
          //Alternate state when riding bike.
          turnOffBuiltInLed();
        }
        break;
    }
  }

  //Blue Tooth Status
  if (curHasClient != lastHasClient) { //If bt connection changes
    lastHasClient = curHasClient;
    if (otaStatus == 0) { //Only show status when ota is not setup
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

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  btSetup();
  ledSetup();
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */ 
}