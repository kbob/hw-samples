#ifndef LED_UTIL_included
#define LED_UTIL_included

#include <stdint.h>

// Colors and levels are 0 .. 127.

// Refresh LEDs individually.
extern void begin_LEDs_refresh(void);
extern void set_pixel_color(uint8_t red, uint8_t green, uint8_t blue);
extern void set_pixel_level(uint8_t level);
extern void end_LEDs_refresh(void);

// Set all LEDs.
extern void set_LEDs_color(uint8_t red, uint8_t green, uint8_t blue);
extern void set_LEDs_level(uint8_t level);

extern void repeat_LEDs_off(void);

extern void ramp_LEDs(uint8_t initial, uint8_t final, uint16_t ms);

__attribute__((noreturn))
extern void soothing_green_glow(void);

#endif /* !LED_UTIL_included */
