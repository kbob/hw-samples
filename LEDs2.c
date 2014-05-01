#include <stdlib.h>

#include <avr/interrupt.h>

#include "ATX_power.h"
#include "SPI.h"
#include "serial.h"
#include "timer.h"

#define PIXEL_COUNT 22
#define FRAME_MS 2
#define RAMP_FRAMES (2 * 1000 / FRAME_MS)
//#define MAX_BRIGHTNESS 4
#define MAX_BRIGHTNESS 0x7f

const uint8_t dither_matrix[16] = {
    0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13
};

uint32_t next_update_time;

void delay(void)
{
    while ((int16_t)millisecond_time() - (int16_t)next_update_time < 0)
        continue;
    next_update_time += FRAME_MS;
}

void set_LEDs_white(uint8_t b)
{
    b |= 0x80;
    for (uint8_t i = 0; i < 3 * PIXEL_COUNT; i++)
        SPI_write_byte(b);
    SPI_write_byte(0x00);
    delay();
}

void set_LEDs_CMY(uint8_t b)
{
    b |= 0x80;
    for (uint8_t i = 0; i < 3 * PIXEL_COUNT; i++)
        SPI_write_byte(i & 3 ? b : 0x80);
    SPI_write_byte(0x00);
    delay();
}

void set_LEDs_RGB(uint8_t b)
{
    b |= 0x80;
    for (uint8_t i = 0; i < 3 * PIXEL_COUNT; i++)
        SPI_write_byte(i & 3 ? 0x80 : b);
    SPI_write_byte(0x00);
    delay();
}

void abort()
{
    disable_ATX_power();
    while (true)
        ;
}

void ramp(uint8_t b0, uint8_t b1, void (*set_LEDs)(uint8_t))
{
    int df = RAMP_FRAMES;
    int db = abs(b1 - b0) * 16;
    int32_t err = df / 2;
    int b = b0 * 16;
    int bstep = b1 < b0 ? -1 : +1;
    for (int f = 0; f < RAMP_FRAMES; f++) {
        int c = b / 16;
        c += (b % 16) >= dither_matrix[f % 16];
        set_LEDs(c);
        err -= db;
        if (err < 0) {
            b += bstep;
            err += df;
        }
    }
}

int main()
{
    init_timer();
    init_serial();
    init_SPI();
    init_ATX_power();
    sei();

    enable_ATX_power();
    delay_milliseconds(100);
    // init_stdio();

    next_update_time = millisecond_time() + FRAME_MS;
    SPI_write_byte(0x00);
    while (true) {
        const uint8_t b1 = MAX_BRIGHTNESS / 4;
        const uint8_t b2 = MAX_BRIGHTNESS * 3 / 4; 
        const uint8_t b3 = MAX_BRIGHTNESS;
        ramp(0, b1, set_LEDs_white);
        ramp(b1, 0, set_LEDs_white);
        ramp(0, b2, set_LEDs_CMY);
        ramp(b2, 0, set_LEDs_CMY);
        ramp(0, b3, set_LEDs_RGB);
        ramp(b3, 0, set_LEDs_RGB);
    }
}
