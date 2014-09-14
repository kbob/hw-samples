#include <stdint.h>
#include <stdlib.h>

#include <avr/interrupt.h>

#include "ATX_power.h"
#include "LED-util.h"
#include "SPI.h"
#include "timer.h"

#define ROTATION_SECONDS 3600

static void wait_for_it(uint32_t ms)
{
    static uint32_t nt;
    static uint32_t interval;
    if (ms) {
        interval = ms;
        nt = millisecond_time() + interval;
    } else {
        while (((int32_t)millisecond_time() - (int32_t)nt) < 0)
            continue;
        nt += interval;
    }
}

static void calc_color(uint8_t i, uint8_t j,
                       uint8_t *rp, uint8_t *gp, uint8_t *bp)
{
    switch (i) {

    case 0:         // red -> yellow
        *rp = 127;
        *gp = j;
        *bp = 0;
        break;

    case 1:         // yellow -> green
        *rp = 127 - j;
        *gp = 127;
        *bp = 0;
        break;

    case 2:         // green -> cyan
        *rp = 0;
        *gp = 127;
        *bp = j;
        break;

    case 3:         // cyan -> blue
        *rp = 0;
        *gp = 127 - j;
        *bp = 127;
        break;

    case 4:         // blue -> magenta
        *rp = j;
        *gp = 0;
        *bp = 127;
        break;

    case 5:         // magenta -> red
        *rp = 127;
        *gp = 0;
        *bp = 127 - j;
        break;
    }
}

int main()
{
    init_timer();
    init_SPI();
    init_ATX_power();
    sei();

    enable_ATX_power();
    repeat_LEDs_off();

    uint32_t d = ROTATION_SECONDS * 1000L / (6 * 127);
    wait_for_it(d);
    
#if 0
    while (true) {
        for (uint8_t i = 0; i < 6; i++) {
            for (uint8_t j = 0; j < 127; j++) {
                wait_for_it(0);
                uint8_t r, g, b;
                calc_color(i, j, &r, &g, &b);
                set_LEDs_color(r, g, b);
            }
        }
    }
#else
    while (true) {
        for (uint8_t i = 0; i < 6; i++) {
            for (uint8_t j = 0; j < 127; j++) {
                wait_for_it(0);
                begin_LEDs_refresh();
                for (uint8_t p = 0; p < PIXEL_COUNT; p++) {
                    uint8_t r=0, g=0, b=0;
                    uint8_t ii = i, jj = j + 4 * p;
                    if (jj >= 127) {
                        jj -= 127;
                        if (++ii == 6) ii = 0;
                    }
                    calc_color(ii, jj, &r, &g, &b);
                    set_pixel_color(r, g, b);
                }                
                end_LEDs_refresh();
            }
        }
    }
#endif
}
