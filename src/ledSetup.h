#include <FastLED.h>
#include <Canrun.h>

//15, 15, 32, 92 (32 + 16 + 14 + 14 + 16)
#define STRIP1_COUNT 15
#define STRIP2_COUNT 92
#define STRIP3_COUNT 32
#define STRIP4_COUNT 15
#define STRIP1_PIN 23
#define STRIP2_PIN 19
#define STRIP3_PIN 5
#define STRIP4_PIN 4
#define STRIP_TYPE WS2812B
#define LED_ORDER GRB

Canrun canrun;

CRGB strip1[STRIP1_COUNT];
CRGB strip2[STRIP2_COUNT];
CRGB strip3[STRIP3_COUNT];
CRGB strip4[STRIP4_COUNT];

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
bool shouldSparkle = false;
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
		strip1[ random16(STRIP1_COUNT) ] += CRGB::White;
		strip2[ random16(STRIP2_COUNT) ] += CRGB::White;
		strip3[ random16(STRIP3_COUNT) ] += CRGB::White;
		strip4[ random16(STRIP4_COUNT) ] += CRGB::White;
	}
	FastLED.show();
}

/**
 * All rainbow.
 */
void rainbow() {
	black();
	fill_rainbow(strip1, STRIP1_COUNT, 0);
	fill_rainbow(strip2, STRIP2_COUNT, 0);
	fill_rainbow(strip3, STRIP3_COUNT, 0);
	fill_rainbow(strip4, STRIP4_COUNT, 0);
	FastLED.show();
}

/**
 * Turn on sinelon.
 */
bool shouldSinelon = false;
void sinelon() {
	black();
	shouldSinelon = true;
}

/**
 * Called by loop to do the sinelon.
 */
void doSinelon() {
  fadeToBlackBy( strip1, STRIP1_COUNT, 20);
  fadeToBlackBy( strip2, STRIP2_COUNT, 20);
  fadeToBlackBy( strip3, STRIP3_COUNT, 20);
  fadeToBlackBy( strip4, STRIP4_COUNT, 20);
  int pos1 = beatsin16( 13, 0, STRIP1_COUNT - 1 );
  int pos2 = beatsin16( 13, 0, STRIP2_COUNT - 1 );
  int pos3 = beatsin16( 13, 0, STRIP3_COUNT - 1 );
  int pos4 = beatsin16( 13, 0, STRIP4_COUNT - 1 );
  strip1[pos1] += CHSV( 0, 255, 192);
  strip2[pos2] += CHSV( 0, 255, 192);
  strip3[pos3] += CHSV( 0, 255, 192);
  strip4[pos4] += CHSV( 0, 255, 192);
  FastLED.show();
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
 * All a specific color, called internally.
 */
void doShowColor(CRGB color) {
	FastLED.showColor(color);
}

void setBrightness(byte* bytes = 0) {
	/*
	uint8_t brightness = uint8_t(
            (unsigned char)(bytes[0]) << 24 |
            (unsigned char)(bytes[1]) << 16 |
            (unsigned char)(bytes[2]) << 8 |
            (unsigned char)(bytes[3]));
			*/
	uint8_t brightness = 255;
	Serial.println("Setting brightness to:");
	Serial.println(brightness);
	FastLED.setBrightness(brightness);
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
 * Setup.
 */
void ledSetup() {
	FastLED.addLeds<STRIP_TYPE, STRIP1_PIN, LED_ORDER>(strip1, STRIP1_COUNT).setCorrection(TypicalSMD5050);
	FastLED.addLeds<STRIP_TYPE, STRIP2_PIN, LED_ORDER>(strip2, STRIP2_COUNT).setCorrection(TypicalSMD5050);
	FastLED.addLeds<STRIP_TYPE, STRIP3_PIN, LED_ORDER>(strip3, STRIP3_COUNT).setCorrection(TypicalSMD5050);
	FastLED.addLeds<STRIP_TYPE, STRIP4_PIN, LED_ORDER>(strip4, STRIP4_COUNT).setCorrection(TypicalSMD5050);

	setBrightness();
	black();
	canrun.setupDelay('s', 100);
}

/**
 * Loop.
 */
void ledLoop() {
	if (canrun.run('s')) {
		if (shouldSparkle == true) {
			doSparkle(100);
		}
		if (shouldSinelon == true) {
			doSinelon();
		}
	}
}