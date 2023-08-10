/*
  Canrun.h - An easy way to run a function in the loop every x milliseconds.
  Created by Anthony Petty
*/
#ifndef Canrun_h
#define Canrun_h

#include <unordered_map>

class Canrun {
  public:
    Canrun();
	void setupDelay(char key, uint16_t delay);
	bool run(char key);
  private:
	std::unordered_map<char, uint16_t> _delays{};
	std::unordered_map<char, uint16_t> _times{};
};

#endif