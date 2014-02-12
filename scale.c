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

#define HOME_IVL 2032

#define PLAY_MSEC       800
#define REST_MSEC       200
#define LONG_REST_MSEC 1000

uint16_t ivls[] = {
    61156,                      // Do
    // 57724,
    54484,                      // Re
    // 51426,
    48540,                      // Mi
    45815,                      // Fa
    // 43244,
    40817,                      // Sol
    // 38526,
    36364,                      // La
    // 34323,
    32396,                      // Ti
    30578,                      // Do
};
uint8_t note_count = sizeof ivls / sizeof ivls[0];

static void y_intr(void)
{
    static bool will_home = true;
    static bool homing;
    static uint8_t note;
    static uint16_t rest;
    static uint32_t play;

    if (will_home) {
        will_home = false;
        // Begin homing.
        set_y_direction_negative();
        enable_y_motor();
        load_y_interval(HOME_IVL);
        homing = true;
        return;
    }
    if (homing) {
        if (y_min_reached()) {
            homing = false;
            note = 0;
            play = 0;
            rest = LONG_REST_MSEC;
            set_y_direction_positive();
            disable_y_motor();
            load_y_interval(F_CPU / 1000);
        }
        return;
    }
    if (rest) {
        if (!--rest) {
            if (note == note_count) {
                note = 0;
                will_home = true;
            } else {
                enable_y_motor();
                load_y_interval(ivls[note]);
                play = PLAY_MSEC * (F_CPU / 1000);
            }
        }
    }
    if (play) {
        if (play > ivls[note])
            play -= ivls[note];
        else {
            play = 0;
            rest = REST_MSEC;
            disable_y_motor();
            load_y_interval(F_CPU / 1000);
            ++note;
        }
    }
}

int main()
{
    for (int i = 0; i < note_count; i++)
        ivls[i] /= 8;
    // ivls[4] -= 2;
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
    enable_y_motor();
    home_y();
    set_y_direction_positive();
    init_y_timer(y_intr, 1 * F_CPU / 1000);
    start_y_timer();

    while (true)
        continue;
}
