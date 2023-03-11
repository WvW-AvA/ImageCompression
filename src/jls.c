#include "jls.h"
#include "prediction.h"
#include "string.h"
#include "stdlib.h"
#include "golomb.h"
// 量化边界
int quatization_edge[4];
// 当前channel的编码模式
int channel_mode[4];
// 预测误差绝对值之和
int contex_arg_a[4][365];
// 预测误差重建值之和
int contex_arg_b[4][365];
// 预测误差矫正值
int contex_arg_c[4][365];
// 上下文出现次数
int contex_arg_n[4][365];

uint32_t run_length_count[4];
uint8_t run_length_referance[4];
void write_value(uint32_t val, jls *jls)
{
    golomb_exp_encode(val, jls->data_segment, &(jls->data_size), jls->golomb_exp_k);
}

void run_length_encode(jls *jls, uint8_t channel, uint8_t x, uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    if (x == run_length_referance[channel])
        run_length_count[channel]++;
    else
    {
        write_value(run_length_count[channel], jls);
        channel_mode[channel] = NORMAL_MODE;
        normal_encode(jls, x, a, b, c, d, channel);
    }
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

void update_context_parameter(int err, int q, jls *jls, int *a, int *b, int *c, int *n)
{
    // update context parameter
    a[q] += ABS(err);
    b[q] += err * (2 * jls->near + 1);
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

void normal_encode(jls *jls, uint8_t x, uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t channel)
{
    int d0 = d - b;
    int d1 = b - c;
    int d2 = c - a;
    // caculate differ
    if (d0 == 0 && d1 == 0 && d2 == 0 && x == a)
    {
        // flat area
        channel_mode[channel] = RUN_LENGTH_MODE;
        write_value(RUN_LENGTH_MODE_FLAG, jls);
        write_value(x, jls);
        run_length_count[channel] = 0;
        run_length_referance[channel] = x;
        run_length_encode(jls, channel, x, a, b, c, d);
    }
    // quantization
    int q0 = gradient_quantization(d0);
    int q1 = gradient_quantization(d1);
    int q2 = gradient_quantization(d2);
    int sign = SIGN(q0);
    q0 *= sign;
    q1 *= sign;
    q2 *= sign;
    int q = ((q0 * 9 + q1) * 9 + q2);

    uint8_t pred;
    if (c > MAX(a, b))
        pred = MIN(a, b);
    else if (c < MIN(a, b))
        pred = MAX(a, b);
    else
        pred = a + b - c;

    pred = CLAMP(0, pred + sign * contex_arg_c[channel][q], 255);

    int err = sign * (x - pred);
    update_context_parameter(err, q, jls, contex_arg_a[channel], contex_arg_b[channel], contex_arg_c[channel], contex_arg_n[channel]);
    uint32_t m_err = err >= 0 ? (2 * err) : (-2 * err - 1);
    write_value(m_err, jls);
}

void run_channel_encode(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t x, uint8_t channel, jls *jls)
{
    if (channel_mode[channel] == RUN_LENGTH_MODE)
    {
        run_length_encode(jls, channel, x, a, b, c, d);
    }
    else if (channel_mode[channel] == NORMAL_MODE)
    {
        normal_encode(jls, x, a, b, c, d, channel);
    }
}

void jls_encode_magnetic_head(image *img, uint32_t x, uint32_t y, jls *jls, uint8_t channel)
{
    // get context
    uint8_t *a, *b, *c, *d, *v;
    v = (uint8_t *)(get_pixiv(img, x, y));
    if (y != 0)
    {
        b = (uint8_t *)(get_pixiv(img, x, y - 1));
        if (x != 0)
        {
            if (x != img->width)
                d = (uint8_t *)(get_pixiv(img, x + 1, y - 1));
            else
                d = b;
            a = (uint8_t *)(get_pixiv(img, x - 1, y));
            c = (uint8_t *)(get_pixiv(img, x - 1, y - 1));
        }
        else
        {
            a = b;
            if (y <= 1)
                *((uint32_t *)c) = 0;
            else
                c = (uint8_t *)(get_pixiv(img, x, y - 2));
        }
    }
    run_channel_encode(a + channel, b + channel, c + channel, d + channel, v + channel, channel, jls);
}

jls jls_encoder_init(image *img, uint8_t scan_mode)
{
    jls jls;
    jls.data_segment = (uint8_t *)malloc(sizeof(color) * img->width * img->hight);
    jls.data_size = 0;
    jls.height = img->hight;
    jls.width = img->width;
    jls.channel_count = 4;
    jls.golomb_exp_k = 0;
    jls.near = 0;
    jls.scan_mode = scan_mode;
    quatization_edge[1] = (8 - 7) * 3 + jls.near;
    quatization_edge[2] = (8 - 7) * 7 + 2 * jls.near;
    quatization_edge[3] = (8 - 7) * 21 + 3 * jls.near;
    return jls;
}

void line_scan(image *img, jls *jls)
{
    for (int channel = 0; channel < jls->channel_count; channel++)
    {
        for (int y = 0; y < img->hight; y++)
            for (int x = 0; x < img->width; x++)
                jls_encode_magnetic_head(img, x, y, jls, channel);
        write_value(CHANNEL_END, jls);
    }
}
void jls_encode(image *img, jls *jls)
{
    if (jls->scan_mode == LINE_SCAN)
    {
        line_scan(img, jls);
    }
}

image jls_decode(jls *jls)
{
}