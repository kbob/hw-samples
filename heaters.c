#include <avr/interrupt.h>
#include <avr/io.h>

#include "ATX_power.h"
#include "heaters.h"
#include "serial.h"

static void serial_write_string(const char *s)
{
    while (*s)
        serial_write_byte(*s++);
}

int main()
{
    init_ATX_power();
    init_heaters();
    init_serial();
    sei();

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
        } else if (c == 'A') {
            serial_write_string("Heater 0 up\r\n");
            enable_heater_0();
        } else if (c == 'a') {
            serial_write_string("Heater 0 down\r\n");
            disable_heater_0();
        } else if (c == 'B') {
            serial_write_string("Heater 1 up\r\n");
            enable_heater_1();
        } else if (c == 'b') {
            serial_write_string("Heater 1 down\r\n");
            disable_heater_1();
        } else if (c == 'C') {
            serial_write_string("Heater 2 up\r\n");
            enable_heater_2();
        } else if (c == 'c') {
            serial_write_string("Heater 2 down\r\n");
            disable_heater_2();
        } else {
            if (ATX_power_state())
                serial_write_string("Power is on\r\n");
            else
                serial_write_string("Power is off\r\n");
            if (heater_0_state())
                serial_write_string("Heater 0 is on\r\n");
            else
                serial_write_string("Heater 0 is off\r\n");
            if (heater_1_state())
                serial_write_string("Heater 1 is on\r\n");
            else
                serial_write_string("Heater 1 is off\r\n");
            if (heater_2_state())
                serial_write_string("Heater 2 is on\r\n");
            else
                serial_write_string("Heater 2 is off\r\n");
        }
    }
}
