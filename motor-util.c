#include "motor-util.h"

#include <stdbool.h>
#include <stdio.h>

#include "limit-switches.h"
#include "motors.h"

#define MOVE_SPEED     40       // mm/second
#define MOVE_SPEED2   200       // mm/second
#define MOVE_SPEED3   250       // mm/second
#define MOVE_SPEED4   300       // mm/second
#define HOME_SPEED_1  100
#define HOME_SPEED_2   10
#define HOME_SPEED_3    5

// c_?_min_reached: conditioned limit switch status

static bool c_x_min_reached(void)
{
    return x_min_reached();
}

static bool c_y_min_reached(void)
{
    return y_min_reached();
}

void move_x(int32_t usteps)
{
    if (usteps > 0) {
        printf("%s: positive\n", __func__);
        set_x_direction_positive();
    } else {
        printf("%s: negative\n", __func__);
        set_x_direction_negative();
        usteps = -usteps;
    }
    printf("%s: usteps = %ld\n", __func__, usteps);
    for (uint32_t i = 0; i < usteps; i++) {
        step_x();
        _delay_us(uSEC_per_uSTEP(MOVE_SPEED));
    }
    printf("%s: done\n", __func__);
}

void move_y(int32_t usteps)
{
    if (usteps > 0)
        set_y_direction_positive();
    else {
        set_y_direction_negative();
        usteps = -usteps;
    }
    for (uint32_t i = 0; i < usteps; i++) {
        step_y();
        _delay_us(uSEC_per_uSTEP(MOVE_SPEED));
    }
}

void move_xy(int32_t x_usteps, int32_t y_usteps)
{
    if (x_usteps > 0)
        set_x_direction_positive();
    else if (x_usteps < 0) {
        set_x_direction_negative();
        x_usteps = -x_usteps;
    }
    if (y_usteps > 0)
        set_y_direction_positive();
    else if (y_usteps < 0) {
        set_y_direction_negative();
        y_usteps = -y_usteps;
    }
    if (x_usteps >= y_usteps) {
        int32_t err = x_usteps / 2;
        for (uint32_t i = 0; i < x_usteps; i++) {
            step_x();
            err -= y_usteps;
            if (err < 0) {
                err += x_usteps;
                step_y();
            }
            if (i < 200 || i > x_usteps - 200)
                _delay_us(uSEC_per_uSTEP(MOVE_SPEED));
            else if (i < 400 || i > x_usteps - 400)
                _delay_us(uSEC_per_uSTEP(MOVE_SPEED2));
            else if (i < 600 || i > x_usteps - 600)
                _delay_us(uSEC_per_uSTEP(MOVE_SPEED3));
            else
                _delay_us(uSEC_per_uSTEP(MOVE_SPEED4));
        }
    } else {
        int32_t err = y_usteps / 2;
        for (uint32_t i = 0; i < y_usteps; i++) {
            step_y();
            err -= x_usteps;
            if (err < 0) {
                err += y_usteps;
                step_x();
            }
            if (i < 200 || i > y_usteps - 200)
                _delay_us(uSEC_per_uSTEP(MOVE_SPEED));
            else if (i < 400 || i > y_usteps - 400)
                _delay_us(uSEC_per_uSTEP(MOVE_SPEED2));
            else if (i < 600 || i > y_usteps - 600)
                _delay_us(uSEC_per_uSTEP(MOVE_SPEED3));
            else
                _delay_us(uSEC_per_uSTEP(MOVE_SPEED4));
        }
    }
}

void home_x(void)
{
    uint16_t usteps1 = 0, usteps2 = 0, usteps3 = 0;

    // 1st: move left until limit hit.
    set_x_direction_negative();
    enable_x_motor();
    while (!c_x_min_reached()) {
        step_x();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_1));
        usteps1++;
    }

    // 2nd: move right until off the limit.
    // Don't go further than 1 cm.
    uint16_t max_usteps2 = MM_to_uSTEPS(10);
    set_x_direction_positive();
    while (usteps2 < max_usteps2 && c_x_min_reached()) {
        step_x();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_2));
        usteps2++;
    }
        
    // 3rd: move left again until limit hit.
    set_x_direction_negative();
    while (!c_x_min_reached()) {
        step_x();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_3));
        usteps3++;
    }
    set_x_direction_positive();
}

void home_y(void)
{
    uint16_t usteps1 = 0, usteps2 = 0, usteps3 = 0;

    // 1st: move down until limit hit.
    set_y_direction_negative();
    while (!c_y_min_reached()) {
        step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_1));
        usteps1++;
    }

    // 2nd: move up until off the limit.
    // Don't go further than 1 cm.
    set_y_direction_positive();
    while (c_y_min_reached()) {
        step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_2));
        usteps2++;
    }

    // 3rd: move down again until limit hit.
    set_y_direction_negative();
    while (!c_y_min_reached()) {
        step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_3));
        usteps3++;
    }
    set_y_direction_positive();
}

void home_xy(void)
{
    bool at_x_min, at_y_min;

    // 1st: move down and left until both X and Y min limit hit.
    printf("Step 1\n");
    set_x_direction_negative();
    set_y_direction_negative();
    at_x_min = at_y_min = false;
    // uint8_t z = 0;
    while (true) {
        at_x_min = at_x_min || c_x_min_reached();
        at_y_min = at_y_min || c_y_min_reached();
        // if (!++z)
        //     printf("at_x_min=%d at_y_min=%d\n", at_x_min, at_y_min);
        if (at_y_min && at_x_min)
            break;
        if (!at_x_min)
            step_x();
        if (!at_y_min)
            step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_1));
    }
    printf("Step 1 end\n");

    // 2nd: move up and right until off both X and Y min limits.
    // Travel 1 cm max.
    printf("Step 2\n");
    set_x_direction_positive();
    set_y_direction_positive();
    uint16_t ustep_count = 0;
    while (true) {
        at_x_min = at_x_min && c_x_min_reached();
        at_y_min = at_y_min && c_y_min_reached();
        if (!at_y_min && !at_x_min)
            break;
        if (at_x_min)
            step_x();
        if (at_y_min)
            step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_2));
        if (ustep_count == MM_to_uSTEPS(10))
            break;
    }
    printf("Step 2 end\n");

    // 3rd: move down and left until both X and Y min limits hit again.
    printf("Step 3\n");
    set_x_direction_negative();
    set_y_direction_negative();
    while (true) {
        at_x_min = at_x_min || c_x_min_reached();
        at_y_min = at_y_min || c_y_min_reached();
        if (at_y_min && at_x_min)
            break;
        if (!at_x_min)
            step_x();
        if (!at_y_min)
            step_y();
        _delay_us(uSEC_per_uSTEP(HOME_SPEED_3));
    }
    printf("Step 3 end\n");
}
