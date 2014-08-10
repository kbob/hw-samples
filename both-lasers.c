#include "main-laser.h"

#include <avr/interrupt.h>

#include <stdio.h>

#include "ATX_power.h"
#include "heaters.h"
#include "i2c.h"
#include "laser-power.h"
#include "safety-switches.h"
#include "serial.h"
#include "stdio_util.h"
#include "timer.h"
#include "visible-laser.h"

#define MAIN_PULSE_MS 100
#define VIS_PULSE_MS   40
#define VIS_OFF_MS     80

int main()
{
    init_timer();
    init_ATX_power();
    init_serial();
    init_stdio();
    init_safety_switches();
    init_i2c();
    init_laser_power();
    init_heaters();
    init_main_laser();
    sei();

    enable_ATX_power();
    uint32_t b = millisecond_time();
    while (!ATX_power_state()) {
        enable_ATX_power();
        continue;
    }
    uint32_t a = millisecond_time();
    printf("ATX power: %ld msec\n", a - b);
    enable_heater_1();          // aka water pump
    delay_milliseconds(2000);
    enable_heater_0();          // aka high voltage supply
    set_laser_power(4095 / 3);  // 1/3rd power
    printf("Main Laser!  Danger!\n");

    while (true) {
        if (lid_is_open()) {
            enable_visible_laser();
            delay_milliseconds(VIS_PULSE_MS);
            disable_visible_laser();
            delay_milliseconds(VIS_OFF_MS);
        } else if (serial_data_ready() && getchar() == '\r') {
            if (e_is_stopped())
                printf("Emergency Stop.  No fire.\n");
            else if (lid_is_open())
                printf("Lid is open.  No fire.\n");
            else {
                printf("Fire!\n");
                enable_main_laser();
                delay_milliseconds(MAIN_PULSE_MS);
                disable_main_laser();
            }
        }
    }
}
