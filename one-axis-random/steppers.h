#ifndef STEPPERS_INCLUDED
#define STEPPERS_INCLUDED

#include <stdbool.h>

#include "action.h"
#include "lock.h"
#include "pin_defs.h"
#include "register_ops.h"

#define X_dir_pos 0
#define X_dir_neg 1

#define Y_dir_pos 1             // Just guessing for now.
#define Y_dir_neg 0

#define Z_dir_pos 1             // Just guessing for now.
#define Z_dir_neg 0

extern void init_steppers          (void);
extern void start_steppers_if_idle (uint8_t   major_axis,
                                    axis_bits minor_axes,
                                    axis_bits positive_axes);
extern void stop_steppers          (void);

static inline void set_X_direction(bool positive)
{
    SET_1_BIT(PORTx_dir, PORTxn_dir, positive ? X_dir_pos : X_dir_neg);
}

static inline void start_X_step_timer(void)
{
    SBI(TCCRxB, CSx0);
}

static inline void stop_X_step_timer(void)
{
    CBI(TCCRxB, CSx0);
}

static inline void set_X_next_step_time_NO_LOCK(uint16_t ticks)
{
    ICRx = ticks;
}

static inline void set_X_next_step_time(uint16_t ticks)
{
    WITH_LOCK( {
        set_X_next_step_time_NO_LOCK(ticks);
    } );
}

#endif /* !STEPPERS_INCLUDED */
