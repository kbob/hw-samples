#ifndef SERIAL_INCLUDED
#define SERIAL_INCLUDED

#include <stdbool.h>

#include <avr/io.h>

typedef enum serial_error_bit {
    SE_OK           = 0,
    SE_NO_DATA      = _BV(UDRE0),
    SE_FRAME_ERROR  = _BV(FE0),
    SE_DATA_OVERRUN = _BV(DOR0),
    SE_PARITY_ERROR = _BV(UPE0),
} serial_error_bit;

extern void    set_baud_rate(uint32_t rate);
extern void    init_serial(void);
extern uint8_t serial_error(void);

// The read functions either return error bits in the high byte
// or, if the high byte is zero, the received char in the low byte.

extern bool    serial_data_ready             (void);
extern int16_t serial_read_byte              (void);
extern int16_t serial_read_byte_NONBLOCKING  (void);
extern void    serial_write_byte             (char);
extern uint8_t serial_write_byte_NONBLOCKING (char);

#endif /* !SERIAL_INCLUDED */
