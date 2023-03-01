#ifndef __LZ77_H__
#define __LZ77_H__
#include "main.h"

/// @brief LZ77 compress
/// @param data data
/// @param size data size
/// @return the size of compressed data.
uint32_t lz77_encode(uint8_t *data, uint32_t size);

uint8_t *lz77_decode(uint8_t *data, uint32_t size, uint32_t origin_size);
#endif
