#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "serial.h"
#include "stdio_util.h"
#include "usec-timer.h"

#define CLOCK_CYCLES_PER_MICROSECOND (F_CPU / 1000000L)

const int32_t LONG_PAUSE = 2000; // milliseconds
const int32_t SHORT_PAUSE = 200; // milliseconds

typedef void (*bench_proc)(void);

typedef struct benchmark {
    bench_proc  bm_proc;
    uint32_t    bm_repetitions;
    const char *bm_label;
} benchmark;

void null_bench(void)
{}

uint32_t a = 7654321, b = 1234567, c;
void div32_bench(void)
{
    c = a / b;
}

uint32_t divu32(uint32_t a, uint32_t b)
{
    uint32_t c = b;
    uint32_t d = 1;
    while (1) {
        uint32_t c1 = c << 1;
        if (c1 > a)
            break;
        c = c1;
        d <<= 1;
    }
    uint32_t q = 0;
    while (d) {
        if (a >= c) {
            a -= c;
            q += d;
        }
        c >>= 1;
        d >>= 1;
    }
    return q;
}
void bob32_large_bench(void)
{
    c = divu32(a, b);
}

void add_u32_bench(void)
{
    c = a + b;
}

uint64_t a64, b64, c64;
void add_u64_bench(void)
{
    c64 = a64 + b64;
}

void micros_bench(void)
{
    c = usec_time();
}

uint32_t aa = 213, bb = 41, cc;
void bob32_small_bench(void)
{
    cc = divu32(aa, bb);    
}

void inline_write_bench(void)
{
    PORTB |= _BV(PB5);
}

uint16_t u16;

void calc_velocity(void)
{
#define V0 0
#define VMAX 300
#define A0 100
#define uV0   (V0   * 2000.0 / 25.4)
#define uVMAX (VMAX * 2000.0 / 25.4)
#define uA0   (A0   * 2000.0 / 25.4)
    float v = sqrt(uV0 * uV0 + 2 * uA0 * (u16 + 0.5));
    // printf("V0 * V0 = %g\n", (double)(V0 * V0));
    // printf("2 * %g * (%u + 0.5) = %g\n", uA0, s, 2 * uA0 * (s + 0.5));
    // printf("sqrt(%g) = %g\n",
    //        V0*V0 + 2 * uA0 * (s + 0.5), sqrt(V0*V0 + 2 * uA0 * (s + 0.5)));
    if (v > uVMAX)
        v = uVMAX;
    float t = (v - uV0) * (1.0 / uA0);
    // printf("v = %g, t = %g\n", (double)v, (double)t);
    uint32_t ivl = t * F_CPU - bb;
    cc = ivl;
#undef uA0
#undef uVMAX
#undef uV0
#undef A0
#undef VMAX
#undef V0
}

float f;
float small_f = 2.0;
float large_f = 123456789.0;
int16_t result_i16;

void float_mul(void)
{
    f = small_f * large_f;
}

void float_div(void)
{
    f = small_f / large_f;
}

void float_to_int16(void)
{
    result_i16 = large_f;
}

void float_sqrt_small(void)
{
    f = sqrt(small_f);
}

void float_sqrt_large(void)
{
    f = sqrt(large_f);
}

uint32_t i_small = 10;
uint32_t i_large = 12345678;

void int32_sqrt_small(void)
{
    float f = (float)i_small;
    a = (uint32_t)sqrt(f);
}

void int32_sqrt_large(void)
{
    float f = (float)i_large;
    a = (uint32_t)sqrt(f);
}

uint16_t isqrt32(uint32_t v)
{
    uint32_t t, r = 0;
    for (t = 0x40000000; t; t >>= 2) {
        if (t + r <= v) {
            v -= t + r;
            r = r >> 1 | t;
        } else
            r >>= 1;
    }
    return (uint16_t)r;
}

