#include "compress.h"

//#define ORIGINAL_PRINT       //原图打印
#define DOUBLE_PRINT       //双倍打印
//#define FOURFLOD_PRINT
//#define SEC_COMPRESS       //二次压缩

static unsigned char uncompress_map[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

/*
 *函数作用：用行程编码的方式对位图进行压缩
 *参数：
				@src ：未压缩前数据存放内存首地址
				@src_end ：未压缩前数据存放内存末地址 
				@row : 打印数据的行数
				@col : 打印数据的列数
				@des : 数据压缩后存放的内存首地址
	*返回值:打印数据压缩后的字节数
 */
unsigned int compress(const unsigned char *src,const unsigned char *src_end,unsigned char row,unsigned char col,unsigned char *des)
{
	volatile unsigned char count = 0x00,map = 0x80;
	unsigned char *des_start = des;
	const unsigned char *src_start = src;
	
	/*压缩完毕后的数组头两个字节代表行数、列数，其余字节为压缩的图像数据*/
	*des++ = row;
	*des++ = col;
#ifdef 	SEC_COMPRESS 
	while(1)
	{
		/*测试连续的1*/
		while((*src) & map)
		{
			if(count++ == 200)    //des存满一个字节
			{
				*des++ = 0x80;
				count = 0;
			}
			else if((map >>= 1) == 0)  //src测试完一个字节
			{
				if(++src == src_end+1)    //假如src已经转换完毕,跳出循环
					break;
				
				map = 0x80;
			}
		}	
		if((count != 0)&&(count <= 127))
		{
			*des++ = 128 + count;
			count = 0;
		}
		else if((count >127)&&(count < 200))
		{
			*des++ = 0xff;
			*des++ = 128 + (count - 127);
			count = 0;
		}
	
		if(src == src_end+1)  //假如src已经转换完毕,跳出循环
		{
			break;
		}
		/*测试连续的0*/
		while(!((*src) & map))
		{
			if(count++ == 127)    //des存满一个字节
			{
				*des++ = 127;
				count = 0;
			}
			else if((map >>= 1) == 0)  //src测试完一个字节
			{
				if(++src == src_end+1)    //假如src已经转换完毕,跳出循环
					break;
				
				map = 0x80;
			}
		}
		
		if(count != 0)
		{
			*des++ = count;
			count = 0;
		}
		
		if(src == src_end+1)  //假如src已经转换完毕,跳出循环
		{
			break;
		}
	}
#else
		while(1)
	{
		/*测试连续的1*/
		while((*src) & map)
		{
			if(count++ == 127)    //des存满一个字节
			{
				*des++ = 0xff;
				count = 0;
			}
			else if((map >>= 1) == 0)  //src测试完一个字节
			{
				if(++src == src_end+1)    //假如src已经转换完毕,跳出循环
					break;
				
				map = 0x80;
			}
		}
		
		if(count != 0)
		{
			*des++ = 128 + count;
			count = 0;
		}
		
		
		if(src == src_end+1)  //假如src已经转换完毕,跳出循环
		{
			break;
		}
		/*测试连续的0*/
		while(!((*src) & map))
		{
			if(count++ == 127)    //des存满一个字节
			{
				*des++ = 127;
				count = 0;
			}
			else if((map >>= 1) == 0)  //src测试完一个字节
			{
				if(++src == src_end+1)    //假如src已经转换完毕,跳出循环
					break;
				
				map = 0x80;
			}
		}
		
		if(count != 0)
		{
			*des++ = count;
			count = 0;
		}
		
		if(src == src_end+1)  //假如src已经转换完毕,跳出循环
		{
			break;
		}
	}
	
#endif
//	printf("压缩前字节数：%d,   压缩后字节数：%d,   压缩率为：%d%%\n",src_end-src_start+1,(des-des_start),100-(int)((float)(des-des_start)/(float)(src_end-src_start+1)*100.0));
	
	return (des-des_start);
}


/*
 *解压并打印
 */
void uncompress_print(const unsigned char *src,const unsigned char *src_end, unsigned char deviceNum)
{ 
  unsigned char line_buf[20] = {0x00}; //存放解压后一行的数据
	unsigned char count = 0x00;  //存放待解压的字节量
	unsigned char bit_count = 0x00;   //字节内部的计数
	const unsigned char row = *src;
	const unsigned char col = *(src+1);
	unsigned char i = 0; //存储行内数据量
	
		//发送位图打印指令
#ifdef ORIGINAL_PRINT
							PRINT_FAST_BITMAP(0,col,0,row,0, deviceNum)
#endif
#ifdef DOUBLE_PRINT
							PRINT_FAST_BITMAP(0,col*2,0,row*2,0, deviceNum)
#endif
#ifdef FOURFLOD_PRINT
							PRINT_FAST_BITMAP(0,col*4,0,row*4,0, deviceNum)
#endif 
	
	src += 2;
	while(src!=src_end+1)
	{
		count  = *src & 0x7F;
#ifdef SEC_COMPRESS		
		if(*src == 0x80)
			count = 200;
#endif
		if(*src & 0x80)  //为1的点
		{
			
			while(count >0)
			{
				if(bit_count != 0)   //在一个字节内还存在0，则1接在0的末端接着填写
				{
					line_buf[i] |=  uncompress_map[bit_count-1];
					bit_count--;
					count--;
					if(!bit_count)
					{
						i++;
						if(i == col)
						{
	#ifdef ORIGINAL_PRINT
							original_print(row,col,line_buf, deviceNum);
	#endif
	#ifdef DOUBLE_PRINT
							double_print(row,col,line_buf, deviceNum);
	#endif
	#ifdef FOURFLOD_PRINT
							fourflod_print(row,col,line_buf, deviceNum);
	#endif 
							i = 0;
						}	
					}
				}
				
				if(!bit_count && count>=8)
				{
						line_buf[i] = 0xFF; 
						i++;
						if(i == col)
						{
	#ifdef ORIGINAL_PRINT
							original_print(row,col,line_buf, deviceNum);
	#endif
	#ifdef DOUBLE_PRINT
							double_print(row,col,line_buf,deviceNum);
	#endif
	#ifdef  FOURFLOD_PRINT
							fourflod_print(row,col,line_buf,deviceNum);
	#endif 
							
							i = 0;
						}	
						
						count -= 8;
				}
				else if(!bit_count && count < 8)
				{
					bit_count = 8;
					
					while(count > 0)
					{
						line_buf[i] |= uncompress_map[bit_count-1];
						bit_count--;
						count--;
					}
				}	
			}
		}
		else   //为0的点
		{
			while(count >0)
			{
				if(bit_count != 0)
				{
					line_buf[i] &=  ~uncompress_map[bit_count-1];
					bit_count--;
					count--;
					if(!bit_count)
					{
						i++;
						if(i == col)
						{
	#ifdef ORIGINAL_PRINT
							original_print(row,col,line_buf, deviceNum);
	#endif
	#ifdef DOUBLE_PRINT
							double_print(row,col,line_buf,deviceNum);
	#endif
	#ifdef  FOURFLOD_PRINT
							fourflod_print(row,col,line_buf,deviceNum);
	#endif 
							
							i = 0;
						}	
					}
				}
				
				if(!bit_count && count>=8)
				{
						line_buf[i] = 0x00; 
						i++;
						if(i == col)
						{
	#ifdef ORIGINAL_PRINT
							original_print(row,col,line_buf, deviceNum);
	#endif
	#ifdef DOUBLE_PRINT
							double_print(row,col,line_buf,deviceNum);
	#endif
	#ifdef  FOURFLOD_PRINT
							fourflod_print(row,col,line_buf,deviceNum);
	#endif 
							
							i = 0;
						}	
							count -= 8;
				}
				else if(!bit_count && count < 8)
				{
					bit_count = 8;
					
					while(count > 0)
					{
						line_buf[i] &= ~uncompress_map[bit_count-1];
						bit_count--;
						count--;
					}
				}
					
			}
		}
		src++;
	}
}
