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
#define MAX_HUNT      (MM_to_uSTEPS(10))

#define MM_to_uSTEPS(mm)           ((int32_t)((mm) * 2000.0 / 25.4))
#define uSEC_per_uSTEP(mm_per_sec) ((uint16_t)(25.4 / 2000 / (mm_per_sec) * 1e6))

#define RETRIES 10

uint16_t freq[RETRIES + 1];

void move_y(uint32_t usteps)
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
    // printf("count=%d\n", count);
    return count > RETRIES / 2;
}

static void home_y(void)
{
    bool ok = true;
    printf("Homing\n");
    printf("MAX_HUNT = %lu\n", (unsigned long)MAX_HUNT);
    for (uint8_t i = 0; i < RETRIES + 1; i++)
        freq[i] = 0;

    printf("Homing 1\n");
    uint16_t usteps1 = 0, usteps2 = 0, usteps3 = 0;
    set_y_direction_negative();
    while (!c_y_min_reached()) {
        set_y_direction_negative();
        _delay_us(1);
        step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_1));
        usteps1++;
    }

    printf("Homing 2\n");
    set_y_direction_positive();
    // for (uint16_t i = 0; i < MAX_HUNT && c_y_min_reached(); i++)
    for (uint16_t i = 0; c_y_min_reached(); i++) {
        if (i == MAX_HUNT) {
            ok = false;
            break;
        }
        step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_2));
        usteps2++;
    }

    printf("Homing 3\n");
    set_y_direction_negative();
    // for (uint16_t i = 0; i < MAX_HUNT && !c_y_min_reached(); i++)
    for (uint16_t i = 0; !c_y_min_reached(); i++) {
        if (i == MAX_HUNT) {
            ok = false;
            break;
        }
        step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_3));
        usteps3++;
    }
    set_y_direction_positive();

    if (ok)
        printf("Homed.\n");
    else
        printf("Home failed.\n");
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
    init_limit_switches();

#define CONTINUOUS_HOMING 0
#if CONTINUOUS_HOMING
    while (1) {
        home_y();
        delay_milliseconds(1000);
        move_y(MM_to_uSTEPS(MOVE_DISTANCE));
        delay_milliseconds(200);
    }        
#else
    home_y();
    while (true)
        continue;
#endif
}
