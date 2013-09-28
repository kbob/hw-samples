#include "limit-switches.h"

#include <avr/io.h>

#include "serial.h"
#include "timer.h"

static bool x_min, y_min, z_min, z_max;
static uint32_t print_time;

static void serial_write_string(const char *s)
{
    while (*s)
        serial_write_byte(*s++);
}

int main()
{
    init_limit_switches();
    init_timer();
    init_serial();
    print_time = millisecond_time() + 2000;
    while (1) {
        bool do_print = false;

        uint32_t time_now = millisecond_time();
        if ((int32_t)time_now - (int32_t)print_time >= 0) {
            do_print = true;
            print_time += 2000;
        }

        bool xmin = x_min_reached();
        if (x_min != xmin) {
            do_print = true;
            x_min = xmin;
        }
        bool ymin = y_min_reached();
        if (y_min != ymin) {
            do_print = true;
            y_min = ymin;
        }
        bool zmin = z_min_reached();
        if (z_min != zmin) {
            do_print = true;
            z_min = zmin;
        }
        bool zmax = z_max_reached();
        if (z_max != zmax) {
            do_print = true;
            z_max = zmax;
        }

        if (do_print) {
            serial_write_string("  X min: ");
            serial_write_string(x_min ? "on " : "off");
            serial_write_string("  Y min: ");
            serial_write_string(y_min ? "on " : "off");
            serial_write_string("  Z min: ");
            serial_write_string(z_min ? "on " : "off");
            serial_write_string("  Z max: ");
            serial_write_string(z_max ? "on " : "off");
            serial_write_string("\r\n");
        }
    }
}
