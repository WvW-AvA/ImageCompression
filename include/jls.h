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
#define CHANNEL_BEGIN 601

typedef struct jls_struct
{
    uint32_t file_size;
    uint32_t width;
    uint32_t height;
    uint32_t data_size;
    uint8_t channel_count;
    uint8_t near;
    uint8_t golomb_exp_k;
    uint8_t scan_mode;
    uint32_t curr_index;
    uint8_t *data_segment;
} jls;

typedef void (*magnetic_func_ptr)(image *img, uint32_t x, uint32_t y, jls *jls);

jls jls_init(image *img, uint8_t scan_mode);
jls jls_load(const char *path);
void jls_save(jls *jls, const char *save_path);
void jls_free(jls *jls);
void jls_encode(image *img, jls *jls);
image jls_decode(jls *jls);
#endif