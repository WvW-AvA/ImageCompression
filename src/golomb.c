#include "golomb.h"
#include "main.h"
#include "math.h"
#include "log.h"

void eden_change(uint32_t *value)
{
    uint8_t *p = (uint8_t *)value;
    int temp = p[0];
    p[0] = p[3];
    p[3] = temp;
    temp = p[1];
    p[1] = p[2];
    p[2] = temp;
}

void golomb_rice_encode(uint16_t val, uint8_t *dst, uint32_t *index_ptr, uint8_t b)
{
    uint8_t m = 0x01 << b;
    uint8_t q = val / m;
    uint8_t r = val & (m - 1);
    for (int i = 0; i < q; i++)
    {
        RESET_BIT(dst, *index_ptr);
        (*index_ptr)++;
    }
    SET_BIT(dst, *index_ptr);
    (*index_ptr)++;
    for (int i = 0; i < b; i++)
    {
        if (GET_BIT(&r, (8 - b) + i))
            SET_BIT(dst, *index_ptr);
        else
            RESET_BIT(dst, *index_ptr);
        (*index_ptr)++;
    }
}

void golomb_exp_encode(uint32_t val, uint8_t *dst, uint32_t *index_ptr, uint8_t k)
{
    val += (0x01 << k) - 1;
    uint8_t m = 0;
    while ((0x01 << m) <= val + 1)
    {
        m++;
    }
    m--;
    uint32_t offset = val + 1 - (0x01 << m);
    for (int i = k; i < m; i++)
    {
        RESET_BIT(dst, *index_ptr);
        (*index_ptr)++;
    }
    SET_BIT(dst, *index_ptr);
    (*index_ptr)++;

    eden_change(&offset);

    for (int i = 0; i < m; i++)
    {
        if (GET_BIT(&offset, ((32 - m) + i)))
            SET_BIT(dst, *index_ptr);
        else
            RESET_BIT(dst, *index_ptr);
        (*index_ptr)++;
    }
}

uint16_t golomb_rice_decode(uint8_t *src, uint32_t *index_ptr, uint8_t b)
{
    uint16_t ret = 0;
    uint8_t n = 0;
    while (GET_BIT(src, *index_ptr) == 0)
    {
        n++;
        (*index_ptr)++;
    }
    ret += n * (0x01 << b);
    uint8_t tem = 0;
    (*index_ptr)++;
    for (int i = 0; i < b; i++)
    {
        if (GET_BIT(src, *index_ptr))
            SET_BIT(&tem, (8 - b) + i);
        else
            RESET_BIT(&tem, (8 - b) + i);
        (*index_ptr)++;
    }
    ret += tem;
    return ret;
}

uint32_t golomb_exp_decode(uint8_t *src, uint32_t *index_ptr, uint8_t k)
{
    uint32_t ret = 0;
    uint8_t n = k;
    while (GET_BIT(src, *index_ptr) == 0)
    {
        n++;
        (*index_ptr)++;
    }
    ret += (0x01 << n);
    uint32_t offset = 0;
    (*index_ptr)++;

    for (int i = 0; i < n; i++)
    {
        if (GET_BIT(src, *index_ptr))
            SET_BIT(&offset, ((32 - n) + i));
        else
            RESET_BIT(&offset, ((32 - n) + i));
        (*index_ptr)++;
    }
    eden_change(&offset);
    ret += offset - (0x01 << k);
    return ret;
}