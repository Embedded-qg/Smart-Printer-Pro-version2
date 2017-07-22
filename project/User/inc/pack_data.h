#ifndef PACK_DATA_H
#define PACK_DATA_H

/**************************************************************
*	Include File Section
**************************************************************/
#include "cc.h"
#include "more_infomation.h"
#include "data_form.h"



/**************************************************************
*	Macro Define Section
**************************************************************/
//32位数据data填充至add起始的4个字节中
#define SET_DATA_4B(add, data) 		\
	do {							\
		*add = data >> 24;			\
		*(add + 1) = data >> 16;	\
		*(add + 2) = data >> 8;		\
		*(add + 3) = data;			\
	}while(0)

//16位数据data填充至add起始的2个字节中
#define SET_DATA_2B(add, data) 		\
	do {							\
		*add = data >> 8;			\
		*(add + 1) = data;			\
	}while(0)
	
/**************************************************************
*	Function Decalre Section
**************************************************************/
/****************************************************************************************
*@Name............: Pack_Req_Or_Status_Message
*@Description.....: 封装数据报
*@Parameters......: message		:用于存放即将打包的消息
*					type		:报文类型
*					symbol		:标志位
*					id			:可以是主控板id(32位),也可以批次序号(高16位)
*					UNIX_time	:服务器或主控板发送时间
*					preservation:保留字段，在订单中，高16位为批次序号，低16位为批次内序号
*@Return values...: void

*****************************************************************************************/
void Pack_Req_Or_Status_Message(char *message, req_type type, u8_t symbol, u32_t id, u32_t UNIX_time, u32_t preservation);
#endif
