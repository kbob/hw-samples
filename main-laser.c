#include "main-laser.h"

#include <avr/interrupt.h>

#include <stdio.h>

#include "safety-switches.h"
#include "serial.h"
#include "stdio_util.h"
#include "timer.h"

#define PULSE_MS 100

int main()
{
    init_timer();
    init_serial();
    init_stdio();
    init_safety_switches();
    sei();

    printf("Main Laser!  Danger!\n");

    while (true) {
        if (getchar() == '\r') {
            if (e_is_stopped())
                printf("Emergency Stop.  No fire.\n");
            else if (lid_is_open())
                printf("Lid is open.  No fire.\n");
            else {
                printf("Fire!\n");
                enable_main_laser();
                delay_milliseconds(PULSE_MS);
                disable_main_laser();
            }
        }
    }
}
