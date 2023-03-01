#include <stdlib.h>
#include <string.h>
#include "lz77.h"
#include "log.h"

void update_kmp_next_array(uint8_t *next, uint8_t *data, uint8_t size)
{
    next[0] = 0;
    uint8_t k = 0;
    for (int i = 1; i < size; i++)
    {
        while (k > 0 && data[k] != data[i])
            k = next[k - 1];
        if (data[k] == data[i])
            k++;
        next[i] = k;
    }
}

uint32_t lz77_encode(uint8_t *data, uint32_t size)
{
    uint8_t *buffer = (uint8_t *)calloc(size, sizeof(uint8_t));
    uint32_t buffer_ptr = 0;
    uint8_t next[256] = {0};
    uint32_t d_ptr = 0, b_ptr = 0;
    while (d_ptr < size)
    {
        uint16_t lookhead_size = MIN(size - d_ptr, 255);
        update_kmp_next_array(next, data + d_ptr, lookhead_size);
        uint8_t offset = 0;
        uint8_t match_size = 0;
        uint8_t last_symbol = 0;
        uint32_t l_ptr = d_ptr - (MIN(d_ptr, 255));
        uint16_t i = 0, j = 0;
        while (l_ptr + i < d_ptr && j < lookhead_size)
        {
            if (data[l_ptr + i + j] == data[d_ptr + j])
            {
                j++;
            }
            else
            {
                if (j > match_size)
                {
                    offset = d_ptr - (l_ptr + i);
                    match_size = j;
                }
                if (j != 0)
                {
                    i += j - next[j - 1];
                    j = next[j - 1];
                }
                else
                {
                    i++;
                }
            }
        }
        if (j > match_size && l_ptr + i < d_ptr)
        {
            offset = d_ptr - (l_ptr + i);
            match_size = j;
        }
        last_symbol = data[d_ptr + match_size];
        d_ptr += match_size + 1;
        LOG("%d:(%d , %d , %d)", d_ptr, offset, match_size, last_symbol);

        buffer[buffer_ptr] = offset;
        buffer[buffer_ptr + 1] = match_size;
        buffer[buffer_ptr + 2] = last_symbol;
        buffer_ptr += 3;
    }
    if (buffer_ptr > size)
    {
        LOG_WARNING("lz77 encode output data is bigger than srouce data,\nit's not recommad to use lz77");
        return 0x3f3f3f3f;
    }
    else
    {
        memcpy(data, buffer, buffer_ptr);
        free(buffer);
        LOG("lz77 encode done");
        return buffer_ptr;
    }
}

uint8_t *lz77_decode(uint8_t *data, uint32_t size, uint32_t origin_size)
{

    uint8_t *buffer = (uint8_t *)malloc(origin_size * 2);
    uint32_t lz_ptr = 0, buffer_ptr = 0;
    for (; lz_ptr < size; lz_ptr += 3)
    {
        for (int i = 0; i < data[lz_ptr + 1]; i++)
        {
            buffer[buffer_ptr] = buffer[buffer_ptr - data[lz_ptr]];
            buffer_ptr++;
        }
        buffer[buffer_ptr] = data[lz_ptr + 2];
        buffer_ptr++;
    }
    if (buffer_ptr != origin_size + 1)
    {
        LOG_ERROR("lz77 decode error, may data is corruption");
    }
    LOG("lz77 decode done decode size %d", origin_size);
    return buffer;
}