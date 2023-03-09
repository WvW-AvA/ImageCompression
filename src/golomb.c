#include "golomb.h"
#include "main.h"
#include "math.h"
#include "log.h"
void golomb_rice_encode(uint16_t val, uint8_t *dst, uint32_t *pos, uint8_t b)
{
    uint8_t m = 0x01 << b;
    uint8_t q = val / m;
    uint8_t r = val & (m - 1);
    for (int i = 0; i < q; i++)
    {
        RESET_BIT(dst, *pos);
        (*pos)++;
    }
    SET_BIT(dst, *pos);
    (*pos)++;
    for (int i = 0; i < b; i++)
    {
        if (GET_BIT(&r, (8 - b) + i))
            SET_BIT(dst, *pos);
        else
            RESET_BIT(dst, *pos);
        (*pos)++;
    }
}

uint16_t golomb_rice_decode(uint8_t *src, uint32_t *pos, uint8_t b)
{
    uint16_t ret = 0;
    uint8_t n = 0;
    while (GET_BIT(src, *pos) == 0)
    {
        n++;
        (*pos)++;
    }
    ret += n * (0x01 << b);
    uint8_t tem = 0;
    (*pos)++;
    for (int i = 0; i < b; i++)
    {
        if (GET_BIT(src, *pos))
            SET_BIT(&tem, (8 - b) + i);
        else
            RESET_BIT(&tem, (8 - b) + i);
        (*pos)++;
    }
    ret += tem;
    return ret;
}