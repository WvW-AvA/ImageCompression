#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "jls.h"
#include "prediction.h"
#include "golomb.h"
#include "log.h"
// 统计
int encode_statistc[610];
int run_length_statistc;
int run_total_length;

// 量化边界
int quatization_edge[4];
// 当前channel的编码模式
uint8_t channel_mode[4];
// 预测误差绝对值之和
int contex_arg_a[4][365];
// 预测误差重建值之和
int contex_arg_b[4][365];
// 预测误差矫正值
int contex_arg_c[4][365];
// 上下文出现次数
int contex_arg_n[4][365];

uint32_t run_length_count[4];
uint8_t run_length_reference[4];

uint8_t channel = 0;
void global_variable_reset()
{
    memset(quatization_edge, 0, 4);
    memset(channel_mode, 0, 4);
    memset(run_length_count, 0, 4);
    memset(run_length_reference, 0, 4);
    memset(contex_arg_a, 0, 4 * 365 * sizeof(int));
    memset(contex_arg_b, 0, 4 * 365 * sizeof(int));
    memset(contex_arg_c, 0, 4 * 365 * sizeof(int));
    memset(contex_arg_n, 0, 4 * 365 * sizeof(int));
}
void normal_encode(jls *jls, uint8_t x, uint8_t a, uint8_t b, uint8_t c, uint8_t d);

void write_value(uint32_t val, jls *jls)
{
    golomb_exp_encode(val, jls->data_segment, &(jls->curr_index), jls->golomb_exp_k);
    // printf("%d  ", val);
}

uint32_t read_value(jls *jls)
{
    uint32_t ret = golomb_exp_decode(jls->data_segment, &(jls->curr_index), jls->golomb_exp_k);
    // printf("%d  ", ret);
    return ret;
}

