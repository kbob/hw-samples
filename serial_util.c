#include "serial_util.h"

#include "serial.h"

void serial_write_string(const char *s)
{
    while (*s)
        serial_write_byte(*s++);
}

static void serial_write_u16_(uint16_t n)
{
    uint16_t d = n;
    if (n >= 10) {
        serial_write_u16_(n / 10);
        d = n % 10;
    }
    if (n)
        serial_write_byte('0' + d);
}

void serial_write_u16(uint16_t n)
{
    if (n)
        serial_write_u16_(n);
    else
        serial_write_byte('0');
}
