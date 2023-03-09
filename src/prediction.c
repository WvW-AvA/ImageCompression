#include "prediction.h"
#include "string.h"
#include "stdlib.h"
#include "log.h"
#include "main.h"
#define COMPARE_MINUS(A, B, D, P) \
    if ((A) > (B))                \
        (A) = (A) - (B);          \
    else                          \
    {                             \
        (A) = (B) - (A);          \
        SET_BIT(D, P);            \
    }

#define COMPARE_ADD(A, B, D, P) \
    if (GET_BIT(D, P))          \
        (A) = (B) - (A);        \
    else                        \
        (A) = (B) + (A)
__always_inline void my_color_minus(uint8_t *sign_flag, color *c1, color *c2, uint32_t d)
{
    COMPARE_MINUS(c1->R, c2->R, sign_flag, d * 4);
    COMPARE_MINUS(c1->G, c2->G, sign_flag, d * 4 + 1);
    COMPARE_MINUS(c1->B, c2->B, sign_flag, d * 4 + 2);
    COMPARE_MINUS(c1->A, c2->A, sign_flag, d * 4 + 3);
}
__always_inline void my_color_add(uint8_t *sign_flag, color *c1, color *c2, uint32_t d)
{
    COMPARE_ADD(c1->R, c2->R, sign_flag, d * 4);
    COMPARE_ADD(c1->G, c2->G, sign_flag, d * 4 + 1);
    COMPARE_ADD(c1->B, c2->B, sign_flag, d * 4 + 2);
    COMPARE_ADD(c1->A, c2->A, sign_flag, d * 4 + 3);
}
__always_inline uint8_t COLO_I_operator(uint8_t A, uint8_t B, uint8_t C)
{
    if (C >= MAX(A, B))
        return MIN(A, B);
    if (C <= MIN(A, B))
        return MAX(A, B);
    return A - C + B;
}
__always_inline void COLO_I_minus(uint8_t *sign_flag, color *A, color *B, color *C, color *X, uint32_t d)
{
    COMPARE_MINUS(X->R, COLO_I_operator(A->R, B->R, C->R), sign_flag, d * 4);
    COMPARE_MINUS(X->G, COLO_I_operator(A->G, B->G, C->G), sign_flag, d * 4 + 1);
    COMPARE_MINUS(X->B, COLO_I_operator(A->B, B->B, C->B), sign_flag, d * 4 + 2);
    COMPARE_MINUS(X->A, COLO_I_operator(A->A, B->A, C->A), sign_flag, d * 4 + 3);
}
__always_inline void COLO_I_add(uint8_t *sign_flag, color *A, color *B, color *C, color *X, uint32_t d)
{
    COMPARE_ADD(X->R, COLO_I_operator(A->R, B->R, C->R), sign_flag, d * 4);
    COMPARE_ADD(X->G, COLO_I_operator(A->G, B->G, C->G), sign_flag, d * 4 + 1);
    COMPARE_ADD(X->B, COLO_I_operator(A->B, B->B, C->B), sign_flag, d * 4 + 2);
    COMPARE_ADD(X->A, COLO_I_operator(A->A, B->A, C->A), sign_flag, d * 4 + 3);
}
void cloumn_differ_predict(prediction *predict, image *target)
{
    predict->reserve_color[0] = target->data[0];
    for (int i = target->hight - 1; i >= 0; i--)
        for (int j = target->width - 1; j > 0; j--)
        {
            uint32_t d = target->width * i + j;
            my_color_minus(predict->sign_flag, target->data + d, target->data + d - 1, d);
        }
    for (int i = target->hight - 1; i > 0; i--)
    {
        uint32_t d = target->width * i;
        my_color_minus(predict->sign_flag, target->data + d, target->data + d - target->width, d);
    }
    *(uint32_t *)(target->data) = 0;
}

void cloumn_differ_recover(prediction *predict, image *target)
{
    target->data[0] = predict->reserve_color[0];
    for (int i = 1; i < target->hight; i++)
    {
        uint32_t d = target->width * i;
        my_color_add(predict->sign_flag, target->data + d, target->data + d - target->width, d);
    }
    for (int i = 0; i < target->hight; i++)
        for (int j = 1; j < target->width; j++)
        {
            uint32_t d = target->width * i + j;
            my_color_add(predict->sign_flag, target->data + d, target->data + d - 1, d);
        }
}

