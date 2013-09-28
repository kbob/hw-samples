#include "visible-laser.h"

#include <avr/interrupt.h>

#include "timer.h"

int main()
{
    init_timer();
    init_visible_laser();
    sei();
    uint16_t d = 40;

    while (1) {
        enable_visible_laser();
        delay_milliseconds(d);
        disable_visible_laser();
        delay_milliseconds(d);
        // d = 2 * d;
        // if (d > 64)
        //     d = 1;
    }
}
