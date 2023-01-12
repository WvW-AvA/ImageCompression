#include "log.h"
#include "main.h"
#include "stdlib.h"
#include "bmp.h"

inline color *get_pixiv(image *target, uint32_t x, uint32_t y)
{
    return target->data + (target->width * y + x);
}
int main(int argc, char **argv)
{
    image im;
    im.data = (color *)malloc(sizeof(color) * 512 * 512);
    im.width = 512;
    im.hight = 512;
    for (int i = 0; i < 512; i++)
    {
        for (int j = 0; j < 512; j++)
        {
            color *tem = get_pixiv(&im, i, j);
            tem->A = i * 0.5;
            tem->R = i * 0.5;
            tem->G = j * 0.5;
            tem->B = j * 0.5;
        }
    }
    bmp *img = bmp_new(&im);
    bmp_save(img, "/home/wu_wa/CICIEC/ImageCompression/test.bmp");
}