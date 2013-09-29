/* blink  - blink the LED on pin D13 (aka PB7). */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "timer.h"


// // //  LED  // // //

const int LED_PIN = 13;

static inline void init_LED_pin(void)
{
    DDRB |= _BV(DDB7);
}

static inline void set_LED(void)
{
    PORTB |= _BV(DDB7);
}

static inline inline void clear_LED(void)
{
    PORTB &= ~_BV(DDB7);
}


// // //  Main Program  // // //

int main()
{
    init_timer();
    init_LED_pin();
    sei();

    while (1) {
        set_LED();
        delay_milliseconds(300);
        clear_LED();
        delay_milliseconds(600);
    }
}
