#ifndef MOTORS_included
#define MOTORS_included

#include <avr/io.h>
#include <util/delay.h>

// X Motor
//   Enable:     PD7 (enabled  = low)
//   Direction:  PF1 (positive = low)
//   Step:       PE4
//   Timer:      TIMER3
//   Comparator: OC3B

#define DDRx_enable     DDRD
#define DDxn_enable     DDD7
#define DDRx_dir        DDRF
#define DDxn_dir        DDF1
#define DDRx_step       DDRE
#define DDxn_step       DDE4

#define PORTx_enable    PORTD
#define PORTxn_enable   PORTD7
#define PORTx_dir       PORTF
#define PORTxn_dir      PORTF1
#define PORTx_step      PORTE
#define PORTxn_step     PORTE4

#define TCCRxA          TCCR3A
#define COMxx1_pulse    COM3B1
#define COMxx0_pulse    COM3B0
#define WGMx1           WGM31
#define WGMx0           WGM30

#define TCCRxB          TCCR3B
#define CSx2            CS32
#define CSx1            CS31
#define CSx0            CS30
#define WGMx3           WGM33
#define WGMx2           WGM32

#define TCNTx           TCNT3
#define OCRxx_pulse     OCR3B
#define ICRx            ICR3

#define TIMSKx          TIMSK3
#define TOIEx           TOIE3

#define TIFRx           TIFR3
#define TOVx            TOV3

#define TIMERx_OVF_vect TIMER3_OVF_vect


// Y Motor
//   Enable:    PF2 (enabled  = low)
//   Direction: PF7 (positive = low)
//   Step:      PB5
//   Timer:      TIMER1
//   Comparator: OC1A


#define DDRy_enable     DDRF
#define DDyn_enable     DDF2
#define DDRy_dir        DDRF
#define DDyn_dir        DDF7
#define DDRy_step       DDRB
#define DDyn_step       DDB5

#define PORTy_enable    PORTF
#define PORTyn_enable   PORTF2
#define PORTy_dir       PORTF
#define PORTyn_dir      PORTF7
#define PORTy_step      PORTB
#define PORTyn_step     PORTB5

#define TCCRyA          TCCR1A
#define COMyy1_pulse    COM1A1
#define COMyy0_pulse    COM1A0
#define WGMy1           WGM11
#define WGMy0           WGM10

#define TCCRyB          TCCR1B
#define CSy2            CS12
#define CSy1            CS11
#define CSy0            CS10
#define WGMy3           WGM13
#define WGMy2           WGM12

#define TCNTy           TCNT1
#define OCRyy_pulse     OCR1A
#define ICRy            ICR1

#define TIMSKy          TIMSK1
#define TOIEy           TOIE1

#define TIFRy           TIFR1
#define TOVy            TOV1

#define TIMERy_OVF_vect TIMER1_OVF_vect

typedef void isr_func(void);

static inline void init_motors(void)
{
    // X Motor
    PORTx_enable |=  _BV(PORTxn_enable); // disable
    PORTx_step   &= ~_BV(PORTxn_step);   // set step low
    DDRx_enable  |=  _BV(DDxn_enable);
    DDRx_dir     |=  _BV(DDxn_dir);
    DDRx_step    |=  _BV(DDxn_step);

    // Y Motor
    PORTy_enable |=  _BV(PORTyn_enable); // disable
    PORTy_step   &= ~_BV(PORTyn_step);   // set step low
    DDRy_enable  |=  _BV(DDyn_enable);
    DDRy_dir     |=  _BV(DDyn_dir);
    DDRy_step    |=  _BV(DDyn_step);
}



//  X  // //  X  // //  X  // //  X  // //  X  // //  X  // //  X  // //  X  //

extern void init_x_timer(isr_func *, uint16_t);

static inline void start_x_timer(void)
{
    // TCCRxB |= _BV(CSx0);
    TCCRxB = _BV(WGMx3) | _BV(WGMx2) | _BV(CSx0);
}

static inline void stop_x_timer(void)
{
    // TCCRxB &= ~_BV(CSx0);
    TCCRxB = 0;
    // TCCRxA &= ~(_BV(COMx1_pulse) | _BV(COMx0_pulse));
    TCCRxA = 0;
    TIMSKx = 0;
    TIFRx = 0;
}

static inline void enable_x_motor(void)
{
    PORTx_enable &= ~_BV(PORTxn_enable);
}

static inline void disable_x_motor(void)
{
    PORTx_enable |= _BV(PORTxn_enable);
}

static inline void set_x_direction_positive(void)
{
    PORTx_dir &= ~_BV(PORTxn_dir);
    _delay_us(1);
}

static inline void set_x_direction_negative(void)
{
    PORTx_dir |= _BV(PORTxn_dir);
    _delay_us(1);
}

static inline void step_x(void)
{
    PORTx_step |= _BV(PORTxn_step);
    _delay_us(1);
    PORTx_step &= ~_BV(PORTxn_step);
    _delay_us(1);
}

static inline void enable_x_step(void)
{
    TCCRxA |= _BV(COMxx1_pulse);
}

static inline void disable_x_step(void)
{
    TCCRxA &= ~_BV(COMxx1_pulse);
}

static inline void load_x_interval(uint16_t interval)
{
    ICRx = interval;
}


//  Y  // //  Y  // //  Y  // //  Y  // //  Y  // //  Y  // //  Y  // //  Y  //

extern void init_y_timer(isr_func *, uint16_t);

static inline void start_y_timer(void)
{
    // TCCRyB |= _BV(CSy0);
    TCCRyB = _BV(WGMy3) | _BV(WGMy2) | _BV(CSy0);
}

static inline void stop_y_timer(void)
{
    // TCCRyB &= ~_BV(CSy0);
    TCCRyB = 0;
    // TCCRyA &= ~(_BV(COMy1_pulse) | _BV(COMy0_pulse));
    TCCRyA = 0;
    TIMSKy = 0;
    TIFRy = 0;
}

static inline void enable_y_motor(void)
{
    PORTy_enable &= ~_BV(PORTyn_enable);
}

static inline void disable_y_motor(void)
{
    PORTy_enable |= _BV(PORTyn_enable);
}

static inline void set_y_direction_positive(void)
{
    PORTy_dir |= _BV(PORTyn_dir);
    _delay_us(1);
}

static inline void set_y_direction_negative(void)
{
    PORTy_dir &= ~_BV(PORTyn_dir);
    _delay_us(1);
}

static inline void step_y(void)
{
    PORTy_step |= _BV(PORTyn_step);
    _delay_us(1);
    PORTy_step &= ~_BV(PORTyn_step);
    _delay_us(1);
}

static inline void enable_y_step(void)
{
    TCCRyA |= _BV(COMyy1_pulse);
}

static inline void disable_y_step(void)
{
    TCCRyA &= ~_BV(COMyy1_pulse);
}

static inline void load_y_interval(uint16_t interval)
{
    ICRy = interval;
}

#endif /* !MOTORS_included */
