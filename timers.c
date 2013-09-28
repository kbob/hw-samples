#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "serial.h"
#include "stdio_util.h"

#define TOGGLE_MS 100

//#define TC1_ONLY 1

#ifdef TC1_ONLY
uint16_t n1;
#else
uint16_t n1, n3, n4, n5;
#endif
volatile uint8_t running;

void toggle_LED(void);
void start_timers(void);
void await_timers(void);

int main(void)
{
    init_stdio();
    init_serial();

    PORTB &= ~_BV(PB7);         // set PB7, aka LED, off
    DDRB  |=  _BV(DDB7);        // make LED an output pin.

    PORTB &= ~_BV(PB5);         // set PB5, aka OC1A, low.
    DDRB  |=  _BV(DDB5);        // make OC1A an output pin.

#ifndef TC1_ONLY
    PORTE &= ~_BV(PE4);         // set PE4, aka OC3B, low.
    DDRE  |=  _BV(DDE4);        // make OC3B an output pin.

    PORTH &= ~_BV(PH3);         // set PH3, aka OC4A, low.
    DDRH  |=  _BV(DDH3);        // make OC4A an output pin.
    
    PORTL &= ~_BV(PL3);         // set PL3, aka OC5A, low.
    DDRL  |=  _BV(DDL3);        // make OC5A an output pin.
#endif


    // Init the four 16-bit timer/counters.
    //   WGMn3:0  = 0b1110 (fast PWM, TOP on ICR1, overflow on TOP)
    //   COMnx1:0 = 0 (PWM output disabled)
    //   CSn2:0   = 0 (no clock source)
    //   1 usec pulse width on one PWM pin (N.B., use B comparator on timer 3)
    //   Interrupt on overflow
    TCCR1A = 0x02;              // WGM1:0 = 0b10
    TCCR1B = 0x18;              // WGM3:2 = 0b11
    OCR1A  = 16;                // 16 / 16000000 Hz = 1 usec
    TIMSK1 = _BV(TOIE1);        // enable overflow interrupt
    TIFR1 |= _BV(TOV1);         // clear overflow interrupt

#ifndef TC1_ONLY
    TCCR3A = 0x02;              // WGM1:0 = 0b10
    TCCR3B = 0x18;              // WGM3:2 = 0b11
    OCR3B  = 16;                // 16 / 16000000 H = 1 usec
    TIMSK3 = _BV(TOIE3);        // enable overflow interrupt
    TIFR3 |= _BV(TOV3);         // clear overflow interrupt

    TCCR4A = 0x02;              // WGM1:0 = 0b10
    TCCR4B = 0x18;              // WGM3:2 = 0b11
    OCR4A  = 16;                // 16 / 16000000 Hz = 1 usec
    TIMSK4 = _BV(TOIE4);        // enable overflow interrupt
    TIFR4 |= _BV(TOV4);         // clear overflow interrupt

    TCCR5A = 0x02;              // WGM1:0 = 0b10
    TCCR5B = 0x18;              // WGM3:2 = 0b11
    OCR5A  = 16;                // 16 / 16000000 H = 1 usec
    TIMSK5 = _BV(TOIE5);        // enable overflow interrupt
    TIFR5 |= _BV(TOV5);         // clear overflow interrupt
#endif

    sei();

#if 1
    while (1) {
        start_timers();
        await_timers();
        toggle_LED();
    }
#else
    while (1) {
        _delay_ms(400);
        toggle_LED();
    }
#endif
}

void toggle_LED(void)
{
    if (PINB & _BV(PB7))
        PORTB &= ~_BV(PB7);
    else
        PORTB |= _BV(PB7);
}

