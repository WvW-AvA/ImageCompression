#include "jls.h"
#include "prediction.h"
#include "string.h"
#include "stdlib.h"

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
    uint32_t d0 = d - b;
    uint32_t d1 = b - c;
    uint32_t d2 = c - a;
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

jls jls_encode(image *img)
{
    jls ret;
    ret.raw = (uint8_t *)malloc(sizeof(color) * img->width * img->hight);
}
image jls_decode(jls *jls)
{
}