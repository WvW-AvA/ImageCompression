#ifndef __BMP_H__
#define __BMP_H__
#include "main.h"
#define BMP_BI_RGB 0  // wothout compression
#define BMP_BI_RLE8 1 // 8 bit color platte compression
#define BMP_BI_RLE4 2 // 4 bit color platte compression

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    char bf_type[2];         // file type
    uint32_t bf_size;        // file size
    uint16_t bf_reserved1;   // reserved 0,0
    uint16_t bf_reserved2;   // reserved 0,0
    uint32_t bf_offset_bits; // address offset to real pixiv data
} bmp_file_head;
#pragma pack(pop)

typedef struct
{
    uint32_t bi_head_size;       // bmp_info_head need size.
    int bi_width;                // image width
    int bi_hight;                // image hight
    uint16_t bi_planes;          //
    uint16_t bi_bit_count;       // bit count 1 , 4 , 8 , 16 , 24 or 32
    uint32_t bi_compression;     // compression type
    uint32_t bi_image_size;      // image size
    int bi_x_resolution;         // x resolution pixiv per meter
    int bi_y_resolution;         // y resolution pixiv per meter
    uint32_t bi_color_used;      // color used platte color count
    uint32_t bi_color_important; // important color index
} bmp_info_head;

typedef struct
{
    uint8_t *raw;
    bmp_file_head *file;
    bmp_info_head *info;
    color *platte;
    color *data;
} bmp;

bmp *bmp_load(const char *path);
bmp *bmp_new(image *image);
int bmp_save(bmp *src, const char *path);
color *bmp_get(uint32_t x, uint32_t y);
void bmp_print(bmp *src);

/// @brief compare img1 and img2 byte by byte.
/// @param img1
/// @param img2
/// @return 1 means true and 0 means false.
int bmp_compare(bmp *img1, bmp *img2);
#endif