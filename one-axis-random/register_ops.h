#ifndef REGISTER_OPS_INCLUDED
#define REGISTER_OPS_INCLUDED

// Register Bit Manipulation
#define SBI(reg, bit)     ((reg) |= 1 << (bit))
#define CBI(reg, bit)     ((reg) &= ~(1 << (bit)))

#define MASK2(b1, b0)     (1 << (b1) | 1 << (b0))
#define MASK3(b2, b1, b0) (1 << (b2) | 1 << (b1) | 1 << (b0))
#define MIN2(b1, b0)      ((b1) < (b0) ? (b1) : (b0))
#define MIN3(b2, b1, b0)  ((b2) < (b1) ? MIN2((b2), (b0)) : MIN2((b1), (b0)))

#define SET_1_BIT(reg, bit, value) ((value) ?                           \
                                   SBI((reg), (bit)) :                  \
                                   CBI((reg), (bit)))

#define SET_2_BITS(reg, bit1, bit0, value)                              \
    ((reg) = ((reg) & ~MASK2((bit1), (bit0))) |                         \
             (value) << MIN2((bit1), (bit0)))

#define SET_3_BITS(reg, bit2, bit1, bit0, value)                        \
    ((reg) = ((reg) & ~MASK3((bit2), (bit1), (bit0))) |                 \
             (value) << MIN3((bit2), (bit1), (bit0)))

#endif /* !REGISTER_OPS_INCLUDED */
