#include <FastLED.h>
#include <Canrun.h>
#include <Preferences.h>

#define PIN 5
#define LED_COUNT 100
#define STRIP_TYPE WS2811
#define LED_ORDER RGB

const uint8_t PAT_OVERRIDE = 0;
const uint8_t PAT_OFF = 1;
const uint8_t PAT_MARQUE = 2;
const uint8_t PAT_SPARKLE = 3;
const uint8_t PAT_RAINBOW = 4;
const uint8_t PAT_SOLID = 5;
const uint8_t PAT_TEST = 6;
const uint8_t PAT_FIRE = 7;

Canrun canrun;
Preferences preferences;

CRGB leds[LED_COUNT];

uint16_t builtInLedFlashMS = 1000;
uint8_t activePattern = PAT_OFF;
uint8_t lastPattern = PAT_OFF;
bool shouldRightBlinker = false;
bool shouldLeftBlinker = false;
bool leftBlinkerOn = false;
bool rightBlinkerOn = false;


//Called internally only to temporarily erase leds.
//See turnOff() below to kill the running pattern.
void black() {
	FastLED.clear(true);
	FastLED.showColor(CRGB::Black);
}

void recallLastPattern() {
	black();
	activePattern = lastPattern;
	SerialBT.println("Restoring Last Pattern:");
	SerialBT.println(lastPattern);
}

void setActivePattern(uint8_t pattern) {
	black();
	if (pattern != PAT_OVERRIDE) {
		if (activePattern == PAT_OVERRIDE) return;
		lastPattern = pattern;
		SerialBT.println("Saving to Last Pattern:");
		SerialBT.println(lastPattern);
    preferences.putUInt("pattern", pattern);
	} else {
		SerialBT.println("Not to saving pattern.");
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

void saveBrightness() {
  preferences.putUInt("bright", FastLED.getBrightness());
}

void setBrightness(byte* bytes) {
	FastLED.setBrightness(int(bytes[1]));
  saveBrightness();
}
void increaseBrightness() {
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
	FastLED.setBrightness(cur);
  saveBrightness();
}
void decreaseBrightness() {
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
	FastLED.setBrightness(cur);
  saveBrightness();
}

//Called externally to turn off the leds and kill the pattern.
void turnOff() {
	setActivePattern(PAT_OFF);
}

//TODO This pattern wont save because it is literally only set once when this method is called.
//If I wanted to make this save, I'd need to convert this into a 'doSolidColor' function, and
//add a 'setSolidColor' function which accepted the bytes and saved them.
void solidColor(byte* bytes) {
	setActivePattern(PAT_SOLID);
	CRGB color;
	color.r = bytes[1];
	color.g = bytes[2];
	color.b = bytes[3];
	FastLED.showColor(color);
}

bool lastBuiltInLedState = false;
void setBuiltInLedFlashRate(uint16_t rate) {
  canrun.setupDelay('i', rate);
}

void turnOffBuiltInLed() {
  canrun.setupDelay('i', 0);
  digitalWrite(LED_BUILTIN, LOW);
  lastBuiltInLedState = false;
}

void turnOnBuiltInLed() {
  canrun.setupDelay('i', 0);
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
    if (activePattern != PAT_OVERRIDE) {
      setActivePattern(PAT_OVERRIDE);
	    FastLED.setBrightness(50);
    }
    for( uint j = 0; j <= otaProgress; j++) {
      CRGB color = CRGB::Blue;
      leds[j] = color;
    }
	  FastLED.show();
  }
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
	FastLED.clear(true); //This makes this method not work - it seems to clear the data after show is executed or something
	if( random8() < chanceOfGlitter) {
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

void setBlinker(bool on, CRGB* leds, uint8_t start, uint8_t end) {
	for (uint8_t i = start; i < end; i++){
		leds[i] = (on) ? CRGB::Orange : CRGB::Black;
	}
	FastLED.show();
}
void setLeftBlinker(bool on) {
	//setBlinker(on, frontLeft, FRONT_LEFT_START, FRONT_LEFT_END);
	//setBlinker(on, body, BODY_LEFT_START, BODY_LEFT_END);
}
void setRightBlinker(bool on) {
	//setBlinker(on, frontRight, FRONT_RIGHT_START, FRONT_RIGHT_END);
	//setBlinker(on, body, BODY_RIGHT_START, BODY_RIGHT_END);
}
void blinkerLoop() {
	if (shouldRightBlinker) {
		setRightBlinker(rightBlinkerOn);
		rightBlinkerOn = !rightBlinkerOn;
	}
	if (shouldLeftBlinker) {
		setLeftBlinker(leftBlinkerOn);
		leftBlinkerOn = !leftBlinkerOn;
	}
}

void blinker(bool isLeft, bool &shouldBlink, bool &shouldBlink2, bool &status, bool &status2) {
	shouldBlink = !shouldBlink;
	if (!shouldBlink) {
		//Turn Blinker Off
		if (isLeft) {
			setLeftBlinker(false);
		} else {
			setRightBlinker(false);
		}
		if (!shouldBlink2) {
			recallLastPattern();
		}
	} else {
		//Turn Blinker On
		setActivePattern(PAT_OVERRIDE);
		status = false;
		if (shouldBlink2) {
			status2 = false;
		}
	}
}
void leftBlinker() {
	blinker(true, shouldLeftBlinker, shouldRightBlinker, leftBlinkerOn, rightBlinkerOn);
}
void rightBlinker() {
	blinker(false, shouldRightBlinker, shouldLeftBlinker, rightBlinkerOn, leftBlinkerOn);
}




/**
 * Setup.
 */
void ledSetup() {
	FastLED.addLeds<STRIP_TYPE, PIN, LED_ORDER>(leds, LED_COUNT).setCorrection(TypicalSMD5050);

  preferences.begin("bikelights", false);

  uint8_t brightInt = preferences.getUInt("bright", 255);
  byte bright[2] = {0, (byte) brightInt};
  setBrightness(bright);
	canrun.setupDelay('t', 1000);
	canrun.setupDelay('s', 1000);
	canrun.setupDelay('r', 50);
	canrun.setupDelay('f', 16);
	canrun.setupDelay('b', 100);
  canrun.setupDelay('o', 500);
  setBuiltInLedFlashRate(1);
	setActivePattern(preferences.getUInt("pattern", PAT_MARQUE));
}

/**
 * Loop.
 * High frequency code here.
 */
void ledLoop() {
	switch (activePattern) {
		case PAT_TEST:
			if (canrun.run('t')) {
				testPatternLoop();
			}
			break;
		case PAT_SPARKLE:
			if (canrun.run('s')) {
				sparkleLoop(255);
			}
			break;
		case PAT_MARQUE:
			marqueLoop(leds, LED_COUNT);
			break;
		case PAT_RAINBOW:
			if (canrun.run('r')) {
				rainbowLoop();
			}
			break;
		case PAT_FIRE:
			if (canrun.run('f')) {
				fireLoop(leds, 0, LED_COUNT, true, heat);
			}
			break;
		default:
			break;
	}
	if (canrun.run('b')) {
		blinkerLoop();
	}
  if (canrun.run('i')) {
    builtInLedFlashLoop();
  }
  if (canrun.run('o')) {
    otaUpdateLoop();
  }
}