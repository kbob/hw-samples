#include "serial.h"

#include <avr/interrupt.h>
#include <util/atomic.h>

const uint32_t BAUD_RATE = 9600;


// // //  Ring Buffers  // // //

#define RING_BUF_SIZE 13

typedef struct ring_buf {
    uint8_t head;
    uint8_t tail;
    uint8_t errs;
    char    ring[RING_BUF_SIZE];
} ring_buf;

static ring_buf rx_ring;

#define RB_INC(pos) ((void)(++(pos) == RING_BUF_SIZE && ((pos) = 0)))

static inline bool rb_is_empty_NONATOMIC(ring_buf *rb)
{
    return rb->head == rb->tail;
}

static inline bool rb_is_empty(ring_buf *rb)
{
    bool empty;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        empty = rb_is_empty_NONATOMIC(rb);
    }
    return empty;
}

static inline bool rb_is_full_NONATOMIC(ring_buf *rb)
{
    uint8_t next = rb->tail;
    RB_INC(next);
    return rb->head == next;
}

//static inline bool rb_is_full(ring_buf *rb);

//static inline void rb_enqueue(ring_buf *rb, char);
//static inline void rb_enqueue_NONATOMIC(ring_buf *rb, char);
//static inline uint8_t rb_enqueue_NONBLOCKING(ring_buf *rb, char);

static inline uint8_t rb_enqueue_NONBLOCKING_NONATOMIC(ring_buf *rb, char c)
{
    if (rb_is_full_NONATOMIC(rb))
        return SE_DATA_OVERRUN;
    rb->ring[rb->tail] = c;
    RB_INC(rb->tail);
    return 0;
}

static inline char rb_dequeue(ring_buf *rb)
{
    while (rb_is_empty(rb))
        continue;
    char c = rb->ring[rb->head];
    RB_INC(rb->head);
    return c;
}

//static inline char rb_dequeue_NONATOMIC(ring_buf *rb);
//static inline uint16_t rb_dequeue_NONBLOCKING(ring_buf *rb);

static inline uint16_t rb_dequeue_NONBLOCKING_NONATOMIC(ring_buf *rb)
{
    if (rb_is_empty_NONATOMIC(rb))
        return (uint16_t)SE_NO_DATA << 8;
    char c = rb->ring[rb->head];
    RB_INC(rb->head);
    return c;
}


// // //  Serial I/O  // // //

static volatile uint8_t error_bits;

void init_serial(void)
{
    const uint16_t baud_setting = F_CPU / 8 / BAUD_RATE - 1;

    // Enable double speed operation.
    UCSR0A = _BV(U2X0);

    // Set baud rate.
    UBRR0 = baud_setting;

    // Enable RX, TX, and RX complete interrupt.
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);

    sei();
}

ISR(USART0_RX_vect)
{
    error_bits |= UCSR0A & (_BV(UPE0) | _BV(DOR0) | _BV(FE0));
    if (bit_is_set(UCSR0A, RXC0))
        error_bits |= rb_enqueue_NONBLOCKING_NONATOMIC(&rx_ring, UDR0);
}

uint8_t serial_error(void)
{
    uint8_t err = error_bits;
    error_bits = 0;
    return err;
}

bool serial_data_ready(void)
{
    if (error_bits)
        return true;
    bool ready;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ready = rx_ring.head != rx_ring.tail;
    }
    return ready;
}

int16_t serial_read_byte(void)
{
    while (!serial_data_ready())
        continue;
    
    uint16_t cs;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (error_bits) {
            cs = (uint16_t)error_bits << 8;
            error_bits = 0;
        } else
            cs = rb_dequeue(&rx_ring);
    }
    return cs;
}

int16_t serial_read_byte_NONBLOCKING(void)
{
    uint16_t cs;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (error_bits) {
            cs = (uint16_t)error_bits << 8;
            error_bits = 0;
        } else
            cs = rb_dequeue_NONBLOCKING_NONATOMIC(&rx_ring);
    }    
    return cs;
}

void serial_write_byte(char c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

uint8_t serial_write_byte_NONBLOCKING(char c)
{
    if (bit_is_clear(UCSR0A, UDRE0))
        return SE_NO_DATA;
    UDR0 = c;
    return 0;
}
