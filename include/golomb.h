#ifndef __GOLOMB_H__
#define __GOLOMB_H__
#include "main.h"
void golomb_rice_encode(uint16_t val, uint8_t *dst, uint32_t *pos, uint8_t b);
uint16_t golomb_rice_decode(uint8_t *src, uint32_t *pos, uint8_t b);
#endif