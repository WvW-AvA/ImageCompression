
#ifndef __LOG_H__
#define __LOG_H__
#include <stdio.h>
#define __DEBUG__

#ifdef __DEBUG__
#define LOG(format, ...) printf("[LOG]:" format "\r\n", ##__VA_ARGS__)
#define LOG_WARNING(format, ...) printf("\033[33m[WARNING]:%s:%d:" format "\033[0m\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) printf("\033[31m[ERROR]:%s:%d:" format "\033[0m\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG(format, ...)
#define LOG_WARNING(format, ...)
#define LOG_ERROR(format, ...) printf("\033[31m[ERROR]:%s:%d:" format "\033[0m\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#endif
