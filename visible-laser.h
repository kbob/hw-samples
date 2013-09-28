#ifndef VISIBLE_LASER_included
#define VISIBLE_LASER_included

#include <avr/io.h>

static inline void init_visible_laser(void)
{
    PORTH &= ~_BV(PH4);
    DDRH  |= _BV(DDH4);
    PORTH &= ~_BV(PH4);
}

static inline void enable_visible_laser(void)
{
    PORTH |= _BV(PH4);
}

static inline void disable_visible_laser(void)
{
    PORTH &= ~_BV(PH4);
}

#endif /* !VISIBLE_LASER_included */
