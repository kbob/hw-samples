#ifndef HEATERS_INCLUDED
#define HEATERS_INCLUDED

// 0: Heated Bed - D8,  aka PH5
// 1: Hot End 1  - D10, aka PB4
// 2: Hot End 2  - D9,  aka PH6


#include <stdbool.h>

#include <avr/io.h>

static inline void init_heaters(void)
{
    PORTH &= ~_BV(PORTH5);
    DDRH  &= ~_BV(DDH5);
    PORTH &= ~_BV(PORTH5);

    PORTB &= ~_BV(PORTB4);
    DDRB  &= ~_BV(DDB4);
    PORTB &= ~_BV(PORTB4);

    PORTH &= ~_BV(PORTH6);
    DDRH  &= ~_BV(DDH6);
    PORTH &= ~_BV(PORTH6);
}

static inline bool heater_0_state(void)
{
    return bit_is_set(PORTH, PORTH5);
}

static inline bool heater_1_state(void)
{
    return bit_is_set(PORTB, PORTB4);
}

static inline bool heater_2_state(void)
{
    return bit_is_set(PORTH, PORTH6);
}

static inline void enable_heater_0(void)
{
    PORTH |= _BV(PORTH5);
}

static inline void disable_heater_0(void)
{
    PORTH &= ~_BV(PORTH5);
}

static inline void enable_heater_1(void)
{
    PORTB |= _BV(PORTB4);
}

static inline void disable_heater_1(void)
{
    PORTB &= ~_BV(PORTB4);
}

static inline void enable_heater_2(void)
{
    PORTH |= _BV(PORTH6);
}

static inline void disable_heater_2(void)
{
    PORTH &= ~_BV(PORTH6);
}

#endif /* !HEATERS_INCLUDED */
