#include "motors.h"

#include <assert.h>
#include <stdio.h>

#include <avr/interrupt.h>
#include <util/atomic.h>

#include "ATX_power.h"
#include "LED-util.h"
#include "limit-switches.h"
#include "motor-util.h"
#define DEFINE_CONV_TABLE
#include "one-axis-random/convert.h"
#include "serial.h"
#include "SPI.h"
#include "stdio_util.h"
#include "timer.h"

#define STROKE_LENGTH ((int)(300 / 25.4 * 2000))
#define SPEED 100               // mm/sec

#define DELAY0 1000
#define DELAY1  200

typedef enum atom {
    A_STOP = 0,
    A_DIR_POSITIVE,
    A_DIR_NEGATIVE,
    A_LOOP_UNTIL_MIN,
    A_LOOP_WHILE_MIN,
    // A_LOOP_UNTIL_MAX,
    // A_LOOP_WHILE_MAX,
    A_ENABLE_STEP,
    A_DISABLE_STEP,
    ATOM_MAX
} atom;

typedef enum queue_mask {
    qm_x = 1 << 0,
    // qm_y = 1 << 1,
    // qm_z = 1 << 2,
    qm_all = qm_x /* | qm_y | qm_z */
} queue_mask;

typedef struct queue {
    volatile uint8_t q_head;
    volatile uint8_t q_tail;
} queue;

uint16_t         x_buf[256];
queue            Xq;
volatile uint8_t running_queues;

void abort()
{
    stop_SPI();
    disable_ATX_power();
    printf("\nabort\n\n");
    while (true)
        continue;
}

static inline bool queue_is_empty_NONATOMIC(const queue *q)
{
    return q->q_head == q->q_tail;
}

static inline bool queue_is_full_NONATOMIC(const queue *q)
{
    return q->q_head == (uint8_t)(q->q_tail + 2);
}

static inline bool queue_is_empty(const queue *q)
{
    bool empty;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        empty = queue_is_empty_NONATOMIC(q);
    }
    return empty;
}

static inline bool queue_is_full(const queue *q)
{
    bool full;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        full = queue_is_full_NONATOMIC(q);
    }
    return full;
}

static inline bool any_queue_is_full(void)
{
    // Just X for now.

    bool any;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        any = (queue_is_full_NONATOMIC(&Xq));
    }
    return any;
}

static inline uint16_t dequeue_atom_X_NONATOMIC(void)
{
    if (queue_is_empty_NONATOMIC(&Xq))
        return A_STOP;
    return x_buf[Xq.q_head++];
}

static inline void undequeue_atom_X_NONATOMIC(uint16_t a)
{
    x_buf[--Xq.q_head] = a;
}

static inline void enqueue_atom_X(uint16_t a)
{
    while (queue_is_full(&Xq))
        continue;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        x_buf[Xq.q_tail++] = a;
    }
}

static void x_intr(void)
{
    while (true) {
        uint16_t a = dequeue_atom_X_NONATOMIC();
        if (a < ATOM_MAX) {
            switch (a) {

            case A_STOP:
                stop_x_timer();
                running_queues &= ~qm_x;
                return;

            case A_DIR_POSITIVE:
                set_x_direction_positive();
                break;

            case A_DIR_NEGATIVE:
                set_x_direction_negative();
                break;

            case A_LOOP_UNTIL_MIN:
                if (!x_min_reached()) {
                    undequeue_atom_X_NONATOMIC(a);
                    return;
                }
                break;

            case A_LOOP_WHILE_MIN:
                if (x_min_reached()) {
                    undequeue_atom_X_NONATOMIC(a);
                    return;
                }
                break;

            case A_ENABLE_STEP:
                enable_x_step();
                break;

            case A_DISABLE_STEP:
                disable_x_step();
                break;

            default:
                fprintf(stderr, "a = %u\n", a);
                assert(false);
            }
        } else {
            load_x_interval(a);
            return;
        }
    }
}

void start_engine(void)
{
    if (!running_queues) {
        running_queues = qm_all;
        init_x_timer(x_intr, 1 * F_CPU / 1000);
        start_x_timer();
    }
}

