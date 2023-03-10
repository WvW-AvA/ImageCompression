
#ifndef __LOG_H__
#define __LOG_H__
#include <stdio.h>
#include <assert.h>
#define __DEBUG__
#ifdef __DEBUG__

#define RESET "\033[0m"
#define BLACK "\033[30m"     /* Black */
#define RED "\033[31m"       /* Red */
#define GREEN "\033[32m"     /* Green */
#define YELLOW "\033[33m"    /* Yellow */
#define BLUE "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"   /* Magenta */
#define CYAN_BLUE "\033[36m" /* Cyan_Blue */

#define LOG(format, ...) printf("[LOG]:" format "\r\n", ##__VA_ARGS__)
#define LOG_WARNING(format, ...) printf("\033[33m[WARNING]:%s:%d:" format "\033[0m\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)                                                                    \
    do                                                                                            \
    {                                                                                             \
        printf("\033[31m[ERROR]:%s:%d:" format "\033[0m\r\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        assert(0);                                                                                \
    } while (0)

#else
#define LOG(format, ...)
#define LOG_WARNING(format, ...)
#define LOG_ERROR(format, ...)                                                                    \
    do                                                                                            \
    {                                                                                             \
        printf("\033[31m[ERROR]:%s:%d:" format "\033[0m\r\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        assert(0);                                                                                \
    } while (0)
#endif

#endif
