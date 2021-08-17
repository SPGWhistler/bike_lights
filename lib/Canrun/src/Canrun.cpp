/*
  Canrun.cpp - An easy way to run a function in the loop every x milliseconds.
  Created by Anthony Petty
*/

#include "Canrun.h"
#include "Arduino.h"
#include <unordered_map>

Canrun::Canrun() {
	std::unordered_map<char, unsigned long> _delays{};
	std::unordered_map<char, unsigned long> _times{};
}

void Canrun::setupDelay(char key, unsigned long delay) {
	_delays[key] = delay;
	_times[key] = 0;
}

bool Canrun::run(char key) {
	unsigned long currentMillis = millis();
	if (currentMillis - _times[key] >= _delays[key]) {
		_times[key] = currentMillis;
		return true;
	}
	return false;
}
