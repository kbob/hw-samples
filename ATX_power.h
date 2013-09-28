#ifndef ATX_POWER_INCLUDED
#define ATX_POWER_INCLUDED

#include <stdbool.h>

#include <avr/io.h>

static inline void init_ATX_power(void)
{
    PORTB |= _BV(PORTB6);
    DDRB  |= _BV(DDB6);
    PORTB |= _BV(PORTB6);
}

static inline bool ATX_power_state(void)
{
    return bit_is_clear(PORTB, PORTB6);
}

static inline void enable_ATX_power(void)
{
    PORTB &= ~_BV(PORTB6);
}

static inline void disable_ATX_power(void)
{
    PORTB |= _BV(PORTB6);
}

#endif /* !ATX_POWER_INCLUDED */
