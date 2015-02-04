/*
 * Spinner firmware, sends back raw color data over MIDI.
 * We do the color-to-angle calculations back in CircleEngine.
 *
 * For the Teensy 2.0, in USB MIDI device mode.
 *
 * Each Teensy can support many spinners.
 * Below, create a Spinner instance for each one,
 * giving it unique pin numbers and MIDI controller number.
 * Call each spinner's setup() and loop() function.
 */

#include "spinner.h"

Spinner spin1(22, 23, 10);

const int LED = 13;

void setup()
{
 	spin1.setup();

 	pinMode(LED, OUTPUT);
}

void loop()
{
 	spin1.loop();

	// Read and ignore incoming MIDI messages
	while (usbMIDI.read());

 	// Blink the LED
 	digitalWrite(LED, (millis() % 1000) < 100);
}
