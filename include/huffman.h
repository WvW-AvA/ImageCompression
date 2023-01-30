#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__
#include "main.h"

typedef struct
{
    uint8_t *raw;
    uint32_t f_size;

    uint16_t symbol_num;      // the number of the symbols
    uint32_t origin_size;     // the size of the origin data [byte]
    uint32_t data_size;       // the size of the data [byte]
    uint8_t last_byte_record; // because of the last word of data is not full of valid data,
                              // we must record the end of the valid data index in last word.
    uint8_t *symbol;          // the pointer of the symbols list
    uint32_t *symbol_freq;    // the pointer of the symbols frequency list
    uint8_t *data;            // the pointer of the encoded value
} huffman_encode_handle;

typedef struct
{
    uint8_t *data;
    uint32_t size;
} huffman_decode_handle;

typedef struct
{
    uint8_t value;
    uint32_t frequency;
    struct huffman_node *point;
} huffman_statstic_struct;

typedef struct huffman_node
{
    struct huffman_node *son0;
    struct huffman_node *son1;
    struct huffman_node *before;
    uint32_t frequency;
    uint8_t value;
    uint8_t code;
} huffman_node;

huffman_encode_handle huffman_encode(huffman_decode_handle src);
huffman_decode_handle huffman_decode(huffman_encode_handle src);
void huffman_save(huffman_encode_handle src, const char *path);
huffman_encode_handle huffman_load(const char *path);

#endif