#ifndef __MAIN_H__
#define __MAIN_H__

#define SET_BIT(D, P) ((*((uint8_t *)(D) + ((P) / 8))) |= (0x80 >> ((P) % 8)))
#define RESET_BIT(D, P) ((*((uint8_t *)(D) + ((P) / 8))) &= (~(0x80 >> ((P) % 8))))
#define GET_BIT(D, P) ((*((uint8_t *)(D) + ((P) / 8))) & (0x80 >> ((P) % 8)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? (-(a)) : (a))
#define SIGN(a) ((a) < 0 ? (-1) : (1))
#define CLAMP(min, a, max) ((a) < (max) ? MAX(a, min) : (max))
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

struct bmp;
typedef struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t A;
} color;

typedef struct image
{
    uint32_t width;
    uint32_t hight;
    color *data;
} image;

typedef struct
{
    uint8_t *raw;
    uint32_t file_size;
    uint32_t image_width;
    uint32_t image_hight;
    struct prediction *predict;
    struct huffman_encode_handle *huffman;
} file_struct;

char *print_bit(uint8_t *src, uint32_t num);
color *get_pixiv(image *target, uint32_t x, uint32_t y);
void print_color(color c);
int is_color_equal(color *c1, color *c2);
color color_add(color c1, color c2);
color color_minus(color c1, color c2);
image new_image_from_bmp(struct bmp *bmp);

image new_image(uint32_t w, uint32_t h);
#endif