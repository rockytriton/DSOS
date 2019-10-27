#include "gpio.h"
#include "logger.h"
#include "utils.h"
#include "timer.h"

using namespace dsos::gpio;

static dsos::Logger &logger = dsos::Logger::inst();

void dsos::gpio::setPinMode(byte pinNumber, PinMode pf) {
    byte functionIndex = pinNumber / 10;
    byte pinIndex = pinNumber % 10;
    
    GPIO()->pinModes[functionIndex] &= ~(0b111 << (pinIndex * 3));
    GPIO()->pinModes[functionIndex] |= (pf << (pinIndex * 3));
}

//bool on18 = false;

extern "C" void onTimer() {

}