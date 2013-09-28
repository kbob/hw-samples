#include "usec-timer.h"

#include <stdio.h>

#include <avr/io.h>
#include <util/atomic.h>

static uint16_t overflow_counter;

void init_usec_timer(void)
{
    TCCR1A = 0;                 // no Compare Output, WGM = Normal
    TCCR1B = 0;                 // stop counter.
    TCNT1  = 0;                 // start at zero.
    TIFR1 |= _BV(TOV1);         // clear pending overflow interrupt.
    TIMSK1 = _BV(TOIE1);        // enable overflow interrupt.
    TCCR1B = _BV(CS11);         // clock select: I/O clock / 8.
}

uint32_t usec_time(void)
{
    union {
        uint32_t l;
        uint16_t s[2];
    } u;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        u.s[0] = TCNT1;
        u.s[1] = overflow_counter;
        if (bit_is_set(TIFR1, TOV1))
            u.s[1]++;
    }
#if   F_CPU == 16000000L
    return u.l >> 1;
#elif F_CPU == 20000000L
    return (u.l / 5) << 1;
#else
    #error "unexpected F_CPU value"
    return -1;
#endif
}


ISR(TIMER1_OVF_vect)
{
    overflow_counter++;
}
