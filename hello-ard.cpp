#include "Arduino.h"

uint32_t t;
char buf[256] __attribute__((__aligned__(256)));


void setup(void)
{
    Serial.begin(9600);
    t = millis() + 1000;
    buf[0] = 0;
}

void loop(void)
{
    if (millis() < t)
        return;
    t += 1000;
    Serial.println("Hello, World!");
}
