#include <avr/interrupt.h>

#include "i2c.h"
#include "laser-power.h"
#include "timer.h"

uint16_t power_levels[] = { 0, 819, 1638, 2457, 3276, 4095 };
#define NPL (sizeof power_levels / sizeof power_levels[0])

int main()
{
    init_timer();
    init_i2c();
    init_laser_power();
    sei();

    for (uint8_t i = 0; ; ++i == NPL && (i = 0)) {
        delay_milliseconds(2000);
        set_laser_power(power_levels[i]);
    }
}
