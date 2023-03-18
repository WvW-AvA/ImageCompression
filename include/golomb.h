#ifndef __GOLOMB_H__
#define __GOLOMB_H__
#include "main.h"
void golomb_rice_encode(uint16_t val, uint8_t *dst, uint32_t *index_ptr, uint8_t b);
uint16_t golomb_rice_decode(uint8_t *src, uint32_t *index_ptr, uint8_t b);
void golomb_exp_encode(uint32_t val, uint8_t *dst, uint32_t *index_ptr, uint8_t k);
uint32_t golomb_exp_decode(uint8_t *src, uint32_t *index_ptr, uint8_t k);

#endif