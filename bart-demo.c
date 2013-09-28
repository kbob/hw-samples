#include <avr/interrupt.h>

#include "ATX_power.h"
#include "LED-util.h"
#include "limit-switches.h"
#include "motor-util.h"
#include "motors.h"
#include "serial.h"
#include "stdio_util.h"
#include "SPI.h"
#include "timer.h"
#include "visible-laser.h"

#define X_ORIGIN      110       // mm
#define Y_ORIGIN      155       // mm
#define WIDTH         135       // mm
#define HEIGHT         70       // mm
#define LED_RAMP_TIME 600       // ms
#define LED_LEVEL      20       // 0..127

int main()
{
    init_stdio();
    init_serial();
    init_ATX_power();
    init_limit_switches();
    init_timer();
    init_motors();
    init_SPI();
    init_visible_laser();
    sei();

    enable_ATX_power();
    repeat_LEDs_off();
    enable_x_motor();
    enable_y_motor();
    delay_milliseconds(200);
    ramp_LEDs(0, 127, LED_RAMP_TIME);
    ramp_LEDs(127, LED_LEVEL, LED_RAMP_TIME);

    home_xy();
    delay_milliseconds(1000);
    move_xy(MM_to_uSTEPS(X_ORIGIN), MM_to_uSTEPS(Y_ORIGIN));

    delay_milliseconds(150);
    enable_visible_laser();

    move_y(MM_to_uSTEPS(HEIGHT));  // up left side
    move_x(MM_to_uSTEPS(WIDTH));   // right across top
    move_y(-MM_to_uSTEPS(HEIGHT)); // down right side
    move_x(-MM_to_uSTEPS(WIDTH));  // left across bottom

    disable_visible_laser();
    delay_milliseconds(150);

    home_xy();
    delay_milliseconds(50);     // let carriage settle.
    disable_x_motor();
    disable_y_motor();
    soothing_green_glow();

    while (1)
        continue;
}
