#ifndef MOTOR_UTIL_included
#define MOTOR_UTIL_included

#include <stdint.h>

#define MM_to_uSTEPS(mm)   ((int32_t)((mm) * 2000.0 / 25.4))
#define uSEC_per_uSTEP(mm_per_sec) \
                           ((uint16_t)(25.4 / 2000 / (mm_per_sec) * 1e6))
#define TICKS_per_uSTEP(mm_per_sec) \
                           (F_CPU * 25.4 / 2000.0 / (mm_per_sec))


extern void move_x  (int32_t usteps);
extern void move_y  (int32_t usteps);
extern void move_xy (int32_t x_usteps, int32_t y_usteps);
extern void home_x  (void);
extern void home_y  (void);
extern void home_xy (void);

#endif /* !MOTOR_UTIL_included */
