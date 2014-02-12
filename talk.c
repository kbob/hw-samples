/* talk - generate occasional lines of text. */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <avr/interrupt.h>

#include "serial.h"
#include "stdio_util.h"
#include "timer.h"

#define MSG_INTERVAL_MSEC 1000

const char *strings[] = {
    "Hello",
    "Who's on First?",
    "What's on second?",
    "Who's on third?",
    "Good Bye",
};
const size_t string_count = sizeof strings / sizeof *strings;

const char *letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz"; 

int main()
{
    set_baud_rate(115200);
    init_stdio();
    init_timer();
    init_serial();
    sei();

#if 0
    uint32_t next = millisecond_time() + MSG_INTERVAL_MSEC;
    int i = 0;
    while (true) {
        while (millisecond_time() < next)
            continue;
        next += MSG_INTERVAL_MSEC;
        printf("talk: %s\n", strings[i]);
        if (++i == string_count)
            i = 0;
    }
#else
    uint32_t next = millisecond_time() + MSG_INTERVAL_MSEC;
    uint8_t n = strlen(letters);
    while (true) {
        while (millisecond_time() < next)
            continue;
        next += MSG_INTERVAL_MSEC;
        for (uint8_t i = 0; i < 20; i++) {
            for (uint8_t j = 0; j < n; j++)
                putchar(letters[(i + j) % n]);
            putchar('\n');
        }
        putchar('\n');
    }
#endif
}
