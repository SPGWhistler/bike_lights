#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#include <Preferences.h>
#include "common.h"

#define PIN 5
#define LED_COUNT 22
#define STRIP_TYPE WS2811
#define LED_ORDER RGB

const uint8_t PAT_PAUSE = 0;
const uint8_t PAT_OFF = 1;
const uint8_t PAT_TEST = 2;
const uint8_t PAT_MARQUE = 3;
const uint8_t PAT_SPARKLE = 4;
const uint8_t PAT_RAINBOW = 5;
const uint8_t PAT_SOLID = 6;
const uint8_t PAT_FIRE = 7;
const uint8_t PAT_LOWBATT = 8;

Preferences preferences;

CRGB leds[LED_COUNT];

uint16_t builtInLedFlashMS = 1000;
uint8_t activePattern = PAT_OFF;
uint8_t lastPattern = PAT_OFF;


//Called internally only to temporarily erase leds.
//Use setActivePattern(PAT_OFF) to kill the running pattern.
void black() {
	FastLED.clear(true);
	FastLED.showColor(CRGB::Black);
}

void setActivePattern(int pattern = -1) {
	black();
	if (pattern != PAT_PAUSE && pattern != PAT_LOWBATT) { //Switch to a real pattern
    if (pattern == -1) { //If no pattern was specified, use the last pattern
		  pattern = lastPattern;
    } else { //If a pattern was specified, use it
		  lastPattern = pattern;
    }
		SerialBT.println("Switching to pattern:");
		SerialBT.println(pattern);
    preferences.putUInt("pattern", pattern);
	} else { //Switch to the pause pattern
		SerialBT.println("Temp switching to pause pattern.");
	}
	activePattern = pattern;
}

/**
 * Reverse a set of leds.
 */
void reverseLeds(CRGB* leds, uint startPos, uint count, uint length) {
	for (uint i = startPos; i < startPos + (count / 2); i++) {
		CRGB temp = leds[i];
		leds[i] = leds[length - (i - startPos) - 1];
		leds[length - (i - startPos) - 1] = temp;
	}
}

void reverseBodySection2() {
	//reverseLeds(body, BODY_SECTION2_START, BODY_SECTION2_COUNT, BODY_COUNT);
}

int setBrightness(int b = FastLED.getBrightness()) {
	FastLED.setBrightness(b);
  preferences.putUInt("bright", b);
  return b;
}
int increaseBrightness() {
  int cur = FastLED.getBrightness();
  if (cur < 10) {
    cur = cur + 1;
  } else if (cur < 50) {
    cur = cur + 3;
  } else if (cur < 128) {
    cur = cur + 5;
  } else {
    cur = cur + 10;
  }
  if (cur > 255) {
    cur = 255;
  }
  return setBrightness(cur);
}
int decreaseBrightness() {
  int cur = FastLED.getBrightness();
  if (cur < 10) {
    cur = cur - 1;
  } else if (cur < 50) {
    cur = cur - 3;
  } else if (cur < 128) {
    cur = cur - 5;
  } else {
    cur = cur - 10;
  }
  if (cur < 0) {
    cur = 0;
  }
  return setBrightness(cur);
}

CRGB solidColor = CRGB::Black;
void setSolidColorFromBytes(byte* bytes) {
	setActivePattern(PAT_SOLID);
	solidColor.r = bytes[1];
	solidColor.g = bytes[2];
	solidColor.b = bytes[3];
}
void solidColorLoop() {
	FastLED.showColor(solidColor);
}

bool lastBuiltInLedState = false;
void setBuiltInLedFlashRate(uint16_t rate) {
  builtInLedFlashMS = rate;
}

void turnOffBuiltInLed() {
  builtInLedFlashMS = 0;
  digitalWrite(LED_BUILTIN, LOW);
  lastBuiltInLedState = false;
}

void turnOnBuiltInLed() {
  builtInLedFlashMS = 0;
  digitalWrite(LED_BUILTIN, HIGH);
  lastBuiltInLedState = true;
}

void builtInLedFlashLoop() {
  if (lastBuiltInLedState == false) {
    digitalWrite(LED_BUILTIN, HIGH);
    lastBuiltInLedState = true;
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    lastBuiltInLedState = false;
  }
}

void otaUpdateLoop() {
  if (otaProgress > 0) {
    if (activePattern != PAT_PAUSE) {
      setActivePattern(PAT_PAUSE);
	    FastLED.setBrightness(128); //Use half brightness
    }
    int ledProgress = map(otaProgress, 0, 100, 0, 22);
    for( uint j = 0; j <= ledProgress; j++) {
      CRGB color = CRGB::Blue;
      leds[j] = color;
    }
	  FastLED.show();
  }
}

uint8_t lastLowBattPatternColor = 0;
void lowBattPatternLoop() {
	CRGB color;
	switch (lastLowBattPatternColor) {
		case 0:
			color = CRGB(255, 0, 0);
			lastLowBattPatternColor = 1;
			break;
		case 1:
			color = CRGB(0, 0, 0);
			lastLowBattPatternColor = 0;
			break;
	}
	FastLED.showColor(color);
}

uint8_t lastTestPatternColor = 0;
void testPatternLoop() {
	CRGB color;
	switch (lastTestPatternColor) {
		case 0:
			color = CRGB(255, 0, 0);
			lastTestPatternColor = 1;
			break;
		case 1:
			color = CRGB(0, 255, 0);
			lastTestPatternColor = 2;
			break;
		case 2:
			color = CRGB(0, 0, 255);
			lastTestPatternColor = 3;
			break;
		case 3:
			color = CRGB(255, 255, 255);
			lastTestPatternColor = 0;
			break;
	}
	FastLED.showColor(color);
}

