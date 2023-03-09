#include "jls.h"
#include "prediction.h"
#include "string.h"
#include "stdlib.h"
// 量化边界
int quatization_edge[4];
// 预测误差绝对值之和
int contex_arg_a[4][365];
// 预测误差重建值之和
int contex_arg_b[4][365];
// 预测误差矫正值
int contex_arg_c[4][365];
// 上下文出现次数
int contex_arg_n[4][365];

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

void update_context_parameter(int err, int q, jls_config config, int *a, int *b, int *c, int *n)
{
    // update context parameter
    a[q] += ABS(err);
    b[q] += err * (2 * config.near + 1);
    if (n[q] == 0)
    {
        a[q] >>= 1;
        b[q] >>= 1;
        c[q] >>= 1;
    }
    n[q]++;
    if (b[q] < -n[q])
    {
        c[q] -= 1;
        b[q] += n[q];
        if (b[q] < -n[q])
            b[q] = 1 - n[q];
    }
    else if (b > 0)
    {
        c[q] += 1;
        b[q] -= n[q];
        if (b[q] > 0)
            b[q] = 0;
    }
}

void normal_encode(jls_config config, image *img, uint32_t *ptr, uint8_t dst, uint32_t a, uint32_t b, uint32_t c,
                   int d0, int d1, int d2, uint8_t channel)
{
    // quantization
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
        pred += contex_arg_c[channel][q];
    else
        pred -= contex_arg_c[channel][q];
    pred = MAX(0, pred);
    int err = sign * ((*ptr) - pred);
    update_context_parameter(err, q, config, contex_arg_a[channel], contex_arg_b[channel], contex_arg_c[channel], contex_arg_n[channel]);
    int m_err = err >= 0 ? (2 * err) : (-2 * err - 1);
}

void run_channel_encode(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t channel)
{
    // caculate differ
    int d0 = d - b;
    int d1 = b - c;
    int d2 = c - a;
    if (d0 == 0 && d1 == 0 && d2 == 0)
    {
        // select run-length encode mode
        // run_length_encode();
    }
    else
    {
        // select normal encode mode
        //   normal_encode();
    }
}

void jls_encode_magnetic_head(image *img, uint32_t x, uint32_t y, uint8_t *dst)
{
    // get context
    color a, b, c, d;
    if (y != 0)
    {
        b = *(get_pixiv(img, x, y - 1));
        if (x != 0)
        {
            if (x != img->width)
                d = *(get_pixiv(img, x + 1, y - 1));
            else
                d = b;
            a = *(get_pixiv(img, x - 1, y));
            c = *(get_pixiv(img, x - 1, y - 1));
        }
        else
        {
            a = b;
            if (y <= 1)
                *((uint32_t *)&c) = 0;
            else
                c = *(get_pixiv(img, x, y - 2));
        }
    }
    run_channel_encode(a.R, b.R, c.R, d.R, CHANNEL_R);
    run_channel_encode(a.G, b.G, c.G, d.G, CHANNEL_G);
    run_channel_encode(a.B, b.B, c.B, d.B, CHANNEL_B);
    run_channel_encode(a.A, b.A, c.A, d.A, CHANNEL_A);
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