uint8_t isqrt16(uint16_t v)
{
#if 0
    uint16_t t, r = 0;
    for (t = 0x4000; t; t >>= 2) {
        if (t + r <= v) {
            v -= t + r;
            r = r >> 1 | t;
        } else
            r >>= 1;
    }
    return (uint16_t)r;
#else
    uint16_t t = 0x4000, r = 0;
    if (t + r <= v) {
        v -= t + r;
        r = t;
    }
    t >>= 2; // t = 0x1000;
    if (t + r <= v) {
        v -= t + r;
        r = r >> 1 | t;
    } else
        r >>= 1;
    t >>= 2; // t = 0x0400;
    if (t + r <= v) {
        v -= t + r;
        r = r >> 1 | t;
    } else
        r >>= 1;
    t >>= 2; // t = 0x0100;
    if (t + r <= v) {
        v -= t + r;
        r = r >> 1 | t;
    } else
        r >>= 1;
    t = 0x0040;
    if (t + r <= v) {
        v -= t + r;
        r = r >> 1 | t;
    } else
        r >>= 1;
    t >>= 2; // t = 0x0010;
    if (t + r <= v) {
        v -= t + r;
        r = r >> 1 | t;
    } else
        r >>= 1;
    t >>= 2; // t = 0x0004;
    if (t + r <= v) {
        v -= t + r;
        r = r >> 1 | t;
    } else
        r >>= 1;
    t >>= 2; // t = 0x0001;
    if (t + r <= v) {
        r = r >> 1 | t;
    } else
        r >>= 1;
    return r;
#endif
}

uint16_t c16;
uint8_t c8;

void isqrt32_small(void)
{
    c16 = isqrt32(a);
}

void isqrt32_large(void)
{
    c16 = isqrt32(b);
}

void isqrt16_small(void) {
    c8 = isqrt16(a);
}

void isqrt16_large(void)
{
    c8 = isqrt16(b);
}

// //// //// //// //// //// //// //// //// //// //// //// //// //// //// ////

const benchmark benchmarks[] = {
    { null_bench,          100000, "null"             },
    { div32_bench,         100000, "divide 32"        },
    { bob32_large_bench,   100000, "divu32 large"     },
    { bob32_small_bench,   100000, "divu32 small"     },
    { add_u32_bench,       100000, "add u32"          },
    { add_u64_bench,       100000, "add u64"          },
    { micros_bench,        100000, "micros"           },
    // { delay_micro_bench,   100000, "delay 1 micro"    },
    // { digital_write_bench, 100000, "digital write"    },
    { inline_write_bench,  100000, "inline write"     },
    { float_sqrt_small,    100000, "float sqrt small" },
    { float_sqrt_large,    100000, "float sqrt large" },
    { int32_sqrt_small,    100000, "int32 sqrt small" },
    { int32_sqrt_large,    100000, "int32 sqrt large" },
    { isqrt32_small,       100000, "isqrt 32 large"   },
    { isqrt32_large,       100000, "isqrt 32 small"   },
    { isqrt16_small,       100000, "isqrt 16 large"   },
    { isqrt16_large,       100000, "isqrt 16 small"   },
};

uint16_t bench_count = sizeof benchmarks / sizeof benchmarks[0];
uint16_t next_bench;

uint32_t time_proc(void *vproc, uint32_t nrep)
{
    bench_proc proc = (bench_proc)vproc;
    uint32_t t0 = usec_time();
    for (uint32_t i = 0; i < nrep; i++)
        (*proc)();
     uint32_t t1 = usec_time();
     return t1 - t0;
}

void run_benchmark(const struct benchmark *bm)
{
    bench_proc proc = bm->bm_proc;
    uint32_t nrep = bm->bm_repetitions;
    uint32_t tare = 0;
    if (proc != null_bench)
       tare = time_proc((void *)null_bench, nrep);
    uint32_t gross = time_proc((void *)proc, nrep);
    uint32_t net = gross - tare;
    
    // bench_label    100 reps   1234567 usec   123 cycles/rep
  
    double usec_per_rep = (float)net / (float)nrep;
    double cycles_per_rep = usec_per_rep * CLOCK_CYCLES_PER_MICROSECOND;
    printf("%-16s   %7ld reps   %6.2f usec/rep   %6.2f cycles/rep\n",
           bm->bm_label, nrep, usec_per_rep, cycles_per_rep);
    _delay_ms(SHORT_PAUSE);
}

int main()
{
    init_serial();
    init_stdio();
    init_usec_timer();
    sei();

    for (uint8_t i = 0; i < bench_count; i++)
        run_benchmark(&benchmarks[i]);

    printf("\n");
    while (1)
        continue;
}
