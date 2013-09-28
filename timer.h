#ifndef TIMER_INCLUDED
#define TIMER_INCLUDED

#include <stdint.h>

extern void     init_timer         (void);
extern uint32_t millisecond_time   (void);
extern void     delay_milliseconds (uint32_t ms);

#endif /* !TIMER_INCLUDED */
