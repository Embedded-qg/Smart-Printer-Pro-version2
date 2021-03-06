#include"print_bmp.h"
#include "printerException.h"


/* 
 *函数作用：原图打印
 *函数参数：
		@row : 打印位图的行数
		@col : 打印位图的列数
		@buf : 位图中一行的数据
  *函数返回值：空
 */
void original_print(const unsigned int row,const unsigned int col,unsigned char *buf, unsigned char deviceNum)
{
	unsigned char original_buf[25] = {0x00};
	unsigned int i = 0;
	
	for(i=0; i<col; i++)
	{
		PRINTER_PUT(*(buf+i), deviceNum);
	}	
}

	
/* 
 *函数作用：双倍放大
 *函数参数：
		@row : 打印位图的行数
		@col : 打印位图的列数
		@buf : 位图中一行的数据
  *函数返回值：空
 */
void double_print(const unsigned int row,const unsigned int col,unsigned char *buf, unsigned char deviceNum)
{
	unsigned short amplify_buf[25] = {0x00};
	unsigned char map = 0x00;  //掩码
	unsigned int i = 0x00; //对字节扩展进行计数，倍宽打印
	unsigned int j = 0x00; //倍高打印
	unsigned char *p = (unsigned char *)amplify_buf;   //对amplify_buf进行字节引用
	
	for(i=0;i<col;i++)
	{
		for(map=0x80;map!=0x00;map>>=1)
		{
			amplify_buf[i] <<= 2;
			if((*buf & map) != 0x00)
			{
				amplify_buf[i] |= 0x03;
			}
		}
		buf++;
	}
	for(j = 0; j<2; j++)
	{
		for(i=0; i<col*2; i+=2)
		{
			PRINTER_PUT(*(p+i+1), deviceNum);
			PRINTER_PUT(*(p+i), deviceNum);
			
//			putchar(*(p+i+1));
//			putchar(*(p+i));
			
		}	
		p = (unsigned char *)amplify_buf;
	}

}

/* 
 *函数作用：四倍放大
 *函数参数：
		@row : 打印位图的行数
		@col : 打印位图的列数
		@buf : 位图中一行的数据
		@deviceNum:
  *函数返回值：空
 */
void fourflod_print(const unsigned char row,const unsigned char col,unsigned char *buf, unsigned char deviceNum)
{
	unsigned int four_amplify_buf[25] = {0x00};
	unsigned char map = 0x00;  //掩码
	unsigned char i = 0x00; //对字节扩展进行计数�4犊泶蛴�
	unsigned char j = 0x00; //4倍高打印
	unsigned char *p = (unsigned char *)four_amplify_buf;   //对amplify_buf进行字节引用
	

	for(i=0;i<col;i++)
	{
		for(map=0x80;map!=0x00;map>>=1)
		{
			four_amplify_buf[i] <<= 4;
			if((*buf & map) != 0x00)
			{
				four_amplify_buf[i] |= 0x0f;
			}
		}
		buf++;
	}
	for(j = 0; j<4; j++)
	{
		for(i=0; i<col*4; i+=4)
		{
			PRINTER_PUT(*(p+i+3), deviceNum);
			PRINTER_PUT(*(p+i+2), deviceNum);
			PRINTER_PUT(*(p+i+1), deviceNum);
			PRINTER_PUT(*(p+i), deviceNum);
			
//			putchar(*(p+i+3));
//			putchar(*(p+i+2));
//			putchar(*(p+i+1));
//			putchar(*(p+i));
		}	
		p = (unsigned char *)four_amplify_buf;
	}
}

