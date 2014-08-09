#ifndef MAIN_LASER_included
#define MAIN_LASER_included

#include <avr/io.h>

#define MAIN_LASER_PULSE
#define MAIN_LASER_PULSE_DDR_reg            DDRH
#define MAIN_LASER_PULSE_DD_bit             DDH3
#define MAIN_LASER_PULSE_PIN_reg            PINH
#define MAIN_LASER_PULSE_PIN_bit            PINH3
#define MAIN_LASER_PULSE_PORT_reg           PORTH
#define MAIN_LASER_PULSE_PORT_bit           PORTH3

#define MAIN_LASER_PULSE_ON                 HIGH
#define MAIN_LASER_PULSE_OFF              (!MAIN_LASER_PULSE_ON)

static inline void init_main_laser(void)
{
    MAIN_LASER_PULSE_PORT_reg &= ~_BV(MAIN_LASER_PULSE_PORT_bit);
    MAIN_LASER_PULSE_DDR_reg |= _BV(MAIN_LASER_PULSE_DD_bit);
    MAIN_LASER_PULSE_PORT_reg &= ~_BV(MAIN_LASER_PULSE_PORT_bit);
}

static inline void enable_main_laser(void)
{
    MAIN_LASER_PULSE_PORT_reg |= _BV(MAIN_LASER_PULSE_PORT_bit);
}

static inline void disable_main_laser(void)
{
    MAIN_LASER_PULSE_PORT_reg &= ~_BV(MAIN_LASER_PULSE_PORT_bit);
}

#endif /* !MAIN_LASER_included */
