#include <stdio.h>
#include "inc/global.h"
#include "driver/gpio.h"
#include "inc/UtilityGPIO.h"

void on(uint8_t PIN) {
    gpio_set_level(PIN, true);
}

void off(uint8_t PIN) {
    gpio_set_level(PIN, false);
}

void setPin(uint8_t PIN, bool status) {
    gpio_set_level(PIN, status);
}