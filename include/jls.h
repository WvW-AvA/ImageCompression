#ifndef __JLS_H__
#define __JLS_H__
#include "main.h"
#define CHANNEL_R 0
#define CHANNEL_G 1
#define CHANNEL_B 2
#define CHANNEL_A 3

#define NORMAL_MODE 0
#define RUN_LENGTH_MODE 1

#define LINE_SCAN 0
#define TILE8_SCAN 1
#define TILE16_SCAN 2

#define RUN_LENGTH_MODE_FLAG 600
#define CHANNEL_END 601

typedef struct jls_struct
{
    uint8_t *raw;
    uint32_t width;
    uint32_t height;
    uint32_t data_size;
    uint8_t channel_count;
    uint8_t near;
    uint8_t golomb_exp_k;
    uint8_t scan_mode;
    uint8_t *data_segment;
} jls;

void jls_encode(image *img, jls *jls);
image jls_decode(jls *jls);
#endif