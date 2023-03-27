#ifndef __TEST_H__
#define __TEST_H__
#include "main.h"
#include "bmp.h"
#include "log.h"
#include "stdlib.h"
#include "string.h"
#include "huffman.h"
#include "prediction.h"
#include "lz77.h"
#include "golomb.h"
#include "jls.h"
int new_image_test()
{
    image im = new_image(3, 3);
    for (int i = 0; i < im.width; i++)
    {
        for (int j = 0; j < im.height; j++)
        {
            color *tem = get_pixiv(&im, i, j);
            tem->A = 255;
            tem->R = 255 * i / 3;
            tem->G = 255 * j / 3;
            tem->B = 0;
        }
    }
    bmp *img = bmp_new(&im);
    bmp_save(img, "/home/wu_wa/CICIEC/ImageCompression/red.bmp");
}

int bmp_compare_test(const char *img1, const char *img2)
{
    bmp *bmp1 = bmp_load(img1);
    bmp *bmp2 = bmp_load(img2);
    bmp_print(bmp1);
    bmp_print(bmp2);
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
    img.height = 1080;
    img.width = 1920;
    img.data = (color *)decode.data;
    bmp *bmp = bmp_new(&img);
    bmp_save(bmp, "/home/wu_wa/CICIEC/ImageCompression/atri_buffman_decode.bmp");
}

int prediction_test()
{
    bmp *bmp1 = bmp_load("/home/wu_wa/CICIEC/ImageCompression/atri.bmp");
    image img = new_image_from_bmp(bmp1);
    prediction pre = new_prediction(LOCO_I_PREDICT);
    predict(&pre, &img);
    bmp_save(bmp1, "/home/wu_wa/CICIEC/ImageCompression/atri_COLO_I_prediction.bmp");
}

int recover_test()
{
    bmp *bmp1 = bmp_load("/home/wu_wa/CICIEC/ImageCompression/atri.bmp");
    image img = new_image_from_bmp(bmp1);
    prediction pre1 = new_prediction(COLUMN_DIFFER_PREDICT);
    predict(&pre1, &img);
    bmp_save(bmp1, "/home/wu_wa/CICIEC/ImageCompression/atri_column_prediction.bmp");
    recover(&pre1, &img);
    bmp_save(bmp1, "/home/wu_wa/CICIEC/ImageCompression/atri_recover.bmp");
}

int golomb_rice_test()
{
    uint32_t res[5];
    uint32_t ind;
    // rice golomb encode decode test
    LOG("// rice golomb encode decode test");
    for (int i = 0; i < 1000; i++)
    {
        ind = 0;
        golomb_rice_encode(i, (uint8_t *)res, &ind, 7);
        ind = 0;
        uint16_t dec = golomb_rice_decode((uint8_t *)res, &ind, 7);
        LOG("%s  %d  %d", print_bit((uint8_t *)res, ind), ind, dec);
    }
    // k-stage exp golomb encode decode test
    LOG("// k-stage exp golomb encode decode test");
    for (uint32_t i = 0; i < 0; i++)
    {
        ind = 0;
        golomb_exp_encode(i, (uint8_t *)res, &ind, 0);
        // LOG("%s", print_bit(&i, 32));
        ind = 0;
        uint32_t dec = golomb_exp_decode((uint8_t *)res, &ind, 0);
        LOG("%s  %d  %d", print_bit((uint8_t *)res, ind), ind, dec);
    }
}

int lz77_test()
{
    uint8_t data[21] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t size = lz77_encode(data, 21);
    uint8_t *d = lz77_decode(data, size, 21);
    printf("\n");
    for (int i = 0; i < 21; i++)
        printf("%d ", d[i]);
    printf("\n");
}

int jls_encode_test(const char *img_path, const char *tar_path)
{
    bmp *bmp = bmp_load(img_path);
    image img = new_image_from_bmp(bmp);
    jls jls = jls_init(&img, LINE_SCAN);
    jls_encode(&img, &jls);
    jls_save(&jls, tar_path);
    jls_free(&jls);
}

int jls_decode_test(const char *myjls_path, const char *img_path)
{
    jls jls = jls_load(myjls_path);
    image img = jls_decode(&jls);
    bmp *bmp = bmp_new(&img);
    bmp_save(bmp, img_path);
}
#endif