void LOCO_I_predict(prediction *predict, image *target)
{
    predict->reserve_color[0] = target->data[0];
    for (int i = target->hight - 1; i > 0; i--)
        for (int j = target->width - 1; j > 0; j--)
        {
            uint32_t d = target->width * i + j;
            COLO_I_minus(predict->sign_flag, target->data + d - 1, target->data + d - target->width,
                         target->data + d - target->width - 1, target->data + d, d);
        }
    for (int i = target->hight - 1; i > 0; i--)
    {
        uint32_t d = target->width * i;
        my_color_minus(predict->sign_flag, target->data + d, target->data + d - target->width, d);
    }
    for (int i = target->width - 1; i > 0; i--)
    {
        my_color_minus(predict->sign_flag, target->data + i, target->data + i - 1, i);
    }
}

void LOCO_I_recover(prediction *predict, image *target)
{
    target->data[0] = predict->reserve_color[0];
    for (int i = 0; i < target->width; i++)
    {
        my_color_add(predict->sign_flag, target->data + i, target->data + i - 1, i);
    }
    for (int i = 1; i < target->hight; i++)
    {
        uint32_t d = target->width * i;
        my_color_add(predict->sign_flag, target->data + d, target->data + d - target->width, d);
    }
    for (int i = 1; i < target->hight; i++)
        for (int j = 1; j < target->width; j++)
        {
            uint32_t d = target->width * i + j;
            COLO_I_add(predict->sign_flag, target->data + d - 1, target->data + d - target->width,
                       target->data + d - target->width - 1, target->data + d, d);
        }
}
void recover(prediction *predict, image *target)
{
    if (predict->predict_type == COLUMN_DIFFER_PREDICT)
    {
        cloumn_differ_recover(predict, target);
    }
    else if (predict->predict_type == LOCO_I_PREDICT)
    {
        LOCO_I_recover(predict, target);
    }
    LOG("recover done!");
}
void predict(prediction *predict, image *target)
{
    uint32_t s_size = sizeof(uint8_t) * target->width * target->hight / 2 + 1;
    predict->sign_flag = (uint8_t *)malloc(s_size);
    predict->file_size += s_size;
    if (predict->predict_type == COLUMN_DIFFER_PREDICT)
    {
        cloumn_differ_predict(predict, target);
    }
    else if (predict->predict_type == LOCO_I_PREDICT)
    {
        LOCO_I_predict(predict, target);
    }
    LOG("prediction done!");
}

prediction new_prediction(uint8_t predict_type)
{
    prediction ret;
    ret.predict_type = predict_type;
    if (predict_type == COLUMN_DIFFER_PREDICT || predict_type == LOCO_I_PREDICT)
    {
        ret.reserve_color = (color *)malloc(sizeof(color));
        ret.file_size = 5 + sizeof(color);
    }
    LOG("new prediction done!");
    return ret;
}
void prediction_map(uint8_t *raw, prediction *pred)
{
    pred->file_size = ((uint32_t *)raw)[0];
    pred->predict_type = ((uint8_t *)raw)[4];
    pred->reserve_color = (color *)(raw + 5);
    if (pred->predict_type == COLUMN_DIFFER_PREDICT || pred->predict_type == LOCO_I_PREDICT)
    {
        pred->sign_flag = raw + 5 + sizeof(color) * 1;
    }
}

void prediction_memcpy(uint8_t *dst, prediction *pred)
{
    *((uint32_t *)dst) = pred->file_size;
    *(dst + 4) = pred->predict_type;
    uint32_t r_color_size;
    if (pred->predict_type == COLUMN_DIFFER_PREDICT || pred->predict_type == LOCO_I_PREDICT)
        r_color_size = 1 * 4;
    uint32_t sign_size = pred->file_size - 5 - r_color_size;
    memcpy(dst + 5, pred->reserve_color, r_color_size);
    memcpy(dst + 5 + r_color_size, pred->sign_flag, sign_size);
}