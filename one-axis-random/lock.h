#ifndef LOCK_INCLUDED
#define LOCK_INCLUDED

#include <avr/interrupt.h>

#define WITH_LOCK(statements)                                           \
    do {                                                                \
        uint8_t TEMPVAR_(sreg_) = SREG;                                 \
        cli();                                                          \
        {                                                               \
            statements                                                  \
        }                                                               \
        SREG = TEMPVAR_(sreg_);                                         \
    } while (0)

#define TEMPVAR_(prefix) CAT_(prefix,__LINE__)
#define CAT_(a,b) a##b

#endif /* !LOCK_INCLUDED */

