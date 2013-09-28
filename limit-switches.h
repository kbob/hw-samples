#ifndef LIMIT_SWITCHES_included
#define LIMIT_SWITCHES_included

#include <stdbool.h>
#include <avr/io.h>

// X min =  D3 = PE5
// X max =  D2 = PE4 (not used)
// Y min = D14 = PJ1
// Y max = D15 = PJ0 (not used)
// Z min = D18 = PD3
// Z max = D19 = PD2

#define DDRx_min   DDRE
#define DDxn_min   DDE5
#define PORTx_min  PORTE
#define PORTxn_min PE5
#define PINx_min   PINE
#define PINxn_min  PINE5

#define DDRy_min   DDRJ
#define DDyn_min   DDJ1
#define PORTy_min  PORTJ
#define PORTyn_min PJ1
#define PINy_min   PINJ
#define PINyn_min  PINJ1

#define DDRz_min   DDRD
#define DDzn_min   DDD3
#define PORTz_min  PORTD
#define PORTzn_min PD3
#define PINz_min   PIND
#define PINzn_min  PIND3

#define DDRz_max   DDRD
#define DDzn_max   DDD2
#define PORTz_max  PORTD
#define PORTzn_max PD2
#define PINz_max   PIND
#define PINzn_max  PIND2


static inline void init_limit_switches(void)
{
    DDRx_min  &= ~_BV(DDxn_min);
    PORTx_min |=  _BV(PORTxn_min);
    DDRy_min  &= ~_BV(DDyn_min);
    PORTy_min |=  _BV(PORTyn_min);
    DDRz_min  &= ~_BV(DDzn_min);
    PORTz_min |=  _BV(PORTzn_min);
    DDRz_max  &= ~_BV(DDzn_max);
    PORTz_max |=  _BV(PORTzn_max);
}

static inline bool x_min_reached(void)
{
    return bit_is_set(PINx_min, PINxn_min);
}

static inline bool y_min_reached(void)
{
    return bit_is_set(PINy_min, PINyn_min);
}

static inline bool z_min_reached(void)
{
    return bit_is_set(PINz_min, PINzn_min);
}

static inline bool z_max_reached(void)
{
    return bit_is_set(PINz_max, PINzn_max);
}

#endif /* !LIMIT_SWITCHES_included */
