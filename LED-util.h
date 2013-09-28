#ifndef LED_UTIL_included
#define LED_UTIL_included

#include <stdint.h>

extern void repeat_LEDs_off(void);

extern void ramp_LEDs(uint8_t initial, uint8_t final, uint16_t ms);

__attribute__((noreturn))
extern void soothing_green_glow(void);

#endif /* !LED_UTIL_included */
