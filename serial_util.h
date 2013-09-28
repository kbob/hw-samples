#ifndef SERIAL_UTIL_included
#define SERIAL_UTIL_included

#include <stdint.h>

extern void serial_write_string(const char *s);

extern void serial_write_u16(uint16_t n);

#endif /* !SERIAL_UTIL_included */
