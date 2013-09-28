#include "timer.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

static uint32_t ticks;

void init_timer(void)
{
    // Timer/Counter 0 waveform generation mode = fast PWM, TOP = OCR0A.
    TCCR0A = _BV(WGM00) | _BV(WGM01) | _BV(WGM02);

    // Timer/Counter 0 clock select = prescale by 64.
    TCCR0B = _BV(CS00) | _BV(CS01);

    // Output Compare Register 0A = 250 (1 millisecond);
    OCR0A = F_CPU / 64 / 1000;

    // Enable Timer/Counter 0 overflow interrupt.
    TIMSK0 |= _BV(TOIE0);

    // Enable interrupts.
    sei();
}

ISR(TIMER0_OVF_vect)
{
    ticks++;
}

uint32_t millisecond_time(void)
{
    uint32_t t;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        t = ticks;
    }
    return t;
}

void delay_milliseconds(uint32_t ms)
{
    uint32_t end = millisecond_time() + ms;
    while (millisecond_time() != end)
        continue;
}

