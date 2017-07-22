#ifndef QUEUE_BUF_H
#define QUEUE_BUF_H

#include "cc.h"
#include "os_cpu.h"
#include "ucos_ii.h"
#include "data_form.h"

#define ElemType u8_t 

/*循环缓冲结构定义：
@base : 指向环形缓冲区首地址
@read : 缓冲区读指针
@write : 缓冲区写指针 
*/
typedef struct 
{
	ElemType *base;
	u32_t read;                    
	u32_t write;
	u32_t MAX;       //存储缓冲区最大空间
	unsigned char buf_empty;   //缓冲区空闲标志：1为空，0为非空
	struct os_event *mutex;
}SqQueue;


/*************Read_Order_Length定义******************/
#define Que_NOT_EMPTY				 	0
#define Que_EMPTY 					 	1

#define BUF_OK               0
/*******************循环缓冲区错误定义******************/
#define BUF_FULL            -2
#define BUF_EMPTY 			-3
#define BUF_ARG_ERR         -4
#define BUF_DATA_ERR        -5

#define MAXQSIZE (1024 * 10)   	//定义环形缓冲区大小为10K
#define MAXUSIZE (1024 * 10)  	//定义加急缓冲区大小为10K
#define BUF_END 4              	//定义订单尾部长度
#define BUF_HEAD 24            	//定义订单头部长度



/*
	判断是否大于阈值的宏
	*/
	
#define  THRESHOLD_SIZE	(1024 * 7)	//本来阈值是7k，现在是4k，以后恢复

extern SqQueue queue_buf;    
extern SqQueue urgent_buf; 

extern void Init_Queue(void);
s8_t Write_Buf(SqQueue *buf,ElemType *e,u32_t len);
s8_t Read_Order_Length_Queue(SqQueue buf,u16_t *order_len);
s8_t Read_Order_Queue(SqQueue *buf,ElemType *e);
u8_t Is_Empty_Queue(SqQueue buf);
u8_t *Get_Batch_Head(SqQueue buf);
u8_t *Get_Order_Head(SqQueue buf);
s8_t Check_Buf_Request_Signal(SqQueue buf);

#endif
