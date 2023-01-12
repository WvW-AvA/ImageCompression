#ifndef __MAIN_H__
#define __MAIN_H__
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t A;
} color;

typedef struct
{
    uint32_t width;
    uint32_t hight;
    color *data;
} image;

color *get_pixiv(image *target, uint32_t x, uint32_t y);
#endif