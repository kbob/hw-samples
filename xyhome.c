#define __DELAY_BACKWARD_COMPATIBLE__
#include "motors.h"

#include <stdio.h>

#include <avr/interrupt.h>

#include "ATX_power.h"
#include "SPI.h"
#include "limit-switches.h"
#include "motor-util.h"
#include "serial.h"
#include "serial_util.h"
#include "stdio_util.h"
#include "timer.h"

#define MOVE_X_DISTANCE 530     // mm
#define MOVE_Y_DISTANCE 280     // mm
//#define MOVE_X_DISTANCE 100     // mm
//#define MOVE_Y_DISTANCE  60     // mm
#define MOVE_SPEED      150     // mm/second
#define HOME_SPEED_1    100
#define HOME_SPEED_2     10
#define HOME_SPEED_3      5

#define LED_MS          600
#define PIXEL_COUNT      22
#define LEDS_OFF_MS     200
#define LEDS_BLINK_MS   150

static void set_LEDs(uint8_t level)
{
    SPI_write_byte(0x00);
    for (uint8_t i = 0; i < 3 * PIXEL_COUNT; i++)
        SPI_write_byte(level | 0x80);
    SPI_write_byte(0x00);
}

static void repeat_LEDs_off(uint16_t ms)
{
    uint32_t done = millisecond_time() + ms;
    uint32_t n = 0;
    while (millisecond_time() < done) {
        set_LEDs(0);
        n++;
    }
}

static void ramp_LEDs(uint8_t initial, uint8_t final, uint16_t ms)
{
    uint8_t min = initial;
    uint8_t max = final;
    if (max < min) {
        min = final;
        max = initial;
    }

    uint16_t t = ms / (max - min);
    for (uint8_t i = min; i <= max; i++) {
        set_LEDs(i);
        _delay_ms(t);
    }
}

int main()
{
    init_ATX_power();
    init_limit_switches();
    init_timer();
    init_motors();
    init_serial();
    init_stdio();
    init_SPI();
    sei();

    enable_ATX_power();
    repeat_LEDs_off(LEDS_OFF_MS);
    enable_x_motor();
    enable_y_motor();
    delay_milliseconds(200);
    ramp_LEDs(0, 127, LED_MS);

    while (1) {
        home_xy();
        delay_milliseconds(1000);
        move_xy(MM_to_uSTEPS(MOVE_X_DISTANCE), MM_to_uSTEPS(MOVE_Y_DISTANCE));
        delay_milliseconds(200);
    }        
}
