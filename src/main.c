#include "log.h"
#include "stdlib.h"
#include "bmp.h"
#include "test.h"
#include "main.h"

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
image new_image(bmp *bmp)
{
    image ret;
    ret.data = bmp->data;
    ret.hight = bmp->info->bi_hight;
    ret.width = bmp->info->bi_width;
    return ret;
}

int main(int argc, char **argv)
{
    // huffman_encode_test();
    // huffman_decode_test();
    // prediction_test();
    // recover_test();
}