#ifndef TC1_ONLY
void start_timers(void)
{
    // printf("%s:%u\n", __func__, __LINE__);
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        running = 1 << 1 | 1 << 3 | 1 << 4 | 1 << 5;
        n1 = n3 = n4 = n5 = 0;

        ICR1 = 16000;           /* 16000 / 16000000 Hz = 1 msec  */
        ICR3 = 16000;           /* 16000 / 16000000 Hz = 1 msec  */
        ICR4 = 16000;           /* 16000 / 16000000 Hz = 1 msec  */
        ICR5 = 16000;           /* 16000 / 16000000 Hz = 1 msec  */

        // In the asm instruction sequence below, the counters are
        // started exactly two CPU cycles apart.  So we preload the
        // counters with initial values exactly two counts apart, and
        // then the counters will all start off in sync.

        TCNT1 = 0;
        TCNT3 = 2;
        TCNT4 = 4;
        TCNT5 = 6;

        // Start timers.  Each sts takes two clocks, and the TCNTs'
        // initial values are staggered by two above, so the counters all
        // count in synchrony.

        uint8_t tccrb = 0x19;   // WGM3:2 = 0b11
                                // CS2:0 = 0b001 (clock source: IO clock / 1)
        __asm__ (
            "sts %1, %0\n\t"
            "sts %2, %0\n\t"
            "sts %3, %0\n\t"
            "sts %4, %0\n\t"
            :: "r"(tccrb),
               "i"((uint16_t)&TCCR1B),
               "i"((uint16_t)&TCCR3B),
               "i"((uint16_t)&TCCR4B),
               "i"((uint16_t)&TCCR5B));
    }
}
#else
void start_timers(void)
{
    // printf("%s:%u\n", __func__, __LINE__);
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        running = 1 << 1;
        n1 = 0;
        ICR1 = 16000;           /* 16000 / 16000000 Hz = 1 msec  */

        // In the asm instruction sequence below, the counters are
        // started exactly two CPU cycles apart.  So we preload the
        // counters with initial values exactly two counts apart, and
        // then the counters will all start off in sync.

        TCNT1 = 0;

        // Start timers.  Each sts takes two clocks, and the TCNTs'
        // initial values are staggered by two above, so the counters all
        // count in synchrony.

        uint8_t tccrb = 0x19;   // WGM3:2 = 0b11
                                // CS2:0 = 0b001 (clock source: IO clock / 1)
        __asm__ (
            "sts %1, %0\n\t"
            :: "r"(tccrb),
               "i"((uint16_t)&TCCR1B));
    }
}
#endif

void await_timers(void)
{
    printf("%s:%u\n", __func__, __LINE__);
    while (running) {
        continue;
    }
}

ISR(TIMER1_OVF_vect)
{
#if 0
    switch (n1++) {

    case 0:
        TCCR1A &= ~_BV(COM1A1);  // disable OC1A output
        ICR1 = 64512;
        break;

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        ICR1 = 64512;
        break;

    case 6:
        ICR1 = 12928;
        break;

    case 7:
        running &= ~(1 << 1);
        TCCR1B = 0x18;          /* clock select = no clock */
        break;

    default:
        while (1) continue;     /* Should not happen.  Lock up. */
    }
#else
    if (n1 == 0)
        TCCR1A &= ~_BV(COM1A1);  // disable OC1A output
    if ((n1 += 4) < TOGGLE_MS)
        ICR1 = 64000;
    else {
        running &= ~(1 << 1);
        TCCR1B = 0x18;          // clock select = no clock
    }
#endif
}

#ifndef TC1_ONLY

ISR(TIMER3_OVF_vect)
{
#if 0
    switch (n3++) {

    case 0:
        TCCR3A &= ~_BV(COM3A1);  // disable OC3A output
        ICR3 = 64512;
        break;

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        ICR3 = 64512;
        break;

    case 6:
        ICR3 = 12928;
        break;

    case 7:
        running &= ~(1 << 3);
        TCCR3B = 0x18;          /* clock select = no clock */
        break;

    default:
        while (1) continue;     /* Should not happen.  Lock up. */
    }
#else
    if (n3 == 0)
        TCCR3A &= ~_BV(COM3A1);  // disable OC3A output
    if ((n3 += 4) < TOGGLE_MS)
        ICR3 = 64000;
    else {
        running &= ~(1 << 3);
        TCCR3B = 0x18;          // clock select = no clock
    }
#endif
}

ISR(TIMER4_OVF_vect)
{
#if 0
    switch (n4++) {

    case 0:
        TCCR4A &= ~_BV(COM4A1);  // disable OC4A output
        ICR4 = 64512;
        break;

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        ICR4 = 64512;
        break;

    case 6:
        ICR4 = 12928;
        break;

    case 7:
        running &= ~(1 << 4);
        TCCR4B = 0x18;          /* clock select = no clock */
        break;

    default:
        while (1) continue;     /* Should not happen.  Lock up. */
    }
#else
    if (n4 == 0)
        TCCR4A &= ~_BV(COM4A1);  // disable OC4A output
    if ((n4 += 4) < TOGGLE_MS)
        ICR4 = 64000;
    else {
        running &= ~(1 << 4);
        TCCR4B = 0x18;          // clock select = no clock
    }
#endif
}

ISR(TIMER5_OVF_vect)
{
#if 0
    switch (n5++) {

    case 0:
        TCCR5A &= ~_BV(COM5A1);  // disable OC5A output
        ICR5 = 64512;
        break;

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        ICR5 = 64512;
        break;

    case 6:
        ICR5 = 12928;
        break;

    case 7:
        running &= ~(1 << 5);
        TCCR5B = 0x18;          /* clock select = no clock */
        break;

    default:
        while (1) continue;     /* Should not happen.  Lock up. */
    }
#else
    if (n5 == 0)
        TCCR5A &= ~_BV(COM5A1);  // disable OC5A output
    if ((n5 += 4) < TOGGLE_MS)
        ICR5 = 64000;
    else {
        running &= ~(1 << 5);
        TCCR5B = 0x18;          // clock select = no clock
    }
#endif
}

#endif
