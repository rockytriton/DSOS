#include "gpio.h"

void gpio_pin_mode(byte pinNumber, PinMode pm) {
    byte functionIndex = pinNumber / 10;
    byte pinIndex = pinNumber % 10;
    
    GPIO()->pinModes[functionIndex] &= ~(0b111 << (pinIndex * 3));
    GPIO()->pinModes[functionIndex] |= (pm << (pinIndex * 3));
}

void gpio_enable_pupd() {
    GPIO()->pupdEnable = 0;
    delay(150);
    GPIO()->pupdClock0 |= (1 << 14) | (1 << 15);
    delay(150);
    GPIO()->pupdClock0 = 0;

    delay(150);
    GPIO()->pupdEnable = 2;
    delay(150);
    GPIO()->pupdClock0 |= (1 << 18);
    delay(150);
    GPIO()->pupdClock0 = 0;
}

void gpio_write(byte pinNumber, bool val) {
    if (val) {
        GPIO()->outputSet.values[0] |= (1 << pinNumber);
    } else {
        GPIO()->outputClear.values[0] |= (1 << pinNumber);
    }
}

