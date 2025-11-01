#include "Hiwonder.hpp"
#include <Arduino.h>

void Battery_t::init(uint8_t pin)
{
    battery_pin = pin;
    analogReadResolution(12); //set the resolution to 12 bits (0-4095)
}

int Battery_t::read_mV(void)
{
  int analogVolts = analogReadMilliVolts(battery_pin) * 11 - 90;
  return analogVolts;
}

