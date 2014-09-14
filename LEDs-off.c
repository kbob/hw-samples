#include "SPI.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "ATX_power.h"
#include "LED-util.h"
#include "timer.h"

#define FRAME_MS 1000

int main()
{
    init_timer();
    init_SPI();
    init_ATX_power();
    sei();

    enable_ATX_power();
    repeat_LEDs_off();

    for (int i = 0; i < 10; i++)
        repeat_LEDs_off();
    disable_ATX_power();
    while (1)
        continue;
}
