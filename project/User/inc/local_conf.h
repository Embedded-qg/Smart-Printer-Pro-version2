#ifndef LOCAL_CONF_H
#define LOCAL_CONF_H

/**************************************************************
*	Include File Section
**************************************************************/
#include "cc.h"
#include "more_infomation.h"
#include "data_form.h"
#include "analyze_data.h"
#include "queue_buf.h"
#include "main.h"
#include "pack_data.h"
#include "app_cfg.h"
#include "netconf.h"
#include "analyze_data.h"
#include <string.h>

typedef enum status{
	True, Fail, Success, Error
}Status;


/**************************************************************
*	Macro Define Section
**************************************************************/

/**************************************************************
*	Function Define Section
**************************************************************/

void 	LocalSendData(char *data, u16_t len);		//从串口发送数据
void 	local_receive(void);						//接收串口数据并进行数据处理
void 	ReadData(char *data, u16_t *len);			//读取串口缓冲区，有多少读多少，但不移动缓冲区指针
void 	USART3_Hook(u8_t ch);						//将从串口3读取到的数据放入缓冲区
void 	AckToLocal(u32_t orderNum);					//打印机发送应答给本地
void 	DealAckFromLocal(char *data);				//解决来自本地的应答
void 	recOrder(char *data, u16_t len);			//接收订单
s8_t 	ReadAssignBytes(char *data, u16_t len);		//读取串口缓冲区，指定长度为len
s8_t 	DeleteBuf(u16_t len);						//删除长度为len的缓冲区
u8_t 	GetTypeOfAck(char *data);					//获取应答类型
u32_t 	GetAckNumber(char *ack);					//获取应答序号
Status 	CheckOrder(u8_t *data, u16_t len);			//检测订单是否正确
Status 	recAck(u16_t offset);						//接收应答
Status 	CheckRec(u8_t *data, u16_t len);			//检测接收报文是否正确

/****************************************************************************************
*@Name............: SendStatusToLocal
*@Description.....: 发送数据报给本地
*@Parameters......: type		:报文类型
*					symbol		:标志位
*					preservation:保留字段，打印机状态时为打印单元序号，订单状态时则为订单序号
*@Return values...: void
*****************************************************************************************/
void SendStatusToLocal(req_type type, u8_t symbol, u32_t preservation);
#endif
