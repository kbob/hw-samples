#include <avr/io.h>

#include "ATX_power.h"
#include "serial.h"

static void serial_write_string(const char *s)
{
    while (*s)
        serial_write_byte(*s++);
}

int main()
{
    init_ATX_power();
    init_serial();
    while (1) {
        uint16_t ce = serial_read_byte();
        uint8_t e = ce >> 8;
        uint8_t c = ce;
        if (e)
            serial_write_string("Serial error.\r\n");
        else if (c == '0') {
            serial_write_string("Power down\r\n");
            disable_ATX_power();
        } else if (c == '1') {
            serial_write_string("Power up\r\n");
            enable_ATX_power();
        } else {
            if (ATX_power_state())
                serial_write_string("Power is on\r\n");
            else
                serial_write_string("Power is off\r\n");
        }
    }
}
