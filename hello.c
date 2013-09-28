/* hello - write "Hello, World!" to the serial port once a second. */

#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>


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


// // //  Serial Output  // // //

const uint32_t BAUD_RATE = 9600;

static inline void init_serial()
{
    const uint16_t baud_setting = F_CPU / 8 / BAUD_RATE - 1;

    // Enable double speed operation.
    UCSR0A = _BV(U2X0);

    // Set baud rate.
    UBRR0 = baud_setting;

    // Enable RX and TX.
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
}

static inline void write_char(char c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

static inline void serial_write_string(const char *s)
{
    while (*s)
        write_char(*s++);
}


// // //  Main Program  // // //

int main()
{
    init_timer();
    init_serial();
    while (1) {
        delay_millis(1000);
        serial_write_string("Hello, World!\n");
    }
}
