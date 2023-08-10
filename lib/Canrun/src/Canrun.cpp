/*
  Canrun.cpp - An easy way to run a function in the loop every x milliseconds.
  Created by Anthony Petty
*/

#include "Canrun.h"
#include "Arduino.h"
#include <unordered_map>

Canrun::Canrun() {
	std::unordered_map<char, uint16_t> _delays{};
	std::unordered_map<char, uint16_t> _times{};
}

/**
 * This is called to setup the delay with a key and a delay.
 * You can also call it to change a delay.
 * If you set delay to 0, run will always return false.
*/
void Canrun::setupDelay(char key, uint16_t delay) {
	_delays[key] = delay;
	_times[key] = 0;
}

/**
 * This is called on every loop with the key.
 * If it returns true, your code should execute.
 * If it returns false, you should not execute your code.
*/
bool Canrun::run(char key) {
	uint16_t currentMillis = millis();
  if (_delays[key] > 0) {
    if (currentMillis - _times[key] >= _delays[key]) {
      _times[key] = currentMillis;
      return true;
    }
  }
	return false;
}
