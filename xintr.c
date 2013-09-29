#include "motors.h"

#include <stdio.h>

#include <avr/interrupt.h>

#include "ATX_power.h"
#include "LED-util.h"
#include "limit-switches.h"
#include "motor-util.h"
#include "serial.h"
#include "SPI.h"
#include "stdio_util.h"
#include "timer.h"

#define STROKE_LENGTH ((int)(300 / 25.4 * 2000))
#define SPEED 100               // mm/sec

#define DELAY0 1000
#define DELAY1  200

static void x_intr(void)
{
    static int8_t dir = +1;
    static uint16_t distance;;
    static uint16_t delay = 1;

    if (delay) {
        if (!--delay) {
            enable_x_step();
            load_x_interval(TICKS_per_uSTEP(SPEED));
            distance = STROKE_LENGTH;
        }
    } else {
        if (!--distance) {
            load_x_interval(16000);
            disable_x_step();
            if ((dir = -dir) < 0) {
                set_x_direction_negative();
                delay = DELAY1;
            } else {
                set_x_direction_positive();
                delay = DELAY0;
            }
        }
    }
}

int main()
{
    init_stdio();
    init_serial();
    init_ATX_power();
    init_limit_switches();
    init_timer();
    init_motors();
    init_SPI();
    sei();

    enable_ATX_power();
    repeat_LEDs_off();
    enable_x_motor();
    home_x();
    ramp_LEDs(0, 127, 1000);
    set_x_direction_positive();
    init_x_timer(x_intr, 1 * F_CPU / 1000);
    start_x_timer();

    while (1)
        // Extra credit: put the CPU to sleep.
        continue;
}
