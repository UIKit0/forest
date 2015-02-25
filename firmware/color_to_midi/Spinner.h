/*
 * The Spinner object uses a color sensor to measure the angle
 * of a single spinner and report the results back over MIDI.
 *
 * (C) 2015 Micah Elizabeth Scott, released under the MIT license.
 */

#pragma once

#include "Adafruit_TCS34725_SoftI2C.h"

class Spinner
{
public:
    Spinner(int scl, int sda, int controller, int channel = 1);

    void setup();
    void loop();

private:
    SoftI2CMaster i2c;
    Adafruit_TCS34725 sensor;
    uint8_t controller, channel;
    uint16_t last_r, last_g, last_b, last_c;
};
