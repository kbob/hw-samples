/* echo - read from serial port, send it back after a delay. */

#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "serial.h"


// // //  Millisecond Timer  // // //

volatile uint16_t ticks;

static inline void init_timer(void)
{
    // Timer/Counter 0 waveform generation mode = fast PWM, TOP = OCR0A.
    TCCR0A = _BV(WGM00) | _BV(WGM01) | _BV(WGM02);

    // Timer/Counter 0 clock select = prescale by 64.
    TCCR0B = _BV(CS00) | _BV(CS01);

    // Output Compare Register 0A = 250 (1 millisecond);
    OCR0A = F_CPU / 64 / 1000;

    // Enable Timer/Counter 0 overflow interrupt.
    TIMSK0 |= _BV(TOIE0);

    // Enable interrupts.
    sei();
}

ISR(TIMER0_OVF_vect)
{
    ticks++;
}

static inline uint16_t get_ticks(void)
{
    uint16_t t;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        t = ticks;
    }
    return t;
}

static inline void delay_millis(uint16_t ms)
{
    uint16_t end = get_ticks() + ms;
    while (get_ticks() != end)
        continue;
}


// // //  Main Program  // // //

static inline void serial_write_string(const char *s)
{
    while (*s)
        serial_write_byte(*s++);
}

static inline void write_hex16(uint16_t n)
{
    static const char tt[] = "0123456789abcdef";
    serial_write_byte(tt[n >> 12 & 0xF]);
    serial_write_byte(tt[n >> 8 &  0xF]);
    serial_write_byte(tt[n >> 4 &  0xF]);
    serial_write_byte(tt[n >> 0 &  0xF]);
}

static inline void write_hex8(uint8_t n)
{
    static const char tt[] = "0123456789abcdef";
    serial_write_byte(tt[n >> 4 &  0xF]);
    serial_write_byte(tt[n >> 0 &  0xF]);
}

int main()
{
    init_timer();
    init_serial();
    while (1) {
        uint8_t x = serial_error();
        if (x) {
            serial_write_string(" !");
            write_hex8(x);
            serial_write_string("! ");
        }
        uint16_t cs = serial_read_byte();
        if (cs >> 8) {
            serial_write_byte('\7');
            serial_write_byte('!');
        } else {
            delay_millis(500);
            serial_write_byte(cs);
        }
    }
}
