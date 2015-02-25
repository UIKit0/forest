/*
 * Spinner debug firmware. For a single color sensor, sends
 * human-readable color data over serial.
 *
 * For the Teensy 2.0, in USB Serial device mode.
 */

#include "Adafruit_TCS34725_SoftI2C.h"

//                 SCL  SDA
SoftI2CMaster i2c( 0,   1   );

Adafruit_TCS34725 sensor(i2c, TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_16X);


void setup()
{
    Serial.begin(9600);
    sensor.begin();
}


void loop()
{
    uint16_t raw_r = 0, raw_g = 0, raw_b = 0, raw_c = 0;
    sensor.getRawData(&raw_r, &raw_g, &raw_b, &raw_c);

    Serial.print("r=");
    Serial.print(raw_r);

    Serial.print(" g=");
    Serial.print(raw_g);

    Serial.print(" b=");
    Serial.print(raw_b);

    Serial.print(" c=");
    Serial.println(raw_c);

    delay(500);
}
