#ifndef COMPRESS_H
#define COMPRESS_H

#include "printerException.h"
#include "stm32f2xx_usart.h"
#include "format.h"
#include "print_bmp.h"


unsigned int compress(const unsigned char *src,const unsigned char *src_end,unsigned char col,unsigned char row,unsigned char *des);
void uncompress_print(const unsigned char *src,const unsigned char *src_end, unsigned char deviceNum);

#endif

