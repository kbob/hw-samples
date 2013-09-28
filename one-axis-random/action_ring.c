#include "action_ring.h"

#include <stdlib.h>

#include "lock.h"

#define PRIV action_ring_private

struct PRIV PRIV;

void init_action_ring(void)
{
    action *ring = PRIV.ring;
    PRIV.alloc_next   = ring;
    PRIV.discard_next = ring;
    PRIV.enqueue_next = ring;
    PRIV.dequeue_next = NULL;
}

action *alloc_action(void)
{
    action *p;
    WITH_LOCK( {
        p = PRIV.alloc_next;
        if (p) {
            ACTION_RING_INC(PRIV.alloc_next);
            if (PRIV.alloc_next == PRIV.discard_next)
                PRIV.alloc_next = NULL;
        }
    } );
    return p;
}

void discard_action(action *p)
{
    WITH_LOCK( {
        discard_action_NO_LOCK(p);
    } );
}

void enqueue_action(action *p)
{
    WITH_LOCK( {
        ACTION_RING_INC(PRIV.enqueue_next);
        if (!PRIV.dequeue_next)
            PRIV.dequeue_next = p;
    } );
}

action *dequeue_action(void)
{
    action *p;
    WITH_LOCK( {
        p = dequeue_action_NO_LOCK();
    } );
    return p;
}

