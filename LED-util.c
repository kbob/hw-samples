#include "LED-util.h"

#define __ASSERT_USE_STDERR 1
#include <assert.h>
#include <math.h>
#include <stdio.h>

#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#include "SPI.h"
#include "timer.h"

#define PIXEL_COUNT      22
#define LEDS_OFF_MS     500

#define M_2PI (2.0 * M_PI)

static uint32_t next_update_time;
static uint8_t illum_level;

static void delay(uint8_t frame_ms)
{
    while ((int16_t)millisecond_time() - (int16_t)next_update_time < 0)
        continue;
    next_update_time += frame_ms;
}

void begin_LEDs_refresh()
{
    SPI_write_byte(0x00);
}

void set_pixel_color(uint8_t red, uint8_t green, uint8_t blue)
{
    SPI_write_byte(green | 0x80);
    SPI_write_byte(red | 0x80);
    SPI_write_byte(blue | 0x80);
}

void set_pixel_level(uint8_t level)
{
    set_pixel_color(level, level, level);
}

void end_LEDs_refresh()
{
    SPI_write_byte(0x00);
}

void set_LEDs_color(uint8_t red, uint8_t green, uint8_t blue)
{
    begin_LEDs_refresh();
    for (uint8_t i = 0; i < PIXEL_COUNT; i++)
        set_pixel_color(red, green, blue);
    end_LEDs_refresh();
}

void set_LEDs_level(uint8_t level)
{
    set_LEDs_color(level, level, level);
}

void repeat_LEDs_off(void)
{
    uint32_t done = millisecond_time() + LEDS_OFF_MS;
    uint32_t n = 0;
    while (millisecond_time() < done) {
        set_LEDs_level(0);
        n++;
    }
}

void ramp_LEDs(uint8_t initial, uint8_t final, uint16_t ms)
{
    uint8_t min = initial;
    uint8_t max = final;
    int8_t  inc = +1;
    if (max < min) {
        min = final;
        max = initial;
        inc = -1;
    }

    uint16_t t = ms / (max - min + 1);
    for (uint8_t i = initial; i != final; i += inc) {
        set_LEDs_level(i);
        _delay_ms(t);
    }
}

#define GG_RAMP_MS    3000
#define GG_FRAME_MS     20
#define GG_BREATHE_MS 4000
#define GG_MAX_LEVEL   127
#define GG_MIN_LEVEL    10
#define GG_RB_LEVEL      1

void soothing_green_glow(void)
{
    bool ramping = true;
    uint16_t r = 0;
    uint8_t f = 0;
    for (uint16_t n = 0; ; n = (n + GG_FRAME_MS) % GG_BREATHE_MS) {
        double theta = (M_2PI / GG_BREATHE_MS) * n;
        assert(0.0 <= theta && theta <= M_2PI);
        begin_LEDs_refresh();
        for (uint8_t i = 0; i < PIXEL_COUNT; i++) {
            double phi = fabs(i * (M_2PI / (PIXEL_COUNT - 1)) - M_PI);
            double  a = sin(theta - phi);
            assert(-1.0 <= a && a <= +1.0);
            double  b = (a + 1.0) / 2.0 * cos(phi / 2.0001);
            if (b < 0.0 || b > +1.0)
                printf("i = %u, b = (%g + 1.0) / 2.0 * cos(%g / 2.0) = %g\n",
                       i, a, phi, b);
            assert(0.0 <= b && b <= +1.0);
            double  c = b * b;
            uint8_t g = GG_MIN_LEVEL + (GG_MAX_LEVEL - GG_MIN_LEVEL) * c;
            assert(GG_MIN_LEVEL <= g && g <= GG_MAX_LEVEL);
            uint8_t rb = GG_RB_LEVEL;
            assert(rb <= g);
            if (ramping) {
                uint16_t m = ((uint32_t)r << 8) / GG_RAMP_MS;
                uint16_t mm = 256 - m;
                assert(0 <= m && m <= 256);
                g  =  (g * m + illum_level * mm) >> 8;
                rb = (rb * m + illum_level * mm) >> 8;
            }
            set_pixel_color(rb, g, rb);
        }            
        end_LEDs_refresh();
        if (ramping) {
            r += GG_FRAME_MS;
            if (r >= GG_RAMP_MS)
                ramping = false;
        }
        f++;
        delay(GG_FRAME_MS);
    }
    while (true)
        continue;
}
