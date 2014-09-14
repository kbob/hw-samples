#include <stdint.h>
#include <stdlib.h>

#include <avr/interrupt.h>

#include "ATX_power.h"
#include "LED-util.h"
#include "SPI.h"
#include "timer.h"

#define PIXEL_COUNT 22
#define STRIPE_MS 1000
#define IDLE_MS 800
#define STRIPE_WIDTH 4

static void wait_for(uint32_t ms)
{
    static uint32_t last_time;
    static bool been_here = false;
    if (!been_here) {
        last_time = millisecond_time();
        been_here = true;
    }
    last_time += ms;
    uint32_t now;
    while ((int32_t)(now = millisecond_time()) - (int32_t)last_time < 0)
        continue;
}

int main()
{
    init_timer();
    init_SPI();
    init_ATX_power();
    sei();

    enable_ATX_power();
    repeat_LEDs_off();

    uint16_t refresh_ms = STRIPE_MS / PIXEL_COUNT;

    while (true) {
        for (uint8_t i = 0; i < PIXEL_COUNT; i++) {
            uint8_t j = abs(PIXEL_COUNT / 2 - i);
            uint8_t k = PIXEL_COUNT - j - 1;
            uint8_t j1 = (2 * j + PIXEL_COUNT) / 4;
            uint8_t k1 = (2 * k + PIXEL_COUNT) / 4;
            begin_LEDs_refresh();
            for (uint8_t p = 0; p < PIXEL_COUNT; p++) {
                uint8_t r = 127, g = 127, b = 127;
                r = g = b = 63;
               if ((j <= p && p < j1) || (k1 <= p && p < k)) {
                    r = 127;
                    g = 63;
                    b = 0;
                }
                set_pixel_color(r, g, b);
            }
            end_LEDs_refresh();
            wait_for(refresh_ms);
        }
        set_LEDs_level(63);
        wait_for(IDLE_MS);
    }
}
