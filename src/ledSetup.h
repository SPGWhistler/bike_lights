#include <FastLED.h>
#include <Canrun.h>

//15, 15, 32, 92 (32 + 16 + 14 + 14 + 16)
#define FLEFT_COUNT 15
#define FLEFT_PIN 23
#define FRIGHT_COUNT 15
#define FRIGHT_PIN 4
#define MID1_COUNT 92
#define MID2_COUNT 32
#define MID1_PIN 19
#define MID2_PIN 5

#define STRIP_TYPE WS2812B
#define LED_ORDER GRB

Canrun canrun;

const uint MID_COUNT = MID1_COUNT + MID2_COUNT;

CRGB fLeft[FLEFT_COUNT];
CRGB fRight[FRIGHT_COUNT];
CRGB mid[MID_COUNT];

bool shouldSinelon = false;
bool shouldSparkle = false;

/**
 * All a specific color, called internally.
 */
void doShowColor(CRGB color) {
	FastLED.showColor(color);
}

/**
 * All off, kill all effects.
 */
void black() {
	FastLED.clear(true);
	doShowColor(CRGB::Black);
	shouldSparkle = false;
	shouldSinelon = false;
}

/**
 * All a specific color.
 * Four bytes, first is ignored, then R, G, and B values.
 */
void show_color(byte* bytes) {
	black();
	CRGB color;
	color.r = bytes[1];
	color.g = bytes[2];
	color.b = bytes[3];
	doShowColor(color);
}

/**
 * All red.
 */
void red(){
	black();
	FastLED.showColor(CRGB::Red);
}

/**
 * All blue.
 */
void blue() {
	black();
	FastLED.showColor(CRGB::Blue);
}

/**
 * All green.
 */
void green() {
	black();
	FastLED.showColor(CRGB::Green);
}

/**
 * All white.
 */
void white() {
	black();
	FastLED.showColor(CRGB::White);
}

/**
 * Turn on sparkle.
 */
void sparkle() {
	black();
	shouldSparkle = true;
}

/**
 * Called by loop to do the sparkle.
 */
void doSparkle( fract8 chanceOfGlitter) {
	FastLED.clear(true);
	if( random8() < chanceOfGlitter) {
		fLeft[ random16(FLEFT_COUNT) ] += CRGB::White;
		fRight[ random16(FRIGHT_COUNT) ] += CRGB::White;
		mid[ random16(MID_COUNT) ] += CRGB::White;
	}
	FastLED.show();
}

/**
 * All rainbow.
 */
void rainbow() {
	black();
	fill_rainbow(fLeft, FLEFT_COUNT, 0);
	fill_rainbow(fRight, FRIGHT_COUNT, 0);
	fill_rainbow(mid, MID_COUNT, 0);
	FastLED.show();
}

/**
 * Turn on sinelon.
 */
void sinelon() {
	black();
	shouldSinelon = true;
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

void reverseMid2() {
	for (uint i = MID1_COUNT; i < MID1_COUNT + (MID2_COUNT / 2); i++) {
		CRGB temp = mid[i];
		mid[i] = mid[MID_COUNT - (i - MID1_COUNT) - 1];
		mid[MID_COUNT - (i - MID1_COUNT) - 1] = temp;
	}
}

void cylonLoop(CRGB* leds, int numLeds) {
  EVERY_N_SECONDS(5){  // Demo: Change direction every N seconds.
    delta = -1*delta;
  }

  EVERY_N_SECONDS(2){  // Demo: Change the hue every N seconds.
    hue = hue + random8(30,61);  // Shift the hue to something new.
  }

  EVERY_N_MILLISECONDS(holdTime){  // Advance pixels to next position.
	reverseMid2();
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
	reverseMid2();
    FastLED.show();
  }
}

void setBrightness(byte* bytes = 0) {
	/*
	uint8_t brightness = uint8_t(
            (unsigned char)(bytes[0]) << 24 |
            (unsigned char)(bytes[1]) << 16 |
            (unsigned char)(bytes[2]) << 8 |
            (unsigned char)(bytes[3]));
			*/
	uint8_t brightness = 55;
	Serial.println("Setting brightness to:");
	Serial.println(brightness);
	FastLED.setBrightness(brightness);
}

/**
 * Setup.
 */
void ledSetup() {
	//Front Left (15 leds)
	FastLED.addLeds<STRIP_TYPE, FLEFT_PIN, LED_ORDER>(fLeft, FLEFT_COUNT).setCorrection(TypicalSMD5050);
	//Front Right (15 leds)
	FastLED.addLeds<STRIP_TYPE, FRIGHT_PIN, LED_ORDER>(fRight, FRIGHT_COUNT).setCorrection(TypicalSMD5050);
	//Middle1 (92 leds, starting at index 0 of mid)
	FastLED.addLeds<STRIP_TYPE, MID1_PIN, LED_ORDER>(mid, 0, MID1_COUNT).setCorrection(TypicalSMD5050);
	//Middle2 (32 leds, starting at index 92 of mid)
	FastLED.addLeds<STRIP_TYPE, MID2_PIN, LED_ORDER>(mid, MID1_COUNT, MID2_COUNT).setCorrection(TypicalSMD5050);

	setBrightness();
	black();
	canrun.setupDelay('s', 100);
	sinelon();
}

/**
 * Loop.
 */
void ledLoop() {
	if (canrun.run('s')) {
		if (shouldSparkle == true) {
			doSparkle(100);
		}
	}
	if (shouldSinelon == true) {
		cylonLoop(mid, MID_COUNT);
	}
}