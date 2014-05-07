// X min, Y min, Door, E-Stop
#include "safety-switches.h"

#include <stdio.h>

#include <avr/interrupt.h>

#include "limit-switches.h"
#include "serial.h"
#include "stdio_util.h"

typedef struct switch_state {
    union {
        struct {
            bool xmin;
            bool ymin;
            bool stop;
            bool lid;
        };
        uint32_t w;
    };
} switch_state;

static switch_state read_switches(void)
{
    switch_state s;
    s.xmin = x_min_reached();
    s.ymin = y_min_reached();
    s.stop = e_is_stopped();
    s.lid  = lid_is_open();
    return s;
}

int main()
{
    init_serial();
    init_stdio();
    init_limit_switches();
    init_safety_switches();
    sei();

    uint8_t ph_counter = 0;
    switch_state prev = { .w = 0xFFFFFFFF };
    while (true) {
        switch_state curr = read_switches();
        if (curr.w != prev.w) {
            if (!ph_counter) {
                printf("\nX-MIN Y-MIN STOP  LID\n");
                ph_counter = 10;
                prev.w = 0xFFFFFFFF;
            }
            const char *x = curr.xmin ? "lim " : "    ";
            const char *y = curr.ymin ? "lim " : "    ";
            const char *e = curr.stop ? "stop" : "    ";
            const char *l = curr.lid  ? "open" : "    ";
            printf("%4s  %4s  %4s  %4s\n", x, y, e, l);
            prev = curr;
            ph_counter--;
        }
    }
}