void run_length_encode(jls *jls, uint8_t v, uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    if (v == run_length_reference[channel])
        run_length_count[channel]++;
    else
    {
        run_length_statistc++;
        run_total_length += run_length_count[channel];

        write_value(run_length_count[channel], jls);
        channel_mode[channel] = NORMAL_MODE;
        normal_encode(jls, v, a, b, c, d);
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

void normal_encode(jls *jls, uint8_t v, uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    int d0 = d - b;
    int d1 = b - c;
    int d2 = c - a;
    // caculate differ
    if (d0 == 0 && d1 == 0 && d2 == 0)
    {
        // flat area
        channel_mode[channel] = RUN_LENGTH_MODE;
        write_value(RUN_LENGTH_MODE_FLAG, jls);
        write_value(v, jls);
        run_length_count[channel] = 1;
        run_length_reference[channel] = v;
        return;
    }
    // quantization
    int q0 = gradient_quantization(d0);
    int q1 = gradient_quantization(d1);
    int q2 = gradient_quantization(d2);
    int sign = SIGN(q0 ? q0 : q1 ? q1
                                 : q2);
    q0 *= sign;
    q1 *= sign;
    q2 *= sign;
    int q = ((q0 * 9 + q1) * 9 + q2);

    int pred;
    if (c > MAX(a, b))
        pred = MIN(a, b);
    else if (c < MIN(a, b))
        pred = MAX(a, b);
    else
        pred = a + b - c;

    pred = pred + sign * contex_arg_c[channel][q];

    int err = sign * (v - pred);
    update_context_parameter(err, q, jls, contex_arg_a[channel], contex_arg_b[channel], contex_arg_c[channel], contex_arg_n[channel]);
    uint32_t m_err = err >= 0 ? (2 * err) : (-2 * err - 1);
    write_value(m_err, jls);
    encode_statistc[m_err]++;
    // LOG("[q0:%d,q1:%d,q2:%d] sign:%d q:%d pred:%d Normal Eecode:%d\n", q0, q1, q2, sign, q, pred, m_err);
}

void normal_decode(jls *jls, uint32_t e_err, uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t *target_pix)
{
    int err = 0;
    if ((e_err % 2) == 1)
        err = -((int)e_err + 1) / 2;
    else
        err = ((int)e_err) / 2;
    int d0 = d - b;
    int d1 = b - c;
    int d2 = c - a;
    int q0 = gradient_quantization(d0);
    int q1 = gradient_quantization(d1);
    int q2 = gradient_quantization(d2);
    int sign = SIGN(q0 ? q0 : q1 ? q1
                                 : q2);
    q0 *= sign;
    q1 *= sign;
    q2 *= sign;
    int q = ((q0 * 9 + q1) * 9 + q2);

    int pred;
    if (c > MAX(a, b))
        pred = MIN(a, b);
    else if (c < MIN(a, b))
        pred = MAX(a, b);
    else
        pred = a + b - c;

    pred = pred + sign * contex_arg_c[channel][q];
    update_context_parameter(err, q, jls, contex_arg_a[channel], contex_arg_b[channel], contex_arg_c[channel], contex_arg_n[channel]);
    (*target_pix) = err * sign + pred;
    // LOG("[q0:%d,q1:%d,q2:%d] q:%d e_err:%d err:%d pred:%d Normal decode:%d\n",
    //   q0, q1, q2, q, e_err, err, pred, *target_pix);
}

__always_inline void get_contex(image *img, uint32_t x, uint32_t y, uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d, uint8_t *v)
{
    *v = *((uint8_t *)get_pixiv(img, x, y) + channel);
    if (y != 0)
    {
        *b = *(((uint8_t *)get_pixiv(img, x, y - 1)) + channel);
        if (x != 0)
        {
            *c = *(((uint8_t *)get_pixiv(img, x - 1, y - 1)) + channel);
        }
        else
        {
            *a = *b;
            if (y > 1)
                *c = *(((uint8_t *)get_pixiv(img, x, y - 2)) + channel);
        }

        if (x != img->width - 1)
            *d = *(((uint8_t *)get_pixiv(img, x + 1, y - 1)) + channel);
        else
            *d = *b;
    }
    if (x != 0)
        *a = *(((uint8_t *)get_pixiv(img, x - 1, y)) + channel);
    // LOG("get contex (%d,%d) [%d,%d,%d,%d,%d]  mode:%d", x, y, *a, *b, *c, *d, *v, channel_mode[channel]);
}

void jls_encode_magnetic_head(image *img, uint32_t x, uint32_t y, jls *jls)
{
    // get context
    uint8_t zero[5] = {0};
    uint8_t *a = zero, *b = zero + 1, *c = zero + 2, *d = zero + 3, *v = zero + 4;
    get_contex(img, x, y, a, b, c, d, v);
    if (channel_mode[channel] == RUN_LENGTH_MODE)
    {
        run_length_encode(jls, *v, *a, *b, *c, *d);
    }
    else if (channel_mode[channel] == NORMAL_MODE)
    {
        normal_encode(jls, *v, *a, *b, *c, *d);
    }
}

void jls_decode_magnetic_head(image *img, uint32_t x, uint32_t y, jls *jls)
{
    uint8_t *set_target = ((uint8_t *)get_pixiv(img, x, y)) + channel;
    if (channel_mode[channel] == RUN_LENGTH_MODE)
    {
        *set_target = run_length_reference[channel];
        // printf("set:%d  ", *set_target);
        run_length_count[channel] -= 1;
        if (run_length_count[channel] == 0)
            channel_mode[channel] = NORMAL_MODE;
        return;
    }
    if (channel_mode[channel] == NORMAL_MODE)
    {
        uint32_t value = read_value(jls);

        if (value == RUN_LENGTH_MODE_FLAG)
        {
            // begin run-length code
            channel_mode[channel] = RUN_LENGTH_MODE;
            run_length_reference[channel] = read_value(jls);
            run_length_count[channel] = read_value(jls) - 1;
            *set_target = run_length_reference[channel];
            // printf("set:%d  ", *set_target);
            if (run_length_count[channel] == 0)
                channel_mode[channel] = NORMAL_MODE;
            return;
        }
        // get context
        uint8_t zero[5] = {0};
        uint8_t *a = zero, *b = zero + 1, *c = zero + 2, *d = zero + 3, *v = zero + 4;
        get_contex(img, x, y, a, b, c, d, v);
        normal_decode(jls, value, *a, *b, *c, *d, set_target);
    }
}

jls jls_init(image *img, uint8_t scan_mode)
{
    global_variable_reset();
    jls jls;
    jls.data_segment = (uint8_t *)malloc(sizeof(color) * img->width * img->hight * 2);
    jls.curr_index = 0;
    jls.data_size = 0;
    jls.height = img->hight;
    jls.width = img->width;
    jls.channel_count = 4;
    jls.golomb_exp_k = 1;
    jls.near = 0;
    jls.scan_mode = scan_mode;
    quatization_edge[1] = (8 - 7) * 3 + jls.near;
    quatization_edge[2] = (8 - 7) * 7 + 2 * jls.near;
    quatization_edge[3] = (8 - 7) * 21 + 3 * jls.near;
    LOG("jls init done img_size:%dx%d scan_mode %d", jls.width, jls.height, jls.scan_mode);
    return jls;
}

void set_channel(int value, jls *jls, uint8_t isWrite)
{
    if (channel == value - 1 && channel_mode[channel] == RUN_LENGTH_MODE)
    {
        if (isWrite)
            write_value(run_length_count[channel], jls);
        channel_mode[channel] = NORMAL_MODE;
    }
    channel = value;
    LOG("Channel %d scan begin\n", value);
}
void line_scan(image *img, jls *jls, magnetic_func_ptr magnetic_function)
{
    for (int y = 0; y < img->hight; y++)
        for (int x = 0; x < img->width; x++)
        {
            // printf("(%d,%d)", x, y);
            magnetic_function(img, x, y, jls);
        }
}

void jls_encode(image *img, jls *jls)
{

    int c;
    for (c = 0; c < jls->channel_count; c++)
    {
        set_channel(c, jls, 1);
        if (jls->scan_mode == LINE_SCAN)
        {
            line_scan(img, jls, jls_encode_magnetic_head);
        }
    }
    set_channel(c, jls, 1);
    LOG("jls encode done");
    for (int i = 0; i < 610; i++)
        LOG("%d:%d", i, encode_statistc[i]);
    LOG("run length num:%d  run count:%d", run_length_statistc, run_total_length);
}

image jls_decode(jls *jls)
{
    image img = new_image(jls->width, jls->height);
    int c;
    for (c = 0; c < jls->channel_count; c++)
    {
        set_channel(c, jls, 0);
        if (jls->scan_mode == LINE_SCAN)
        {
            line_scan(&img, jls, jls_decode_magnetic_head);
        }
    }
    set_channel(c, jls, 0);
    LOG("jls decode done");
    return img;
}

void jls_save(jls *jls, const char *save_path)
{
    FILE *f = fopen(save_path, "w");
    jls->data_size = (jls->curr_index / 8) + 1;
    jls->file_size = sizeof(uint32_t) * 4 + sizeof(uint8_t) * 4 + jls->data_size;
    LOG("jls save path:%s file size:%d segment size %d", save_path, jls->file_size, jls->data_size);
    fwrite(jls, sizeof(uint32_t) * 4 + sizeof(uint8_t) * 4, 1, f);
    fwrite(jls->data_segment, jls->data_size, 1, f);
    fclose(f);
}

jls jls_load(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        LOG_ERROR("jls path %s not found", path);
    }
    global_variable_reset();
    jls ret;
    fread(&ret, sizeof(uint32_t) * 4 + sizeof(uint8_t) * 4, 1, f);
    ret.data_segment = (uint8_t *)malloc(sizeof(uint8_t) * ret.data_size);
    fread(ret.data_segment, ret.data_size, 1, f);
    ret.curr_index = 0;
    fclose(f);
    LOG("Load myjls file from %s file size %d channel count %d golomb k %d scan mode %d", path, ret.file_size, ret.channel_count, ret.golomb_exp_k, ret.scan_mode);
    return ret;
}

void jls_free(jls *jls)
{
    LOG("jls free");
    if (jls->data_segment != NULL)
        free(jls->data_segment);
}