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
	void setupDelay(char key, unsigned long delay);
	bool run(char key);
  private:
	std::unordered_map<char, unsigned long> _delays{};
	std::unordered_map<char, unsigned long> _times{};
};

#endif