void sparkleLoop( uint8_t chanceOfGlitter) {
  //This probably shouldn't clear every loop ?
	//FastLED.clear(true); //This makes this method not work - it seems to clear the data after show is executed or something
	FastLED.showColor(CRGB::Black);
	if( random8(100) <= chanceOfGlitter) {
    leds[ random16(LED_COUNT) ] = CRGB::White;
	}
	FastLED.show();
}

#define COOLING 55
#define SPARKING 120
byte heat[LED_COUNT];
void fireLoop(CRGB* leds, uint start, uint end, bool reverse, byte* heat) {
	uint count = end - start;
    for( uint i = start; i < end; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / count) + 2));
    }
    for( uint k = end - 1; k >= start + 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    if( random8() < SPARKING ) {
      uint y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }
    for( uint j = start; j < end; j++) {
      CRGB color = HeatColor( heat[j]);
      uint pixelnumber;
      if( reverse ) {
        pixelnumber = (end - 1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
	FastLED.show();
}

uint8_t lastHue = 0;
void rainbowLoop() {
	fill_rainbow(leds, LED_COUNT, lastHue);
	reverseBodySection2();
	lastHue++;
	FastLED.show();
}

uint16_t holdTime = 50;  // Milliseconds to hold position before advancing.
uint8_t spacing = 20;      // Sets pixel spacing. [Use 2 or greater]
int8_t delta = 1;         // Sets forward or backwards direction amount. (Can be negative.)
uint8_t width = 1;        // Can increase the number of pixels (width) of the chase. [1 or greater]
uint8_t fadeRate = 75;   // How fast to fade out tail. [0-255]
uint8_t hue = 180;        // Starting color.
uint8_t hue2_shift = 20;  // Hue shift for secondary color.  Use 0 for no shift. [0-255]
int16_t pos;                // Pixel position.
int8_t advance = -1*width;  // Stores the advance amount.
uint8_t color;              // Stores a hue color.
void marqueLoop(CRGB* leds, int numLeds) {
  EVERY_N_SECONDS(5){  // Demo: Change direction every N seconds.
    delta = -1*delta;
  }

  EVERY_N_SECONDS(2){  // Demo: Change the hue every N seconds.
    hue = hue + random8(30,61);  // Shift the hue to something new.
  }

  EVERY_N_MILLISECONDS(holdTime){  // Advance pixels to next position.
	reverseBodySection2();
    // Advance pixel postion down strip, and rollover if needed.
    advance = (advance + delta + numLeds) % numLeds;
    // Fade out tail.
    fadeToBlackBy(leds, numLeds, fadeRate);
    // Update pixels down the strip.
    for (uint8_t i=0; i<(numLeds/spacing); i++){
      for (uint8_t w = 0; w<width; w++){
        pos = (spacing * (i-1) + spacing + advance + w) % numLeds;
        if ( w % 2== 0 ){  // Is w even or odd?
          color = hue;
        } else {
          color = hue + hue2_shift;
        }
        leds[pos] = CHSV(color,255,255);
      }
    }
	reverseBodySection2();
    FastLED.show();
  }
}

void ledStatus() {
  Serial.println("LED Status:");
  Serial.print("Current Millis: ");
  Serial.println(millis());
  Serial.print("Built In Led Flash Rate: ");
  Serial.println(builtInLedFlashMS);
  Serial.print("Active Pattern: ");
  Serial.println(activePattern);
  Serial.print("LED Controllers: ");
  Serial.println(FastLED.count());
  Serial.print("LED Brightness: ");
  Serial.println(FastLED.getBrightness());
  Serial.print("LED FPS: ");
  Serial.println(FastLED.getFPS());
  Serial.println("");
}


/**
 * Setup.
 */
void ledSetup() {
	FastLED.addLeds<STRIP_TYPE, PIN, LED_ORDER>(leds, LED_COUNT).setCorrection(TypicalSMD5050);

  preferences.begin("bikelights", false);

  setBrightness(preferences.getUInt("bright", 128)); //Default to half brightness
	setActivePattern(preferences.getUInt("pattern", PAT_MARQUE)); //Default to marque
}

/**
 * Loop.
 * High frequency code here.
 */
void ledLoop() {
	switch (activePattern) {
		case PAT_TEST:
      EVERY_N_MILLIS(1000) {
				testPatternLoop();
			}
			break;
		case PAT_SOLID:
      EVERY_N_MILLIS(1000) {
				solidColorLoop();
			}
			break;
		case PAT_SPARKLE:
      EVERY_N_MILLIS(100) {
				sparkleLoop(100);
			}
			break;
		case PAT_MARQUE:
			marqueLoop(leds, LED_COUNT);
			break;
		case PAT_RAINBOW:
      EVERY_N_MILLIS(50) {
				rainbowLoop();
			}
			break;
		case PAT_FIRE:
      EVERY_N_MILLIS(16) {
				fireLoop(leds, 0, LED_COUNT, true, heat);
			}
			break;
		case PAT_LOWBATT:
      EVERY_N_MILLIS(500) {
				lowBattPatternLoop();
			}
			break;
		default:
			break;
	}
  if (builtInLedFlashMS > 0) {
    EVERY_N_MILLISECONDS_I( timingObj, 1) {
      timingObj.setPeriod(builtInLedFlashMS);
      builtInLedFlashLoop();
    }
  }
  EVERY_N_MILLIS(500) {
    otaUpdateLoop();
  }
}