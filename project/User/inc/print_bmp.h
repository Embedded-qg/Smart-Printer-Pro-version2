#ifndef _PRINT_BMP_H
#define _PRINT_BMP_H
#include "compress.h"

void original_print(const unsigned int row,const unsigned int col,unsigned char *buf, unsigned char deviceNum);
void double_print(const unsigned int row,const unsigned int col,unsigned char *buf, unsigned char deviceNum);
void fourflod_print(const unsigned char row,const unsigned char col,unsigned char *buf,unsigned char deviceNum);

#endif
