#ifndef ACTION_INCLUDED
#define ACTION_INCLUDED

#include <stdint.h>

typedef enum axis_bits {
    X_AXIS       = 1 << 0,
    Y_AXIS       = 1 << 1,
    Z_AXIS       = 1 << 2,
    P_AXIS       = 1 << 3,
    ALL_AXES     = X_AXIS | Y_AXIS | Z_AXIS | P_AXIS,
    HOME_BIT     = 1 << 7,
} axis_bits;

typedef enum action_type {

    STOP         = 0,

    MOVE_X_MAJOR = X_AXIS,
    MOVE_Y_MAJOR = Y_AXIS,
    MOVE_Z_MAJOR = Z_AXIS,
    MOVE_P_MAJOR = P_AXIS,

    HOME_X_MAJOR = X_AXIS | HOME_BIT,
    HOME_Y_MAJOR = Y_AXIS | HOME_BIT,
    HOME_Z_MAJOR = Z_AXIS | HOME_BIT,

} action_type;

typedef struct major_action {
    uint16_t     accel_usteps;  // acceleration distance, usteps
    uint16_t     cruise_usteps; // cruise distance,       usteps
    uint16_t     decel_usteps;  // deceleration distance, usteps
    uint16_t     cruise_ticks;  // cruise 1/velocity,     ticks/ustep
    uint16_t     V0;            // initial velocity,      usteps/sec
    uint16_t     Vmax;          // max velocity,          usteps/sec
    uint32_t     A;             // acceleration,          usteps/sec/tick << 16
} major_action;

typedef struct minor_action {
    char         TBD;
} minor_action;

typedef struct pulse_action {
    char         TBD;
} pulse_action;

typedef struct action {
    action_type  type:8;
    axis_bits    active_axes:8;
    axis_bits    positive_axes:8;
    major_action major;
    pulse_action pulse;
    minor_action x;
    minor_action y;
    minor_action z;
    minor_action p;
} action;

#endif /* !ACTION_INCLUDED */

