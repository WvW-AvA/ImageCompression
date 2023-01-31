#include "prediction.h"
#include "string.h"
#include "stdlib.h"
#include "log.h"
#define SET_BIT(D, P) (*(uint8_t *)((D) + ((P) / 8))) |= (0x80 >> ((P) % 8))
#define GET_BIT(D, P) (*(uint8_t *)((D) + ((P) / 8))) & (0x80 >> ((P) % 8))
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
__always_inline void my_color_minus(color *data, uint8_t *sign_flag, color *c1, color *c2, uint32_t d)
{
    COMPARE_MINUS(c1->R, c2->R, sign_flag, d * 4);
    COMPARE_MINUS(c1->G, c2->G, sign_flag, d * 4 + 1);
    COMPARE_MINUS(c1->B, c2->B, sign_flag, d * 4 + 2);
    COMPARE_MINUS(c1->A, c2->A, sign_flag, d * 4 + 3);
}
__always_inline void my_color_add(color *data, uint8_t *sign_flag, color *c1, color *c2, uint32_t d)
{
    COMPARE_ADD(c1->R, c2->R, sign_flag, d * 4);
    COMPARE_ADD(c1->G, c2->G, sign_flag, d * 4 + 1);
    COMPARE_ADD(c1->B, c2->B, sign_flag, d * 4 + 2);
    COMPARE_ADD(c1->A, c2->A, sign_flag, d * 4 + 3);
}

void cloumn_differ_predict(prediction *predict, image *target)
{
    predict->first_pixiv = target->data[0];
    for (int i = target->hight - 1; i >= 0; i--)
        for (int j = target->width - 1; j > 0; j--)
        {
            uint32_t d = target->width * i + j;
            my_color_minus(target->data, predict->sign_flag, target->data + d, target->data + d - 1, d);
        }
    for (int i = target->hight - 1; i > 0; i--)
    {
        uint32_t d = target->width * i;
        my_color_minus(target->data, predict->sign_flag, target->data + d, target->data + d - target->width, d);
    }
    *(uint32_t *)(target->data) = 0;
}

void cloumn_differ_recover(prediction *predict, image *target)
{
    target->data[0] = predict->first_pixiv;
    for (int i = 1; i < target->hight; i++)
    {
        uint32_t d = target->width * i;
        my_color_add(target->data, predict->sign_flag, target->data + d, target->data + d - target->width, d);
    }
    for (int i = 0; i < target->hight; i++)
        for (int j = 1; j < target->width; j++)
        {
            uint32_t d = target->width * i + j;
            my_color_add(target->data, predict->sign_flag, target->data + d, target->data + d - 1, d);
        }
}

void recover(prediction *predict, image *target)
{
    if (predict->predict_type == COLUMN_DIFFER_PREDICT)
    {
        cloumn_differ_recover(predict, target);
    }
    LOG("recover done!");
}
void predict(prediction *predict, image *target)
{
    predict->sign_flag = (uint8_t *)malloc(sizeof(uint8_t) * target->width * target->hight / 2 + 1);
    if (predict->predict_type == COLUMN_DIFFER_PREDICT)
    {
        cloumn_differ_predict(predict, target);
    }
    LOG("prediction done!");
}
