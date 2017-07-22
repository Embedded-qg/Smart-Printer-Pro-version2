#ifndef MORE_INFO_H
#define MORE_INFO_H

/**************************************************************
*	Include File Section
**************************************************************/
#include "cc.h"
#include "stm32f2xx.h"
#include "print_queue.h"

/**************************************************************
*	Function Decalre Section
**************************************************************/
//校验和算法
u16_t Check_Sum(u16_t *data, int len);

//当数据在不同区域时，需要对两个段进行校验和
u16_t Check_Sum_With_Diff_Part(u16_t *src1, int len1, u16_t *src2, int len2);

//获取主控板id
u32_t Get_Printer_ID(void);

//获取时间戳
u32_t Get_Current_Unix_Time(void);

//字节填充0
void Fill_Blank(char *add, u8_t len);

//获取批次时间戳
u32_t Get_Batch_Unix_Time(u16_t batch_number);

//获取订单时间戳
u32_t Get_Order_Unix_Time(u32_t order_number);

/*
	判断缓冲区数据格式是否正确（起始位，订单长度）
	若返回0，则表示正确
	若返回1，则表示错误
*/
s8_t checkBufData(SqQueue *buf,u32_t writePtr);

#endif

