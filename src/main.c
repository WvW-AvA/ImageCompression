#include "log.h"
#include "main.h"
#include "stdlib.h"
#include "bmp.h"
#include "test.h"

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

int main(int argc, char **argv)
{
    //   huffman_encode_test();
    huffman_decode_test();
}
