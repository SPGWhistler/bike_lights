#include <FastLED.h>

#define STRIP1_COUNT 15
#define DATA_PIN 23
#define STRIP_TYPE WS2812B
#define LED_ORDER GRB

CRGB strip1[STRIP1_COUNT];

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

void rainbow() {
	fill_rainbow(strip1, 15, 0, 255);
	FastLED.show();
}

void black() {
	FastLED.showColor(CRGB::Black);
}

void ledSetup() {
	FastLED.addLeds<STRIP_TYPE, DATA_PIN, LED_ORDER>(strip1, STRIP1_COUNT);
	black();
}
