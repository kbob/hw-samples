#include "motors.h"

#include <stdio.h>

#include <avr/interrupt.h>

#include "ATX_power.h"
#include "limit-switches.h"
#include "serial.h"
#include "serial_util.h"
#include "stdio_util.h"
#include "timer.h"

#define MOVE_DISTANCE 280       // mm
#define MOVE_SPEED    150       // mm/second
#define HOME_SPEED_1  100
#define HOME_SPEED_2   10
#define HOME_SPEED_3    5

#define MM_to_uSTEPS(mm)           ((int32_t)((mm) * 2000.0 / 25.4))
#define uSEC_per_uSTEP(mm_per_sec) ((uint16_t)(25.4 / 2000 / (mm_per_sec) * 1e6))

#define RETRIES 10

uint16_t freq[RETRIES + 1];

static void move_y(uint32_t usteps)
{
    set_y_direction_positive();
    for (uint32_t i = 0; i < usteps; i++) {
        step_y();
        _delay_us(uSEC_per_uSTEP(MOVE_SPEED));
    }
}

static bool c_y_min_reached(void)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < RETRIES; i++)
        if (y_min_reached())
            count++;
    freq[count]++;
    //printf("count=%d\n", count);
    return count > RETRIES / 2;
}

static void home_y(void)
{
    for (uint8_t i = 0; i < RETRIES + 1; i++)
        freq[i] = 0;

    uint16_t usteps1 = 0, usteps2 = 0, usteps3 = 0;
    set_y_direction_negative();
    while (!c_y_min_reached()) {
        step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_1));
        usteps1++;
    }

    set_y_direction_positive();
    while (c_y_min_reached()) {
        step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_2));
        usteps2++;
    }
    set_y_direction_negative();
    while (!c_y_min_reached()) {
        step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_3));
        usteps3++;
    }
    set_y_direction_positive();

    printf("%d %d %d.  ", usteps1, usteps2, usteps3);

    for (uint8_t i = 0; i < RETRIES + 1; i++) {
        if (freq[i]) {
            printf("%u: %u  ", i, freq[i]);
        }
    }
    printf("\n");
}

int main()
{
    init_ATX_power();
    init_limit_switches();
    init_timer();
    init_motors();
    init_serial();
    init_stdio();
    sei();

    enable_ATX_power();
    delay_milliseconds(100);
    enable_y_motor();
    delay_milliseconds(200);

    while (1) {
        home_y();
        delay_milliseconds(1000);
        move_y(MM_to_uSTEPS(MOVE_DISTANCE));
        delay_milliseconds(200);
    }        
}
