#include <Arduino.h>
#include "otaSetup.h"
#include "btSetup.h"
#include "ledSetup.h"
#include "common.h"

#ifndef LED_BUILTIN
    #define LED_BUILTIN 2
#endif

const int voltage_pin = 33;
const float low_battery_voltage = 14.0;
const float critical_battery_voltage = 12.0;
bool batteryVoltageWarned = false;
float voltage_value = 0;
float test_voltage_value = 0;

uint8_t lastOtaState = 255;
u_int8_t otaState = 0;
u_int8_t otaProgress = 0;
bool lastHasClient = true;
bool curHasClient = false;

TaskHandle_t Task0;
TaskHandle_t Task1;
TaskHandle_t Task2;

//void test(byte* bytes) {
//  int n = sizeof(bytes);
//  char mystr[n + 1];
//  memcpy(mystr, bytes, n);
//  mystr[n] = '\0'; // Null-terminate the string
//  SerialBT.println(mystr);
//}


//Main Loop:
//Runs on core 0
void MainLoopCode( void * pvParameters ){
  Serial.print("Main Loop running on core ");
  Serial.println(xPortGetCoreID());
  for(;;){
    //Blue Tooth Commands
    byte bytes[4] = {0, 0, 0, 0};
    curHasClient = btLoop(bytes); //Refactor to not return a value, use global instead
    if (bytes[0]) { //Only enter here if we've received bytes
      switch (bytes[0]) {
        case 0x28: //Requires 3 additional bytes
          SerialBT.println("solid color");
          setSolidColorFromBytes(bytes);
          break;
        case 0x30: //Requires 1 additional byte
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
        case 0x37: //Requires 1 additional byte
          SerialBT.println("set brightness");
          SerialBT.println(setBrightness(int(bytes[1])));
          break;
        case 0x38:
          voltage_value = (analogRead(voltage_pin) * 16.5 ) / (4095);
          SerialBT.print(voltage_value);
          SerialBT.println(" volts");
          break;
        case 0x42:
          SerialBT.println("enable OTA");
          otaSetup(SerialBT, 0);
          break;
        case 0x43:
          SerialBT.println("enable OTA Lexis");
          otaSetup(SerialBT, 1);
          break;
        case 0x44:
          SerialBT.print("set test voltage value ");
          SerialBT.println(int(bytes[1]));
          test_voltage_value = int(bytes[1]);
          break;
        default:
          SerialBT.println("Commands (in hex):");
          SerialBT.println("28 XX XX XX: set color 3 bytes");
          SerialBT.println("30 XX: set pattern 1 byte");
          SerialBT.println("35: dec brightness");
          SerialBT.println("36: inc brightness");
          SerialBT.println("37: set brightness 1 byte");
          SerialBT.println("38: get battery voltage");
          SerialBT.println("42: enable ota");
          SerialBT.println("43: enable ota lexis");
          SerialBT.println("");
          SerialBT.println("Patterns:");
          SerialBT.println("0 Pause");
          SerialBT.println("1 Off");
          SerialBT.println("2 Test");
          SerialBT.println("3 Marque");
          SerialBT.println("4 Sparkle");
          SerialBT.println("5 Rainbow");
          SerialBT.println("6 Solid");
          SerialBT.println("7 Fire");
          SerialBT.println("8 LowBatt");
          break;
      }
    }

    //OTA Stuff
    if (otaState != lastOtaState) { //If ota state changes
      lastOtaState = otaState;
      switch (otaState) {
        case 3: //OTA in error
          setBuiltInLedFlashRate(50);
          SerialBT.println("ota error");
          break;
        case 2: //OTA is running
          setBuiltInLedFlashRate(500);
          SerialBT.println("ota running");
          break;
        case 1: //OTA is waiting for update
          setBuiltInLedFlashRate(200);
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
      if (otaState == 0) { //Only show status when ota is not setup
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

    //Check battery voltage every 10 seconds
    EVERY_N_MILLIS(10000) {
      voltage_value = (analogRead(voltage_pin) * 16.5 ) / (4095); //16.5 is the max voltage the esp32 can read
      if (test_voltage_value > 0) {
        voltage_value = test_voltage_value;
      }
      if (voltage_value <= low_battery_voltage && batteryVoltageWarned == false) {
        //If the voltage is low, warn by flashing lights. But allow for continued use.
        batteryVoltageWarned = true; //Warned already, dont warn again
        setBrightness(128); //Set half brightness
        setActivePattern(8); //PAT_LOWBATT
      }
      if (voltage_value <= critical_battery_voltage) {
        //If voltage is critically low, force a power down
        setActivePattern(PAT_PAUSE); //Pause pattern without saving pause
	      FastLED.setBrightness(0); //Turn off LEDs without saving brightness
        FastLED.clear(true); //Clear pattern without saving it
        esp_deep_sleep_start(); //Go as low power as possible until reset is pressed or power is lost
      }
    }
    delay(1);
  } 
}

//Led Loop:
//Runs on core 1
void LEDLoopCode( void * pvParameters ){
  Serial.print("LED Loop running on core ");
  Serial.println(xPortGetCoreID());
  for(;;){
	  ledLoop();
    delay(1);
  } 
}


void loop(void) {
  otaLoop(); //This must be in loop because if it is in a task on processor 0, it crashes with 'no more processes' when updating.
  delay(1);
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("bike_lights started");
  pinMode(LED_BUILTIN, OUTPUT);
  btSetup();
  ledSetup();
  xTaskCreatePinnedToCore(
                    MainLoopCode,   /* Task function. */
                    "Main loop",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    0,           /* priority of the task */
                    &Task0,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */ 
  xTaskCreatePinnedToCore(
                    LEDLoopCode,   /* Task function. */
                    "Led loop",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */ 
}