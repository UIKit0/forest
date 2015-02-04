/*
 * The Spinner object uses a color sensor to measure the angle
 * of a single spinner and report the results back over MIDI.
 *
 * (C) 2015 Micah Elizabeth Scott, released under the MIT license.
 */

#include "Spinner.h"

Spinner::Spinner(int scl, int sda, int controller, int channel) :
	i2c(scl, sda),
	sensor(i2c, TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_60X),
	controller(controller),
	channel(channel)
{
    last_r = last_g = last_b = last_c = -1;
}

void Spinner::setup()
{
	  sensor.begin();
}

void Spinner::loop()
{
	  uint16_t raw_r, raw_g, raw_b, raw_c;
  	sensor.getRawData(&raw_r, &raw_g, &raw_b, &raw_c);

    if (raw_r != last_r || raw_g != last_g || raw_b != last_b) {
        last_r = raw_r;
        last_g = raw_g;
        last_b = raw_b;
        last_c = raw_c;

      	usbMIDI.sendControlChange(controller, raw_r & 0x7F, channel + 0);
      	usbMIDI.sendControlChange(controller, raw_r >> 7, channel + 1);
      	usbMIDI.sendControlChange(controller, raw_g & 0x7F, channel + 2);
      	usbMIDI.sendControlChange(controller, raw_g >> 7, channel + 3);
      	usbMIDI.sendControlChange(controller, raw_b & 0x7F, channel + 4);
      	usbMIDI.sendControlChange(controller, raw_b >> 7, channel + 5);
      	usbMIDI.sendControlChange(controller, raw_c & 0x7F, channel + 6);
      	usbMIDI.sendControlChange(controller, raw_c >> 7, channel + 7);
    }
}
