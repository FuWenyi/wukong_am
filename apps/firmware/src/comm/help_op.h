#ifndef _HELP_OPERATE_H
#define _HELP_OPERATE_H

#include <stdint.h>

# define INT8_WIDTH       8
# define INT16_WIDTH      16
# define INT32_WIDTH      32
# define INT64_WIDTH      64

# define UINT8_WIDTH      8
# define UINT16_WIDTH     16
# define UINT32_WIDTH     32
# define UINT64_WIDTH     64

static void set_bit_dword(volatile uint32_t *val, uint32_t i)
{
    *val |= (1 << i);
}

static void clear_bit_dword(volatile uint32_t *val, uint32_t i)
{
    *val &= ~(1 << i);
}

static int32_t test_bit_dword(volatile uint32_t *val, uint32_t i)
{
    return ((*val) & (1 << i)) >> i;
}

#define ROUND_UP(x, y) (((x)+(y)-1)/(y))
#define MIN(x, y) ((x) < (y)? (x) : (y))
#define MAX(x, y) ((x) > (y)? (x) : (y))

#endif
