#include "stdio_util.h"

#include <stdio.h>

#include "serial.h"

static int my_getc(FILE *stream)
{
    int16_t c = serial_read_byte();
    if (c & 0xFF00)
        return _FDEV_ERR;
    else
        return c;
}

static int my_putc(char c, FILE *stream)
{
    if (c == '\n')
        serial_write_byte('\r');
    serial_write_byte(c);
    return 0;
}

static FILE my_stdin = FDEV_SETUP_STREAM(NULL, my_getc, _FDEV_SETUP_READ);
static FILE my_stdouterr = FDEV_SETUP_STREAM(my_putc, NULL, _FDEV_SETUP_WRITE);

void init_stdio(void)
{
    stdin = &my_stdin;
    stdout = stderr = &my_stdouterr;
}
