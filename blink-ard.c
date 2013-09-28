/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  This example code is in the public domain.
*/

#include "Arduino.h"

const int LED_PIN = 13;

void setup(void)
{
    pinMode(LED_PIN, OUTPUT);
}

void loop(void)
{
    digitalWrite(LED_PIN, HIGH);
    delay(400);
    digitalWrite(LED_PIN, LOW);
    delay(100);
}
