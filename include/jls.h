#ifndef __JLS_H__
#define __JLS_H__
#include "main.h"
#define CHANNEL_R 0
#define CHANNEL_G 1
#define CHANNEL_B 2
#define CHANNEL_A 3
typedef struct jls_struct
{
    uint8_t *raw;
} jls;

typedef struct jls_config
{
    uint8_t bit_per_pixiv;
    uint32_t near;
} jls_config;

jls jls_encode(image *img);
image jls_decode(jls *jls);
#endif