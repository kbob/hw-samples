#ifndef SPI_included
#define SPI_included

#include <stdbool.h>

#include <avr/io.h>
#include <util/atomic.h>


// SS   = PB0
// SCK  = PB1
// MOSI = PB2
// MISO = PB3

#define SPI_DIVIDER 2

static inline void init_SPI(void)
{
    // Set SS high.
    // PORTB |= _BV(PB0);
    PORTB &= ~_BV(PB0);

    // Set SS to output mode.
    DDRB |= _BV(DDB0);
    PORTB &= ~_BV(PB0);

    // Set SPI master mode and enabled.
    SPCR |= _BV(SPE) | _BV(MSTR);
    ////SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1);

    // Set SCK, MOSI to output mode.
    DDRB |= _BV(DDB1) | _BV(DDB2);

    // Set bit order MSB-first.
    SPCR &= ~_BV(DORD);

    // Set data mode 0.
    SPCR &= ~(_BV(CPOL) | _BV(CPHA));

    // Set clock divider

#if SPI_DIVIDER == 2            // 8 MHz
    // div 2: SPI2X = 1, SPR1 = 0, SPR0 = 0
    SPSR |= _BV(SPI2X);
#elif SPI_DIVIDER == 8          // 2 MHz
    // div 8: SPI2X = 1, SPR1 = 0, SPR0 = 1
    SPSR |= _BV(SPI2X);
    SPCR |= _BV(SPR0);
#elif SPI_DIVIDER == 64         // 250 KHz
    // div 64: SPI2X = 0, SPR1 = 1, SPR0 = 0
    SPCR |= _BV(SPR1);
#elif SPI_DIVIDER == 128        // 125 KHz
    // div 128: SPI2X = 0, SPR1 = 1, SPR0 = 1
    SPCR |= _BV(SPR1) | _BV(SPR0);
#else
    #error Bad SPI_DIVIDER
#endif
    
    // Enable interrupt.
    // SPCR |= _BV(SPIE);
}

static inline void stop_SPI(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        DDRB &= ~(_BV(DDB0) | _BV(DDB1) | _BV(DDB2));
    }
}

static inline bool SPI_data_ready(void)
{
    return bit_is_set(SPSR, SPIF);
}

static inline uint16_t SPI_read_byte(void)
{
    loop_until_bit_is_set(SPSR, SPIF);
    return SPDR;
}

static inline void SPI_write_byte(uint8_t c)
{
    SPDR = c;
    loop_until_bit_is_set(SPSR, SPIF);
}

#endif /* !SPI_included */
