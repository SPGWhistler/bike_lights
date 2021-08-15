#include <FastLED.h>
#include <unordered_map>

//15, 15, 92 (32 + 16 + 14 + 14 + 16)
#define STRIP1_COUNT 15
#define STRIP2_COUNT 92
#define STRIP3_COUNT 15
#define STRIP1_PIN 23
#define STRIP2_PIN 19
#define STRIP3_PIN 4
#define STRIP_TYPE WS2812B
#define LED_ORDER GRB
#define BRIGHTNESS 50

CRGB strip1[STRIP1_COUNT];
CRGB strip2[STRIP2_COUNT];
CRGB strip3[STRIP3_COUNT];

std::unordered_map<char, unsigned long> delays{};
std::unordered_map<char, unsigned long> times{};
void setupDelay(char key, unsigned long delay) {
	delays[key] = delay;
	times[key] = 0;
}
unsigned long currentMillis;
bool canRun(char key) {
	currentMillis = millis();
	if (currentMillis - times[key] >= delays[key]) {
		times[key] = currentMillis;
		return true;
	}
	return false;
}

void red(){
	FastLED.showColor(CRGB::Red);
}

void blue() {
	FastLED.showColor(CRGB::Blue);
}

void green() {
	FastLED.showColor(CRGB::Green);
}

void white() {
	FastLED.showColor(CRGB::White);
}

void doSparkle( fract8 chanceOfGlitter) {
	FastLED.clear(true);
	if( random8() < chanceOfGlitter) {
		strip1[ random16(STRIP1_COUNT) ] += CRGB::White;
		strip2[ random16(STRIP2_COUNT) ] += CRGB::White;
		strip3[ random16(STRIP3_COUNT) ] += CRGB::White;
	}
	FastLED.show();
}

bool shouldSparkle = false;
const long sparkleDelay = 100;
void sparkle() {
	shouldSparkle = true;
}

void rainbow() {
	fill_rainbow(strip1, STRIP1_COUNT, 0);
	fill_rainbow(strip2, STRIP2_COUNT, 0);
	fill_rainbow(strip3, STRIP3_COUNT, 0);
	FastLED.show();
}

void black() {
	FastLED.showColor(CRGB::Black);
	shouldSparkle = false;
}

void show_color(byte* bytes) {
	CRGB color;
	color.r = bytes[1];
	color.g = bytes[2];
	color.b = bytes[3];
	FastLED.showColor(color);
}


void ledSetup() {
	FastLED.addLeds<STRIP_TYPE, STRIP1_PIN, LED_ORDER>(strip1, STRIP1_COUNT).setCorrection(TypicalSMD5050);
	FastLED.addLeds<STRIP_TYPE, STRIP2_PIN, LED_ORDER>(strip2, STRIP2_COUNT).setCorrection(TypicalSMD5050);
	FastLED.addLeds<STRIP_TYPE, STRIP3_PIN, LED_ORDER>(strip3, STRIP3_COUNT).setCorrection(TypicalSMD5050);

	FastLED.setBrightness(BRIGHTNESS);

	black();
	setupDelay('s', 100);
}
void ledLoop() {
	if (canRun('s')) {
		if (shouldSparkle == true) {
			doSparkle(100);
		}
	}
}