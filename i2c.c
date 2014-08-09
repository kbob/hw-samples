#include "i2c.h"

#include <assert.h>
#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/twi.h>

#define I2C_BIT_RATE 100000
#define I2C_MAX 3

#define I2C_CLOCK_pullup                    true
#define I2C_CLOCK
#define I2C_CLOCK_DDR_reg                   DDRD
#define I2C_CLOCK_DD_bit                    DDD0
#define I2C_CLOCK_PIN_reg                   PIND
#define I2C_CLOCK_PIN_bit                   PIND0
#define I2C_CLOCK_PORT_reg                  PORTD
#define I2C_CLOCK_PORT_bit                  PORTD0

#define I2C_DATA_pullup                     true
#define I2C_DATA
#define I2C_DATA_DDR_reg                    DDRD
#define I2C_DATA_DD_bit                     DDD1
#define I2C_DATA_PIN_reg                    PIND
#define I2C_DATA_PIN_bit                    PIND1
#define I2C_DATA_PORT_reg                   PORTD
#define I2C_DATA_PORT_bit                   PORTD1

typedef enum i2c_state {
    IS_UNINIT,
    IS_IDLE,
    IS_MTX_BUSY,
} i2c_state;

static i2c_state state = IS_UNINIT;

static uint8_t  *tx_data;
static uint8_t   tx_count;
static uint8_t   tx_status;
static uint8_t   buf[I2C_MAX + 1];


#define OR3(a,b,c)     (_BV(a) | _BV(b) | _BV(c))
#define OR4(a,b,c,d)   (_BV(a) | _BV(b) | _BV(c) | _BV(d))
#define OR5(a,b,c,d,e) (_BV(a) | _BV(b) | _BV(c) | _BV(d) | _BV(e))

// bits in order:      TWINT  TWEA  TWSTA  TWSTO  TWEN  TWIE

#define TWC_INIT  (OR3(       TWEA,               TWEN, TWIE))
#define TWC_START (OR5(TWINT, TWEA, TWSTA,        TWEN, TWIE))
#define TWC_CONT  (OR4(TWINT, TWEA,               TWEN, TWIE))
#define TWC_STOP  (OR5(TWINT, TWEA,        TWSTO, TWEN, TWIE))
#define TWC_ABORT (OR4(TWINT, TWEA,               TWEN, TWIE))

void init_i2c(void)
{
    // enable pull-ups on SCL and SDA.
    I2C_CLOCK_DDR_reg &= ~_BV(I2C_CLOCK_DD_bit);
    I2C_CLOCK_PORT_reg |= _BV(I2C_CLOCK_PORT_bit);

    I2C_CLOCK_DDR_reg &= ~_BV(I2C_CLOCK_DD_bit);
    I2C_CLOCK_PORT_reg |= _BV(I2C_CLOCK_PORT_bit);

    TWSR = 0;
    TWBR = (F_CPU / I2C_BIT_RATE - 16) / 2;
    TWCR = TWC_INIT;
    state = IS_IDLE;
}

void i2cm_transmit(uint8_t slave_addr, uint8_t *data, uint8_t  size)
{
    assert(size <= I2C_MAX);

    (void)i2cm_status();    // wait for previous transaction to finish
    buf[0] = slave_addr << 1 | TW_WRITE;
    for (uint8_t i = 0; i < size; i++)
        buf[i + 1] = data[i];
    tx_data = buf;
    tx_count = size + 1;
    TWCR = TWC_START;
}

uint8_t i2cm_status (void)
{
    while (state != IS_IDLE)
        continue;
    loop_until_bit_is_clear(TWCR, TWSTO);
    return tx_status;
}

ISR(TWI_vect)
{
    uint8_t tw_sts = TW_STATUS;
    switch (tw_sts) {

    case TW_START:
    case TW_MT_SLA_ACK:
    case TW_MT_DATA_ACK:
        if (tx_count) {
            // Transmit next byte.
            TWDR = *tx_data;
            TWCR = TWC_CONT;
            tx_data++;
            tx_count--;
        } else {
            // Done.  Transmit STOP.
            TWCR = TWC_STOP;
            tx_status = 0;
            state = IS_IDLE;
        }
        break;

    case TW_MT_SLA_NACK:
    case TW_MT_DATA_NACK:
    case TW_MT_ARB_LOST:
    default:
        tx_status = tw_sts;
        TWCR = TWC_ABORT;
        break;
    }
}
