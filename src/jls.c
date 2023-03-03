#include "jls.h"
#include "prediction.h"
#include "string.h"
#include "stdlib.h"
// 量化边界
int quatization_edge[4];
// 预测误差绝对值之和
int contex_arg_a[364];
// 预测误差重建值之和
int contex_arg_b[364];
// 预测误差矫正值
int contex_arg_c[364];
// 上下文出现次数
int contex_arg_n[364];

void run_length_encode(image *img, uint32_t *ptr, uint8_t *dst)
{
}

int gradient_quantization(int differ)
{
    int ret = 0;
    int abs_d = ABS(differ);
    for (; ret < 4 && abs_d > quatization_edge[ret]; ret++)
    {
    }
    return SIGN(differ) * ret;
}
void normal_encode(jls_config config, image *img, uint32_t *ptr, uint8_t dst, uint32_t a, uint32_t b, uint32_t c,
                   int d0, int d1, int d2)
{
    int q0 = gradient_quantization(d0);
    int q1 = gradient_quantization(d1);
    int q2 = gradient_quantization(d2);
    int sign = SIGN(q0);
    q0 *= sign;
    q1 *= sign;
    q2 *= sign;
    int q = ((q0 * 9 + q1) * 9 + q2);
    uint32_t pred;
    if (c > MAX(a, b))
        pred = MIN(a, b);
    else if (c < MIN(a, b))
        pred = MAX(a, b);
    else
        pred = a + b - c;
    if (sign == 1)
        pred += contex_arg_c[q];
    else
        pred -= contex_arg_c[q];
    pred = MAX(0, pred);
    int err = sign * ((*ptr) - pred);

    // update context parameter
    contex_arg_a[q] += ABS(err);
    contex_arg_b[q] += err * (2 * config.near + 1);
    if (contex_arg_n[q] == 0)
    {
        contex_arg_a[q] >>= 1;
        contex_arg_b[q] >>= 1;
        contex_arg_c[q] >>= 1;
    }
    contex_arg_n[q]++;
    if (contex_arg_b[q] < -contex_arg_n[q])
    {
        contex_arg_c[q] -= 1;
        contex_arg_b[q] += contex_arg_n[q];
        if (contex_arg_b[q] < -contex_arg_n[q])
            contex_arg_b[q] = 1 - contex_arg_n[q];
    }
    else if (contex_arg_b > 0)
    {
        contex_arg_c[q] += 1;
        contex_arg_b[q] -= contex_arg_n[q];
        if (contex_arg_b[q] > 0)
            contex_arg_b[q] = 0;
    }
}
void jls_encode_magnetic_head(image *img, uint32_t x, uint32_t y, uint8_t *dst)
{
    // context module
    uint32_t a = 0, b = 0, c = 0, d = 0;
    if (y != 0)
    {
        b = *(uint32_t *)(get_pixiv(img, x, y - 1));
        if (x != 0)
        {
            if (x != img->width)
                d = *(uint32_t *)(get_pixiv(img, x + 1, y - 1));
            else
                d = b;
            a = *(uint32_t *)(get_pixiv(img, x - 1, y));
            c = *(uint32_t *)(get_pixiv(img, x - 1, y - 1));
        }
        else
        {
            a = b;
            if (y <= 1)
                c = 0;
            else
                c = *(uint32_t *)(get_pixiv(img, x, y - 2));
        }
    }
    // caculate differ
    int d0 = d - b;
    int d1 = b - c;
    int d2 = c - a;
    if (d0 == 0 && d1 == 0 && d2 == 0)
    {
        // select run-length encode mode
        run_length_encode();
    }
    else
    {
        // select normal encode mode
        normal_encode();
    }
}
void jls_encoder_init(jls_config jls_config)
{
    quatization_edge[1] = (jls_config.bit_per_pixiv - 7) * 3 + jls_config.near;
    quatization_edge[2] = (jls_config.bit_per_pixiv - 7) * 7 + 2 * jls_config.near;
    quatization_edge[3] = (jls_config.bit_per_pixiv - 7) * 21 + 3 * jls_config.near;
}
jls jls_encode(image *img)
{
    jls ret;
    ret.raw = (uint8_t *)malloc(sizeof(color) * img->width * img->hight);
}
image jls_decode(jls *jls)
{
}