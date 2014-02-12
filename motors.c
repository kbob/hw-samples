#include "motors.h"

#include <avr/interrupt.h>
#include <util/atomic.h>

static isr_func *x_isr;
static isr_func *y_isr;

void init_x_timer(isr_func *isr, uint16_t initial_interval)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        x_isr = isr;

        // Waveform Generation Mode:  Fast PWM w/ TOP on ICRn.
        // Compare Output Mode x:     Fast PWM, clear on match, set on BOTTOM.
        // Compare Output Mode non-x: Disconnected.
        // Clock Select:              No clock source (stopped).
        // Timer/Counter:             0 (initial value).
        // Output Compare Register x: 1 microsecond.
        // Input Compare Register:    Initial interval
        // Overflow Interrupt:        Enabled.
        // Overflow Flag:             Cleared.
        TCCRxA = _BV(COMxx1_pulse) | _BV(WGMx1);
        TCCRxB = _BV(WGMx3) | _BV(WGMx2);
        TCNTx = 0;
        OCRxx_pulse = F_CPU / 1000000L;
        ICRx = initial_interval;
        TIMSKx = _BV(TOIEx);
        TIFRx = _BV(TOVx);
    }
}

void init_y_timer(isr_func *isr, uint16_t initial_interval)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        y_isr = isr;

        // Waveform Generation Mode:  Fast PWM w/ TOP on ICRn.
        // Compare Output Mode y:     Fast PWM, clear on match, set on BOTTOM.
        // Compare Output Mode non-x: Disconnected.
        // Clock Select:              No clock source (stopped).
        // Timer/Counter:             0 (initial value).
        // Output Compare Register y: 1 microsecond.
        // Input Compare Register:    Initial interval
        // Overflow Interrupt:        Enabled.
        // Overflow Flag:             Cleared.
        TCCRyA = _BV(COMyy1_pulse) | _BV(WGMy1);
        TCCRyB = _BV(WGMy3) | _BV(WGMy2);
        TCNTy = 0;
        OCRyy_pulse = F_CPU / 1000000L;
        ICRy = initial_interval;
        TIMSKy = _BV(TOIEy);
        TIFRy = _BV(TOVy);
    }
}

ISR(TIMERx_OVF_vect)
{
    if (x_isr)
        (*x_isr)();
}

ISR(TIMERy_OVF_vect)
{
    if (y_isr)
        (*y_isr)();
}
