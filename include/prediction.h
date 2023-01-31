#ifndef __PREDICTION_H__
#define __PREDICTION_H__
#include "main.h"
#define COLUMN_DIFFER_PREDICT 0
#define ROW_DIFFER_PREDICT 1
#define MIX_DIFFER_PREDICT 2
typedef struct
{
    uint8_t predict_type;
    color first_pixiv;
    uint8_t *sign_flag;
} prediction;

void predict(prediction *predict, image *target);
void recover(prediction *predict, image *target);
#endif