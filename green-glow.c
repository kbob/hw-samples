#include <avr/interrupt.h>

#include "ATX_power.h"
#include "LED-util.h"
#include "SPI.h"
#include "serial.h"
#include "stdio_util.h"
#include "timer.h"

int main()
{
    init_timer();
    init_serial();
    init_SPI();
    init_ATX_power();
    sei();

    enable_ATX_power();
    repeat_LEDs_off();
    init_stdio();

    ramp_LEDs(0, 127, 1000);
    soothing_green_glow();
}
