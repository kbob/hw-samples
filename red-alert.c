#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ATX_power.h"
#include "LED-util.h"
#include "serial.h"
#include "stdio_util.h"
#include "SPI.h"
#include "timer.h"

#define UPDATE_MS 20

static void update_LEDs(void)
{
    // Red Alert!
    static int8_t n = 0;
    begin_LEDs_refresh();
    for (uint8_t i = 0; i < PIXEL_COUNT; i++) {
        if (n <= 20) {
            uint8_t m = abs(10 - n);
            set_pixel_color(27 + 10 * m, 0, 0);
        } else if (n == 35)
            set_pixel_color(63, 63, 63);
        else
            set_pixel_color(0, 0, 0);
    }
    end_LEDs_refresh();
    n++;
    if (n == 55)
        n = 0;
}

int main()
{
    init_serial();
    init_stdio();
    init_timer();
    init_SPI();
    init_ATX_power();
    sei();

    enable_ATX_power();
    int counter = 10;
    while (true) {
        bool lv_ready = ATX_power_state();
        set_LEDs_level(0);
        if (lv_ready && !--counter)
            break;
    }

    uint32_t t = millisecond_time() + UPDATE_MS;
    while (true) {
        if (t - millisecond_time() > 0)
            continue;
        t += UPDATE_MS;

        putchar('.');
        update_LEDs();
    }
}
