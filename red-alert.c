#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ATX_power.h"
#include "serial.h"
#include "stdio_util.h"
#include "SPI.h"
#include "timer.h"

#define PIXEL_COUNT 22
#define UPDATE_MS 10

static void set_pixel_color(int pixel, int r, int g, int b)
{
    SPI_write_byte(g | 0x80);
    SPI_write_byte(r | 0x80);
    SPI_write_byte(b | 0x80);
}

static void update_LEDs(void)
{
    // Red Alert!
    static int n = 0;
    for (int i = 0; i < PIXEL_COUNT; i++) {
        if (n < 20) {
            int m = abs(10 - n);
            set_pixel_color(i, 27 + 10 * m, 0, 0);
        } else if (n == 35)
            set_pixel_color(i, 63, 63, 63);
        else
            set_pixel_color(i, 0, 0, 0);
     }
    SPI_write_byte(0x00);
    n++;
    if (n == 55)
        n = 0;
}

static void set_LEDs(uint8_t level)
{
    SPI_write_byte(0x00);
    for (uint8_t i = 0; i < 3 * PIXEL_COUNT; i++)
        SPI_write_byte(level | 0x80);
    SPI_write_byte(0x00);
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
        set_LEDs(0);
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