static inline bool maybe_start_engine(void)
{
    if (!running_queues && any_queue_is_full()) {
        printf("start\n");
        start_engine();
        return true;
    }
    return false;
}

static inline void await_engine_stopped(void)
{
    while (running_queues)
        continue;
}

#define MIN_IVL 1024
#define MAX_IVL (65536L - MIN_IVL)

bool slow_step_x(uint32_t ivl)
{
    if (ivl > MAX_IVL) {
        assert(0);
        enqueue_atom_X(A_DISABLE_STEP);
        maybe_start_engine();
        while (ivl > MAX_IVL + MIN_IVL) {
            enqueue_atom_X(MAX_IVL);
            maybe_start_engine();
            ivl -= MAX_IVL;
        }            
        if (ivl > MAX_IVL) {
            enqueue_atom_X(ivl / 2);
            maybe_start_engine();
            ivl -= ivl / 2;
        }
        enqueue_atom_X(A_ENABLE_STEP);
        maybe_start_engine();
    }
    enqueue_atom_X(ivl);
    return maybe_start_engine();
}

#define STROKE_DISTANCE 500     // mm
#define A0              900     // mm/sec/sec
#define V0                0     // mm/sec
#define VMAX            400     // mm/sec

/*
 main:
    maj->A = A / TIMER_FREQ * 65536;
    maj->V = sqrt(2 * A);

 interrupt:
    uint16_t rate = ap->major.V0 +
                    (major_state.accel_time * ap->major.A >> 16);
    interval = convert_rate_to_interval(rate);

 gen-convert:
    convert_rate_to_interval(rate) => TIMER_FREQ / rate
*/

void engine_stroke_x(uint16_t dir_atom, uint16_t x_usteps)
{
    assert(dir_atom == A_DIR_POSITIVE || dir_atom == A_DIR_NEGATIVE);
    enqueue_atom_X(dir_atom);
    enqueue_atom_X(A_ENABLE_STEP);

#define uVMAX (MM_to_uSTEPS(VMAX))
#define uA0   (MM_to_uSTEPS(A0))
#define A     ((uint32_t)(256.0 * uA0 / F_CPU))

    // A is not well conditioned.  Need to calculate the shift on the fly.

    float v0 = sqrt(2 * uA0);
    uint32_t prev_t = 0;
    uint32_t interval;
    uint16_t rate = uVMAX;
    uint16_t s;
    for (s = 0; s < x_usteps / 2; s++) {
        rate = v0 + (prev_t * A >> 8);
        if (rate > uVMAX) {
            rate = uVMAX;
            break;
        }
        interval = convert_rate_to_interval(rate);
        assert(interval);
        prev_t += interval;
        if (slow_step_x(interval))
            printf("s=%d accel prev_t=%lu rate=%u interval=%lu\n",
                   s, prev_t, rate, interval);
    }
    interval = convert_rate_to_interval(rate);
    uint16_t s1 = x_usteps - s;
    for ( ; s < s1; s++) {
        if (slow_step_x(interval))
            printf("s=%d cruise\n", s);
    }
    for ( ; s < x_usteps; s++) {
        rate = v0 + (prev_t * A >> 8);
        assert(rate > 0);
        interval = convert_rate_to_interval(rate);
        prev_t -= interval;
        if (slow_step_x(interval))
            printf("s=%d decel\n", s);
    }
#undef A
#undef uA0
#undef uVMAX
    start_engine();
}

int main()
{
    init_stdio();
    init_serial();
    init_ATX_power();
    init_limit_switches();
    init_timer();
    init_motors();
    init_SPI();
    sei();

    enable_ATX_power();
    repeat_LEDs_off();
    enable_x_motor();
    home_x();
    ramp_LEDs(0, 127, 1000);

    while (true) {
        engine_stroke_x(A_DIR_POSITIVE, MM_to_uSTEPS(STROKE_DISTANCE));
        engine_stroke_x(A_DIR_NEGATIVE, MM_to_uSTEPS(STROKE_DISTANCE));
    }
}
