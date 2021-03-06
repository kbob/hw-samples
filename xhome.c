#include "motors.h"

#include <stdio.h>

#include <avr/interrupt.h>

#include "ATX_power.h"
#include "limit-switches.h"
#include "motor-util.h"
#include "timer.h"

//#define MOVE_DISTANCE 530       // mm
#define MOVE_DISTANCE  50       // mm
#define MOVE_SPEED    150       // mm/second
#define HOME_SPEED_1  100
#define HOME_SPEED_2   10
#define HOME_SPEED_3    5

int main()
{
    init_ATX_power();
    init_limit_switches();
    init_timer();
    init_motors();
    sei();

    enable_ATX_power();
    delay_milliseconds(100);
    enable_x_motor();
    delay_milliseconds(200);

#define CONTINUOUS_HOMING 0
#if CONTINUOUS_HOMING
    while (1) {
        home_x();
        delay_milliseconds(4000);
        move_x(MM_to_uSTEPS(MOVE_DISTANCE));
        delay_milliseconds(200);
    }        
#else
    home_x();
    delay_milliseconds(500);
    disable_x_motor();
    disable_ATX_power();
    while (1)
        continue;
#endif
}
