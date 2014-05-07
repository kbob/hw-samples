#ifndef SAFETY_SWITCHES_included
#define SAFETY_SWITCHES_included

// E-stop = D65 = PK3 = PCINT19
// lid    = D66 = PK4 = PCINT20

#include <stdbool.h>

#include <avr/io.h>

#define EMERGENCY_STOP_DDR_reg  DDRK
#define EMERGENCY_STOP_DD_bit   DDK3
#define EMERGENCY_STOP_PIN_reg  PINK
#define EMERGENCY_STOP_PIN_bit  PINK3
#define EMERGENCY_STOP_PORT_reg PORTK
#define EMERGENCY_STOP_PORT_bit PORTK3
#define EMERGENCY_STOP_STOPPED  1

#define LID_DDR_reg             DDRK
#define LID_DD_bit              DDK4
#define LID_PIN_reg             PINK
#define LID_PIN_bit             PINK4
#define LID_PORT_reg            PORTK
#define LID_PORT_bit            PORTK4
#define LID_OPEN                1
#define LID_CLOSED            (!LID_OPEN)

static inline void init_safety_switches(void)
{
    EMERGENCY_STOP_DDR_reg  &= ~_BV(EMERGENCY_STOP_DD_bit);
    EMERGENCY_STOP_PORT_reg |=  _BV(EMERGENCY_STOP_PORT_bit);

    LID_DDR_reg             &= ~_BV(LID_DD_bit);
    LID_PORT_reg            |=  _BV(LID_PORT_bit);
}

static inline bool e_is_stopped(void)
{
    return (EMERGENCY_STOP_STOPPED ?
            bit_is_set(EMERGENCY_STOP_PIN_reg, EMERGENCY_STOP_PIN_bit) :
            bit_is_clear(EMERGENCY_STOP_PIN_reg, EMERGENCY_STOP_PIN_bit));
}

static inline bool lid_is_open(void)
{
    return (LID_OPEN ?
            bit_is_set(LID_PIN_reg, LID_PIN_bit) :
            bit_is_clear(LID_PIN_reg, LID_PIN_bit));
}

#endif /* !SAFETY_SWITCHES_included */
