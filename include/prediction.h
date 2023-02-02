#ifndef __PREDICTION_H__
#define __PREDICTION_H__
#include "main.h"
#define COLUMN_DIFFER_PREDICT 0
#define LOCO_I_PREDICT 1
typedef struct prediction
{
    uint32_t file_size;
    uint8_t predict_type;
    color *reserve_color;
    uint8_t *sign_flag;
} prediction;

prediction new_prediction(uint8_t predict_type);
void prediction_map(uint8_t *raw, prediction *pred);
void prediction_memcpy(uint8_t *dst, prediction *pred);
void predict(prediction *predict, image *target);
void recover(prediction *predict, image *target);
#endif