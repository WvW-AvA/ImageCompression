#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "bmp.h"
#include "test.h"
#include "main.h"
char str[100];
char *print_bit(uint8_t *src, uint32_t num)
{
    int j = 0;
    for (int i = 0; i < num; i++)
    {
        if (i % 4 == 0)
        {
            str[j] = ' ';
            j++;
        }
        if (GET_BIT(src, i))
            str[j] = '1';
        else
            str[j] = '0';
        j++;
    }
    str[j] = '\0';
    return str;
}
inline color *get_pixiv(image *target, uint32_t x, uint32_t y)
{
    return target->data + (target->width * y + x);
}
inline int is_color_equal(color *c1, color *c2)
{
    return (*(uint32_t *)c1) == (*(uint32_t *)c2);
}
inline void print_color(color c)
{
    LOG("(RGBA):(%d,%d,%d,%d)", c.R, c.G, c.B, c.A);
}
inline color color_add(color c1, color c2)
{
    color c;
    c.R = MIN(c1.R + c2.R, 255);
    c.G = MIN(c1.G + c2.G, 255);
    c.B = MIN(c1.B + c2.B, 255);
    c.A = MIN(c1.A + c2.A, 255);
    return c;
}
inline color color_minus(color c1, color c2)
{
    color c;
    c.R = MAX(c1.R - c2.R, 0);
    c.G = MAX(c1.G - c2.G, 0);
    c.B = MAX(c1.B - c2.B, 0);
    c.A = MAX(c1.A - c2.A, 0);
    return c;
}

image new_image(uint32_t w, uint32_t h)
{
    image ret;
    ret.data = (color *)malloc(w * h * sizeof(color));
    ret.hight = h;
    ret.width = w;
    return ret;
}
image new_image_from_bmp(bmp *bmp)
{
    image ret;
    ret.data = bmp->data;
    ret.hight = bmp->info->bi_hight;
    ret.width = bmp->info->bi_width;
    return ret;
}

