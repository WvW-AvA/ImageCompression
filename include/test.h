#ifndef __TEST_H__
#define __TEST_H__
#include "main.h"
#include "bmp.h"
#include "log.h"
#include "stdlib.h"
#include "string.h"
#include "huffman.h"
#include "prediction.h"
int new_image_test()
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

int bmp_compare_test()
{
    bmp *bmp1 = bmp_load("/home/wu_wa/CICIEC/ImageCompression/atri.bmp");
    bmp *bmp2 = bmp_load("/home/wu_wa/CICIEC/ImageCompression/atri_j2k.bmp");
    if (bmp_compare(bmp1, bmp2))
        LOG("bmp1 and bmp2 are" GREEN " equal" RESET);
    else
        LOG("bmp1 and bmp2 are" RED " not equal" RESET);
}

int huffman_encode_test()
{
    //  uint8_t test[11] = {0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3};
    //  huffman_decode_handle decode;
    //  decode.data = test;
    //  decode.size = 11;
    bmp *bmp1 = bmp_load("/home/wu_wa/CICIEC/ImageCompression/atri.bmp");
    huffman_decode_handle decode;
    decode.data = (uint8_t *)(bmp1->data);
    decode.size = bmp1->info->bi_image_size;
    huffman_encode_handle encode = huffman_encode(decode);
    LOG("%d, %d", *((uint16_t *)encode.raw), encode.raw[10]);
    huffman_save(encode, "/home/wu_wa/CICIEC/ImageCompression/save.huffman");
}

int huffman_decode_test()
{
    huffman_encode_handle encode = huffman_load("/home/wu_wa/CICIEC/ImageCompression/save.huffman");
    huffman_decode_handle decode = huffman_decode(encode);
    image img;
    img.hight = 1080;
    img.width = 1920;
    img.data = (color *)decode.data;
    bmp *bmp = bmp_new(&img);
    bmp_save(bmp, "/home/wu_wa/CICIEC/ImageCompression/atri_buffman_decode.bmp");
}
int prediction_test()
{
    bmp *bmp1 = bmp_load("/home/wu_wa/CICIEC/ImageCompression/atri.bmp");
    image img = new_image(bmp1);
    prediction pre;
    pre.predict_type = COLUMN_DIFFER_PREDICT;
    predict(&pre, &img);
    bmp_save(bmp1, "/home/wu_wa/CICIEC/ImageCompression/atri_column_prediction.bmp");
}

int recover_test()
{
    bmp *bmp1 = bmp_load("/home/wu_wa/CICIEC/ImageCompression/atri.bmp");
    image img = new_image(bmp1);
    prediction pre1;
    pre1.predict_type = COLUMN_DIFFER_PREDICT;
    predict(&pre1, &img);
    bmp_save(bmp1, "/home/wu_wa/CICIEC/ImageCompression/atri_column_prediction.bmp");
    recover(&pre1, &img);
    bmp_save(bmp1, "/home/wu_wa/CICIEC/ImageCompression/atri_recover.bmp");
}
#endif