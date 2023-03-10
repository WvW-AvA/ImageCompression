#include "bmp.h"
#include "main.h"
#include "log.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
bmp *bmp_load(const char *path)
{
    FILE *f = fopen(path, "r");
    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    bmp *img = (bmp *)malloc(sizeof(bmp));
    img->raw = (uint8_t *)malloc(size * sizeof(uint8_t));
    fseek(f, 0, SEEK_SET);
    fread(img->raw, size, 1, f);
    img->file = (bmp_file_head *)img->raw;
    img->info = (bmp_info_head *)(img->raw + 14);
    img->data = (color *)(img->raw + img->file->bf_offset_bits);
    LOG("Load BMP file %s file size:%d", path, size);
    fclose(f);
    return img;
}
int bmp_save(bmp *src, const char *path)
{
    FILE *f = fopen(path, "w");
    fwrite(src->raw, src->file->bf_size, 1, f);
    LOG("Save BMP file %s", path);
    fclose(f);
    return 0;
}
void bmp_print(bmp *src)
{
    LOG("BMP info\nFile size:%d \nResolution: %dx%d\nOffset:%d\nBit count:%d\nCompression:%d",
        src->file->bf_size, src->info->bi_width, src->info->bi_hight, src->file->bf_offset_bits,
        src->info->bi_bit_count, src->info->bi_compression);
}

bmp *bmp_new(image *image)
{
    bmp *res = (bmp *)malloc(sizeof(bmp));
    res->raw = (uint8_t *)malloc(sizeof(color) * image->hight * image->width + 138);
    uint8_t head[138] = {
        0x42, 0x4D, 0x8A, 0x90, 0x7E, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x8A, 0x00, 0x00, 0x00, 0x7C, 0x00,
        0x00, 0x00, 0x80, 0x07, 0x00, 0x00, 0x38, 0x04,
        0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x03, 0x00,
        0x00, 0x00, 0x00, 0x90, 0x7E, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x42, 0x47,
        0x52, 0x73, 0x80, 0xC2, 0xF5, 0x28, 0x60, 0xB8,
        0x1E, 0x15, 0x20, 0x85, 0xEB, 0x01, 0x40, 0x33,
        0x33, 0x13, 0x80, 0x66, 0x66, 0x26, 0x40, 0x66,
        0x66, 0x06, 0xA0, 0x99, 0x99, 0x09, 0x3C, 0x0A,
        0xD7, 0x03, 0x24, 0x5C, 0x8F, 0x32, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00};
    memcpy(res->raw, head, 138);
    res->file = (bmp_file_head *)res->raw;
    res->info = (bmp_info_head *)(res->raw + 14);
    res->data = (color *)(res->raw + res->file->bf_offset_bits);
    res->info->bi_compression = BMP_BI_RGB;
    res->info->bi_hight = image->hight;
    res->info->bi_width = image->width;
    res->info->bi_image_size = image->hight * image->width * sizeof(color);
    res->file->bf_size = res->info->bi_image_size + 138;
    memcpy(res->data, image->data, res->info->bi_image_size);
    return res;
}

int bmp_compare(bmp *img1, bmp *img2)
{
    if (img1->info->bi_image_size != img2->info->bi_image_size)
        return 0;

    for (int i = 0; i < img1->info->bi_image_size / 4; i++)
    {
        if (is_color_equal(img1->data + i, img2->data + i) == 0)
        {
            LOG("Color different in number %d pixiv,total pixiv %d", i, img1->info->bi_image_size / 4);
            print_color(*(img1->data + i));
            print_color(*(img2->data + i));
            return 0;
        }
    }
    return 1;
}