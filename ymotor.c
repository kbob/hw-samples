#include "motors.h"

#include <avr/interrupt.h>

#include "ATX_power.h"
#include "timer.h"

#define STROKE_LENGTH ((int)(100 / 25.4 * 2000))

static void stroke(void)
{
    for (int i = 0; i < STROKE_LENGTH; i++) {
        step_y();
        delay_milliseconds(1);
    }
}

int main()
{
    init_ATX_power();
    init_timer();
    init_motors();
    sei();

    enable_ATX_power();
    enable_y_motor();

    while (1) {
        set_y_direction_positive();
        stroke();
        delay_milliseconds(100);
        set_y_direction_negative();
        stroke();
        delay_milliseconds(1000);
    }        
}