void data_save(file_struct *f, const char *path)
{
    uint32_t pix_num = f->image_hight * f->image_hight;
    f->file_size = 12 + f->huffman->file_size + f->predict->file_size;
    f->raw = (uint8_t *)malloc(f->file_size * sizeof(uint8_t));
    ((uint32_t *)(f->raw))[0] = f->file_size;
    ((uint32_t *)(f->raw))[1] = f->image_width;
    ((uint32_t *)(f->raw))[2] = f->image_hight;
    prediction_memcpy(f->raw + 12, f->predict);
    LOG("file_size:%d", f->file_size);
    memcpy(f->raw + 12 + f->predict->file_size, f->huffman->raw, f->huffman->file_size);
    FILE *fi = fopen(path, "w");
    fwrite(f->raw, f->file_size, 1, fi);
    fclose(fi);
    LOG("Compress done!\nSave in %s", path);
}
file_struct data_load(const char *path)
{
    file_struct ret;
    ret.predict = (prediction *)malloc(sizeof(prediction));
    ret.huffman = (huffman_encode_handle *)malloc(sizeof(huffman_encode_handle));

    FILE *f = fopen(path, "r");
    fseek(f, 0L, SEEK_END);
    uint32_t size = ftell(f);
    fseek(f, 0L, SEEK_SET);
    ret.raw = (uint8_t *)malloc(size);
    fread(ret.raw, size, 1, f);
    fclose(f);
    ret.file_size = ((uint32_t *)ret.raw)[0];
    if (ret.file_size != size)
    {
        LOG_ERROR("file %s load error!", path);
    }
    ret.image_width = ((uint32_t *)ret.raw)[1];
    ret.image_hight = ((uint32_t *)ret.raw)[2];
    LOG("11");
    prediction_map(ret.raw + 12, ret.predict);
    ret.huffman->raw = ret.raw + 12 + ret.predict->file_size;
    huffman_map(ret.huffman);
    return ret;
}
void RGBARGBA_2_RRGGBBAA(uint8_t *data, uint32_t size)
{
    uint8_t *temp = (uint8_t *)malloc(size);
    uint32_t pix_num = size / 4;
    for (int i = 0; i < pix_num; i++)
    {
        temp[pix_num * 0 + i] = data[i * 4 + 0];
        temp[pix_num * 1 + i] = data[i * 4 + 1];
        temp[pix_num * 2 + i] = data[i * 4 + 2];
        temp[pix_num * 3 + i] = data[i * 4 + 3];
    }
    memcpy(data, temp, size);
    free(temp);
}
void RRGGBBAA_2_RGBARGBA(uint8_t *data, uint32_t size)
{
    uint8_t *temp = (uint8_t *)malloc(size);
    uint32_t pix_num = size / 4;
    for (int i = 0; i < pix_num; i++)
    {
        temp[i * 4 + 0] = data[pix_num * 0 + i];
        temp[i * 4 + 1] = data[pix_num * 1 + i];
        temp[i * 4 + 2] = data[pix_num * 2 + i];
        temp[i * 4 + 3] = data[pix_num * 3 + i];
    }
    memcpy(data, temp, size);
    free(temp);
}
void encode(const char *image_path, const char *data_path)
{
    file_struct data;
    bmp *bmp = bmp_load(image_path);
    image img = new_image_from_bmp(bmp);
    data.image_width = bmp->info->bi_width;
    data.image_hight = bmp->info->bi_hight;
    // prediction
    prediction pred = new_prediction(LOCO_I_PREDICT);
    predict(&pred, &img);
    RGBARGBA_2_RRGGBBAA((uint8_t *)bmp->data, bmp->info->bi_image_size);
    // lz77
    uint32_t lz_size = 0x3f3f3f3f;
    lz_size = lz77_encode((uint8_t *)bmp->data, bmp->info->bi_image_size);
    // huffman
    huffman_decode_handle src;
    src.data = (uint8_t *)bmp->data;
    src.size = MIN(bmp->info->bi_image_size, lz_size);
    huffman_encode_handle h_encode = huffman_encode(src);
    data.huffman = &h_encode;
    data.predict = &pred;
    data_save(&data, data_path);
    huffman_decode_handle src2;
    src2.data = (uint8_t *)data.raw;
    src2.size = data.file_size;
    huffman_encode_handle h_encode2 = huffman_encode(src2);
    huffman_save(h_encode2, "/home/wu_wa/CICIEC/ImageCompression/atri_lz77_LOCO_I_2.save");
}
void decode(const char *data_path, const char *save_path)
{
    file_struct data = data_load(data_path);
    image img;
    img.width = data.image_width;
    img.hight = data.image_hight;
    huffman_decode_handle decode = huffman_decode(*data.huffman);
    img.data = (color *)lz77_decode(decode.data, decode.size, img.hight * img.width * 4);
    RRGGBBAA_2_RGBARGBA((uint8_t *)img.data, img.width * img.hight * 4);
    recover(data.predict, &img);
    bmp *bmp = bmp_new(&img);
    bmp_save(bmp, save_path);
}

int main(int argc, char **argv)
{
    // huffman_encode_test();
    // huffman_decode_test();
    // prediction_test();
    // recover_test();
    // lz77_test();
    // encode("/home/wu_wa/CICIEC/ImageCompression/atri.bmp", "/home/wu_wa/CICIEC/ImageCompression/atri_lz77_LOCO_I.save");
    // decode("/home/wu_wa/CICIEC/ImageCompression/atri_lz77_LOCO_I.save", "/home/wu_wa/CICIEC/ImageCompression/atri_lz77_LOCO_new.bmp");

    // golomb_rice_test();
    //   new_image_test();
    // jls_encode_test("red.bmp", "red.myjls");
    // jls_decode_test("red.myjls", "red_recover.bmp");
    // bmp_compare_test("red.bmp", "red_recover.bmp");
    jls_encode_test("atri.bmp", "atri.myjls");
    jls_decode_test("atri.myjls", "atri_recover.bmp");
    bmp_compare_test("atri.bmp", "atri_recover.bmp");
}
