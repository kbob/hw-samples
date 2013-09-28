#include "steppers.h"

#include "action.h"
#include "action_ring.h"
#include "convert.h"

typedef enum action_phase {
    ACCEL,
    CRUISE,
    DECEL
} action_phase;

typedef struct major_axis_state {
    action_phase phase;
    uint16_t     usteps_left;
    uint32_t     accel_time;
} major_axis_state;

typedef struct minor_axis_state {
    char TBD;
} minor_axis_state;

static struct major_axis_state major_state;
//static minor_axis_state X_state, Y_state, Z_state, P_state;
static action *current_action;
static uint8_t busy_axes;

void init_steppers(void)
{
    // Stop the X pulse timer.
    SET_3_BITS(TCCRxB, CSx2,CSx1,CSx0, 0b000);

    // Waveform Generation Mode = Fast PWM w/ TOP on ICRn.
    SET_2_BITS(TCCRxB, WGMx3,WGMx2, 0b11);
    SET_2_BITS(TCCRxA, WGMx1,WGMx0, 0b10);

    // Set the enable, step, and direction pins to output mode.
    SBI(DDRx_enable,  DDxn_enable);
    SBI(DDRx_dir,     DDxn_dir);
    SBI(DDRx_step,    DDxn_step);

    // Initialize enable, direction, and step pins.
    CBI(PORTx_step,   PORTxn_step);
    SBI(PORTx_dir,    PORTxn_dir);
    CBI(PORTx_enable, PORTxn_enable);

    // Set counter to zero.
    TCNTx = 0;

    // Set ICR (TOP) to 1 millisecond.
    ICRx = F_CPU / 1000;

    // Set the step pulse width to 1 microsecond.
    OCRxx_pulse = F_CPU / 1000000;

    // Comparator Output Mode.  Fast PWM, clear on match, set on BOTTOM.
    SET_2_BITS(TCCRxA, COMxx1_pulse,COMxx0_pulse, 0b10);

    // Clear counter overflow interrupt flag.
    SBI(TIFRx, TOVx);

    // Enable interrupt on counter overflow.
    SBI(TIMSKx, TOIEx);

    // N.B., the timers are not started yet.  They will be started
    // after the first action is enqueued.
}

void start_steppers_if_idle(uint8_t   major_axis,
                            axis_bits active_axes,
                            axis_bits positive_axes)
{
    WITH_LOCK( {
        if (!busy_axes) {
            if (major_axis == X_AXIS) {
                set_X_direction(positive_axes & X_AXIS);
                set_X_next_step_time(F_CPU / 1000); // XXX 1 msec. will work
                                                    // but is not optimal.
                start_X_step_timer();
                busy_axes |= X_AXIS;
            }
            // XXX need much more code here for the other cases and the
            // minor axes.
        }
    } );
}
                            
//void stop_steppers(void)
//{
//    stop_X_step_timer();
//}

ISR(TIMERx_OVF_vect)
{
    // Begin new action.
    // (same as simple_accel)
    //
    // Calculate timer.
    // (For major axis, same as simple_accel.  For minor axis,
    // not yet implemented.)
    //
    // If major axis phase complete, initialize for next phase.
    // (Same as simple_accel.)

    const action *ap = current_action;
    if (!ap) {
        ap = current_action = dequeue_action_NO_LOCK();
        if (!ap) {
            stop_X_step_timer();
            busy_axes &= ~X_AXIS;
            return;
        }

        // Begin new action.

        if (ap->type & X_AXIS) {
            // This is the major axis.
            set_X_direction(ap->positive_axes & X_AXIS);
            if (ap->major.accel_usteps) {
                // begin accel phase
                major_state.phase = ACCEL;
                major_state.usteps_left = ap->major.accel_usteps;
                major_state.accel_time = 0;
            } else if (ap->major.cruise_usteps) {
                // begin cruise phase
                major_state.phase = CRUISE;
                major_state.usteps_left = ap->major.cruise_usteps;
            } else if (ap->major.decel_usteps) {
                // begin decel phase
                major_state.phase = DECEL;
                major_state.usteps_left = ap->major.decel_usteps;
                major_state.accel_time = 0;
            }
            // start other active axes' step timers.
        } else if (ap->active_axes & X_AXIS) {
            // ... X is an active, minor axis...
        } else {
            // ... X is inactive...
        }
    }

    // Calculate next step time.

    if (ap->type & X_AXIS) {
        uint16_t interval = 0;       // XXX rename interval to ustep_ticks?
        if (major_state.phase == ACCEL) {
            // XXX This could be Bresenham, especially with the shift.
            uint16_t rate = ap->major.V0 +
                            (major_state.accel_time * ap->major.A >> 16);
            interval = convert_rate_to_interval(rate);
        } else if (major_state.phase == CRUISE) {
            interval = ap->major.cruise_ticks;
        } else if (major_state.phase == DECEL) {
            uint16_t rate = ap->major.Vmax -
                            (major_state.accel_time * ap->major.A >> 16);
            interval = convert_rate_to_interval(rate);
       }
        // XXX do something if interval == 0
//      if (interval == 0)
//          interval = 65535;
        set_X_next_step_time_NO_LOCK(interval);
        major_state.accel_time += interval;

        if (!--major_state.usteps_left) {

            // Calculate next action phase.

            if (major_state.phase == ACCEL && ap->major.cruise_usteps) {
                // begin cruise phase
                major_state.phase = CRUISE;
                major_state.usteps_left = ap->major.cruise_usteps;
            } else if (major_state.phase != DECEL && ap->major.decel_usteps) {
                // begin decel phase
                major_state.phase = DECEL;
                major_state.usteps_left = ap->major.decel_usteps;
                major_state.accel_time = 0;
            } else {
                // Finished action.  Start new one next time.
                discard_action_NO_LOCK(current_action);
                current_action = NULL;
            }
        }
    } else {
        // ... X is a minor axis...
    }
}

