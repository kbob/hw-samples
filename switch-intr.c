#include "safety-switches.h"

#include <stdio.h>

#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "serial.h"
#include "stdio_util.h"

// print interrupt count, switches up/down.

typedef union state {
    struct {
        int  intr_count;
        bool is_stopped;
        bool is_open;
    };
    uint32_t w;
} state;

static state curr_state;

static void init_intr(void)
{
    PCICR                    |= _BV(EMERGENCY_STOP_PCIE_bit);
    PCICR                    |= _BV(LID_PCIE_bit);
    EMERGENCY_STOP_PCMSK_reg |= _BV(EMERGENCY_STOP_PCINT_bit);
    LID_OPEN_PCMSK_reg       |= _BV(LID_PCINT_bit);
    // PCMSK2 |= _BV(PCINT19);
    // PCMSK2 |= _BV(PCINT20);
}

static state get_state(void)
{
    state s;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        s = curr_state;
    }
    return s;
}

int main()
{
    init_serial();
    init_stdio();
    init_safety_switches();
    init_intr();
    sei();

    // poor man's assertions
    while (PCINT18 != 2)
        printf("PCINT18 = %d = %#x\n", PCINT18, PCINT18);
    while (PCINT19 != 3)
        printf("PCINT19 = %d = %#x\n", PCINT19, PCINT19);
    while (PCINT20 != 4)
        printf("PCINT20 = %d = %#x\n", PCINT20, PCINT20);

    state prev = { .is_open = 2 };
    while (true) {
        state curr = get_state();
        if (curr.w != prev.w) {
            const char *e = curr.is_stopped ? "stop" : "    ";
            const char *l = curr.is_open  ? "open" : "    ";
            int d = curr.intr_count - prev.intr_count;
            printf("%4s  %4s  %5d intr\n", e, l, d);
            prev = curr;
        }
    }
}

ISR(PCINT2_vect)
{
    curr_state.intr_count++;
    curr_state.is_stopped = e_is_stopped();
    curr_state.is_open = lid_is_open();
}
