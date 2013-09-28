#ifndef ACTION_RING_INCLUDED
#define ACTION_RING_INCLUDED

#include <stddef.h>

#include "action.h"

// The lifecycle of an action.
//
// Producer:
//
//    action *p = alloc_action();
//    if (p == NULL) {
//        [ring is full; try again later.];
//    } else {
//        p->[whatever] = [whatever];
//        enqueue_action(p);
//    }
//
// Consumer:
//    action *p = dequeue_action(p);
//    if (p == NULL) {
//        [no actions available];
//    } else {
//        [do stuff with action]
//        discard_action(p);
//    }

extern void init_action_ring(void);

extern action *alloc_action(void);
extern void discard_action(action *);

extern void enqueue_action(action *);
extern action *dequeue_action(void);

// Module private variables.  Exposed here for the inline functions below.

#define PRIV action_ring_private

enum { ACTION_RING_SIZE = 8 };
extern struct PRIV {
    action ring[ACTION_RING_SIZE];
    action *alloc_next;
    action *discard_next;
    action *dequeue_next;
    action *enqueue_next;
} PRIV;

#define ACTION_RING_INC(p) \
    ((void)(++(p) == PRIV.ring + ACTION_RING_SIZE && ((p) = PRIV.ring)))

// Fast consumer functions for ISRs.
// They are inlined for speed and have no locking.
// Call only when interrupts are already disabled.

static inline action *dequeue_action_NO_LOCK(void)
{
    action *p = PRIV.dequeue_next;
    if (p) {
        ACTION_RING_INC(PRIV.dequeue_next);
        if (PRIV.dequeue_next == PRIV.enqueue_next)
            PRIV.dequeue_next = NULL;
    }
    return p;
}

static inline void discard_action_NO_LOCK(action *p)
{
    ACTION_RING_INC(PRIV.discard_next);
    if (!PRIV.alloc_next)
        PRIV.alloc_next = p;
}

#undef PRIV

#endif /* !ACTION_RING_INCLUDED */

