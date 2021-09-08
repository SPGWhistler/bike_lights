#include <FastLED.h>
#include <Canrun.h>

//15, 15, 32, 92 (32 + 16 + 14 + 14 + 16)
//Front left led strip
#define FRONT_LEFT_PIN 23
#define FRONT_LEFT_START 0
#define FRONT_LEFT_COUNT 15
const uint FRONT_LEFT_END = FRONT_LEFT_START + FRONT_LEFT_COUNT;

//Front right led strip
#define FRONT_RIGHT_PIN 4
#define FRONT_RIGHT_START 0
#define FRONT_RIGHT_COUNT 15
const uint FRONT_RIGHT_END = FRONT_RIGHT_START + FRONT_RIGHT_COUNT;

//Body led strip section 1
#define BODY_SECTION1_PIN 19
#define BODY_SECTION1_START 0
#define BODY_SECTION1_COUNT 92
const uint BODY_SECTION1_END = BODY_SECTION1_START + BODY_SECTION1_COUNT;

//Body led strip section 2
#define BODY_SECTION2_PIN 5
#define BODY_SECTION2_START 92
#define BODY_SECTION2_COUNT 32
const uint BODY_SECTION2_END = BODY_SECTION2_START + BODY_SECTION2_COUNT;

//Body leds (virtual)
#define BODY_START 0
const uint BODY_COUNT = BODY_SECTION1_COUNT + BODY_SECTION2_COUNT;
const uint BODY_END = BODY_START + BODY_COUNT;

//Body left led strips (virtual)
#define BODY_LEFT_START 0
#define BODY_LEFT_COUNT 62
const uint BODY_LEFT_END = BODY_LEFT_START + BODY_LEFT_COUNT;

//Body right led strips (virtual)
#define BODY_RIGHT_START 62
#define BODY_RIGHT_COUNT 62
const uint BODY_RIGHT_END = BODY_RIGHT_START + BODY_RIGHT_COUNT;

//Rear left led strip (virtual)
#define REAR_LEFT_START 48
#define REAR_LEFT_COUNT 14
const uint REAR_LEFT_END = REAR_LEFT_START + REAR_LEFT_COUNT;

//Rear right led strip (virtual)
#define REAR_RIGHT_START 62
#define REAR_RIGHT_COUNT 14
const uint REAR_RIGHT_END = REAR_RIGHT_START + REAR_RIGHT_COUNT;


#define STRIP_TYPE WS2812B
#define LED_ORDER GRB

const uint8_t PAT_OVERRIDE = 0;
const uint8_t PAT_OFF = 1;
const uint8_t PAT_MARQUE = 2;
const uint8_t PAT_SPARKLE = 3;
const uint8_t PAT_RAINBOW = 4;
const uint8_t PAT_SOLID = 5;
const uint8_t PAT_TEST = 6;
const uint8_t PAT_FIRE = 7;

Canrun canrun;

CRGB frontLeft[FRONT_LEFT_COUNT];
CRGB frontRight[FRONT_RIGHT_COUNT];
CRGB body[BODY_COUNT];
CRGB bodyLeft[BODY_LEFT_COUNT]; //virtual
CRGB bodyRight[BODY_RIGHT_COUNT]; //virtual
CRGB rearLeft[REAR_LEFT_COUNT]; //virtual
CRGB rearRight[REAR_RIGHT_COUNT]; //virtual

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
	reverseLeds(body, BODY_SECTION2_START, BODY_SECTION2_COUNT, BODY_COUNT);
}

void setBrightness(byte* bytes = 0) {
	uint8_t brightness = 55;
	FastLED.setBrightness(brightness);
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

void sparkleLoop( fract8 chanceOfGlitter) {
	FastLED.clear(true);
	if( random8() < chanceOfGlitter) {
		frontLeft[ random16(FRONT_LEFT_COUNT) ] += CRGB::White;
		frontRight[ random16(FRONT_RIGHT_COUNT) ] += CRGB::White;
		body[ random16(BODY_COUNT) ] += CRGB::White;
	}
	FastLED.show();
}

#define COOLING 55
#define SPARKING 120
byte heat[124];
//TODO This looks crappy because its just one fire for all the leds.
//It would be better to convert this to render a fire on each virtual set
//of leds. That requires reworking how the 'heat' variable above is used.
void fireLoop(CRGB* leds, int count, bool reverse, byte* heat) {
    for( int i = 0; i < count; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / count) + 2));
    }
    for( int k= count - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }
    for( int j = 0; j < count; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( reverse ) {
        pixelnumber = (count-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
	FastLED.show();
}

uint8_t lastHue = 0;
void rainbowLoop() {
	fill_rainbow(frontLeft, FRONT_LEFT_COUNT, lastHue);
	fill_rainbow(frontRight, FRONT_RIGHT_COUNT, lastHue);
	fill_rainbow(body, BODY_COUNT, lastHue);
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
	setBlinker(on, frontLeft, FRONT_LEFT_START, FRONT_LEFT_END);
	setBlinker(on, body, BODY_LEFT_START, BODY_LEFT_END);
}
void setRightBlinker(bool on) {
	setBlinker(on, frontRight, FRONT_RIGHT_START, FRONT_RIGHT_END);
	setBlinker(on, body, BODY_RIGHT_START, BODY_RIGHT_END);
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
	//Front Left (15 leds)
	FastLED.addLeds<STRIP_TYPE, FRONT_LEFT_PIN, LED_ORDER>(frontLeft, FRONT_LEFT_COUNT).setCorrection(TypicalSMD5050);
	//Front Right (15 leds)
	FastLED.addLeds<STRIP_TYPE, FRONT_RIGHT_PIN, LED_ORDER>(frontRight, FRONT_RIGHT_COUNT).setCorrection(TypicalSMD5050);
	//Middle1 (92 leds, starting at index 0 of mid)
	FastLED.addLeds<STRIP_TYPE, BODY_SECTION1_PIN, LED_ORDER>(body, 0, BODY_SECTION1_COUNT).setCorrection(TypicalSMD5050);
	//Middle2 (32 leds, starting at index 92 of mid)
	FastLED.addLeds<STRIP_TYPE, BODY_SECTION2_PIN, LED_ORDER>(body, BODY_SECTION2_START, BODY_SECTION2_COUNT).setCorrection(TypicalSMD5050);

	setBrightness();
	canrun.setupDelay('t', 1000);
	canrun.setupDelay('s', 100);
	canrun.setupDelay('r', 50);
	canrun.setupDelay('f', 16);
	canrun.setupDelay('b', 100);
	setActivePattern(PAT_MARQUE);
}

/**
 * Loop.
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
				sparkleLoop(200);
			}
			break;
		case PAT_MARQUE:
			marqueLoop(body, BODY_COUNT);
			break;
		case PAT_RAINBOW:
			if (canrun.run('r')) {
				rainbowLoop();
			}
			break;
		case PAT_FIRE:
			if (canrun.run('f')) {
				fireLoop(body, BODY_COUNT, true, heat);
			}
			break;
		default:
			break;
	}
	if (canrun.run('b')) {
		blinkerLoop();
	}
}