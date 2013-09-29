// This program drives a single motor through a randomly generated
// sequence of actions.  The motor is pulsed by a timer/counter, and
// the timer/counter triggers an interrupt whose ISR calculates and
// loads the interval until the next pulse and interrupt.
//
// The parameters of each action are calculated a little before
// they are needed.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../ATX_power.h"
#include "../serial.h"
#include "../stdio_util.h"
#include "../timer.h"

#include "action.h"
#include "action_ring.h"
#include "steppers.h"
#include "convert.h"

const float max_S  = 300;        // mm
const float min_S  = 0;          // mm
const float max_V  = 600;        // mm/sec
const float min_V  = 60;         // mm/sec
const float max_A  = 1800;       // mm/sec/sec
const float min_A  = 450;        // mm/sec/sec
const float rest_T = 5;          // sec
const uint8_t actions_between_rests = 25;

#define TIMER_FREQ (F_CPU)

const float usteps_per_mm = 78.7402;

#if 1
    #define P(x)   (Serial.print((x)))
    #define Pln(x) (Serial.println((x)))
    #define Pd(x)  (Serial.print(#x " = "), Serial.println((x), DEC))
    #define Px(x)  (Serial.print(#x " = 0x"), Serial.println((x), HEX))
    #define Pb(x)  (Serial.print(#x " = 0b"), Serial.print((x), BIN), \
                    Serial.print(" = 0x"), Serial.println((x), HEX))
#else
    #define P(x)   ((void)0)
    #define Pln(x) ((void)0)
    #define Pd(x)  ((void)0)
    #define Px(x)  ((void)0)
    #define Pb(x)  ((void)0)
#endif

static uint32_t start_delay;
uint16_t action_counter;

uint16_t uniform_random(void)
{
    static uint32_t seed = 17500728;  // JSB RIP
    seed *= 69069L;
    return seed >> 16;
}

uint16_t centered_random(void)
{
    uint32_t r0 = uniform_random();
    uint32_t r1 = uniform_random();
    uint32_t r2 = uniform_random();
    uint32_t r3 = uniform_random();
    uint16_t r = (r0 + r1 + r2 + r3) / 4;
    return r;
}

float scale(float x0, float x1, uint16_t r)
{
    return x0 + (x1 - x0) * (float)r / 65536.0;
}

float scale_usteps(float x0, float x1, uint16_t r)
{
    return usteps_per_mm * scale(x0, x1, r);
}

static void gen_random_action(action *ap)
{
    static uint16_t X_loc = 0;
    const bool rest = ++action_counter == actions_between_rests;
    const bool     fast = rest || (random() & 1) != 0;
    const uint16_t newX = rest ? 0 : 
                                 scale_usteps(min_S, max_S, centered_random());
    const bool     posX = newX >= X_loc;
    const uint16_t S    = posX ? newX - X_loc : X_loc - newX;
    float          V0   = 0;
    const float    Vmax = fast ? scale_usteps(min_V, max_V, random()) :
                                 usteps_per_mm * max_V;
    const float    A    = fast ? scale_usteps(min_A, max_A, random()) :
                                 usteps_per_mm * max_A;
    major_action  *maj  = &ap->major;
    
    uint32_t accel_usteps = (Vmax * Vmax - V0 * V0) / (2 * A);
    uint32_t decel_usteps = accel_usteps;
    maj->accel_usteps = maj->decel_usteps = (Vmax * Vmax - V0 * V0) / (2 * A);
    if (accel_usteps + decel_usteps < S) {
        maj->accel_usteps = accel_usteps;
        maj->decel_usteps = decel_usteps;
        maj->cruise_usteps = S - maj->accel_usteps - maj->decel_usteps;
        maj->cruise_ticks = convert_rate_to_interval(Vmax);
        maj->Vmax = Vmax;
    } else {
        maj->decel_usteps = S / 2;
        maj->accel_usteps = S - maj->decel_usteps;
        maj->cruise_usteps = 0;
        maj->cruise_ticks = 65535;
        maj->Vmax = sqrt(V0 * V0 + A * S);
    }
    if (V0 == 0)
        V0 = sqrt(2 * A);
    maj->V0 = V0;
    maj->A = A / TIMER_FREQ * 65536;

    ap->type = MOVE_X_MAJOR;
    ap->active_axes = X_AXIS;
    ap->positive_axes = posX ? X_AXIS : 0;
    // Remaining fields are unused; do not initialize.
    
    // Update current location.
    X_loc = newX;
    if (rest)
        action_counter = 0;
}

void setup(void)
{
    // Serial.begin(9600);
    init_action_ring();
    init_steppers();
    init_ATX_power();
    enable_ATX_power();
    start_delay = millisecond_time() + 2000;
}

void loop(void)
{
    if (millisecond_time() < start_delay)
        return;
    action *ap = alloc_action();
    if (!ap)
        return;
    gen_random_action(ap);
    enqueue_action(ap);
    start_steppers_if_idle(ap->type, ap->active_axes, ap->positive_axes);
        printf("TCCR3A = %#6x\n", TCCR3A);
        printf("TCCR3B = %#6x\n", TCCR3B);
        printf("TCCR3C = %#6x\n", TCCR3C);
        printf("TCNT3  = %#6x\n", TCNT3);
        printf("OCR3B  = %#6x\n", OCR3B);
        printf("ICR3   = %#6x\n", ICR3);
        printf("TIMSK3 = %#6x\n", TIMSK3);
        printf("TIFR3  = %#6x\n", TIFR3);
    
    if (action_counter == 0)
        start_delay = millisecond_time() + (uint32_t)(1000 * rest_T);
}

int main()
{
    init_serial();
    init_stdio();
    init_timer();
    setup();
    sei();
    while (1)
        loop();
}
