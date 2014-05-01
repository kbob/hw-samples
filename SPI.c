#include "SPI.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include <avr/interrupt.h>

#include "ATX_power.h"
#include "timer.h"

#define PIXEL_COUNT 22

uint8_t sin_table[500];

static void init_table(void)
{
#if 0
    for (uint16_t i = 0; i < 500; i++) {
        float omega = (float)i * M_PI / (float)500;
        float s = sin(omega);
        assert(s >= 0 && s <= 1);
        uint8_t t = 0x7F * s;
        t |= 0x80;
        sin_table[i] = t;
    }
#else
    for (uint16_t i = 0; i < 500; i++) {
        float omega = (float)i * 2 * M_PI / (float)500;
        float s = (1.0 - cos(omega)) / 2;
        assert(s >= 0 && s <= 1);
        uint8_t t = 0x7F * s;
        t |= 0x80;
        sin_table[i] = t;
    }
#endif
}

int main()
{
    init_timer();
    init_SPI();
    init_ATX_power();
    sei();

    enable_ATX_power();
    delay_milliseconds(100);
    SPI_write_byte('\0');
    init_table();

#if 1
    // 2.5 second sine pulse, then 2.5 seconds dark
    for (int j = 0; ; j++) {
        uint8_t v;
        if (j == 1000)
            j = 0;
        if (j < 500)
            v = sin_table[j];
        else
            v = 0x80;
        for (uint8_t i = 0; i < 3 * PIXEL_COUNT; i++)
            SPI_write_byte(v);
        SPI_write_byte(0x00);
        delay_milliseconds(5);
    }
#else
    uint32_t t = millisecond_time() + 10;
    for (int j = 0; ; j++) {
        uint8_t v;
        if (j == 1000)
            j = 0;
        for (uint8_t i = 0; i < 3 * PIXEL_COUNT; i++) {
            uint16_t k = i;
            k ^= 0x55;
            k = k < 33 ? 33 - k : k - 34;
            k = j - k;
            if (k >= 0 && k < 500) {
                v = sin_table[k];
                if (j / 4 % i < i / 8)
                    v = (v & 0x7f) / 3 | 0x80;
            } else
                v = 0x80;
            // v = (j < 500) ? 0xFF : 0x9F;
            SPI_write_byte(v);
        }
        SPI_write_byte(0x00);
        //delay_milliseconds(10);
        while (millisecond_time() < t)
            continue;
        t += 10;
    }
#endif
}
