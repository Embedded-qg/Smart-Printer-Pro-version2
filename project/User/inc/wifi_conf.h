#ifndef WIFI_CONF_H
#define WIFI_CONF_H

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
#include "local_conf.h"
#include "RequestHeap.h"

/**************************************************************
*	Macro Define Section
**************************************************************/

/**************************************************************
*	Function Define Section
**************************************************************/
void 	WifiSendData(u8_t *data, u16_t len);		//从串口发送数据
void 	wifi_receive(void);							//接收串口数据并进行数据处理
void 	WifiReadData(u8_t *data, u16_t *len);		//读取串口缓冲区，有多少读多少，但不移动缓冲区指针
void 	UART6_Hook(u8_t ch);						//将从串口6取到的数据放入缓冲区
void 	AckToWifi(u32_t orderNum, u32_t ipAdd);		//打印机发送应答给本地
void 	WifiRecOrder(u8_t *data);					//接收订单
s8_t 	WifiReadAssignBytes(u8_t *data, u16_t len);	//读取串口缓冲区，指定长度为len
s8_t 	WifiDeleteBuf(u16_t len);					//删除长度为len的缓冲区
void 	putbuf(u8_t *data, u16_t len);				//将wifi缓冲区订单写入加急缓冲区

/****************************************************************************************
*@Name............: SendStatusToLocal
*@Description.....: 发送数据报给本地
*@Parameters......: type		:报文类型
*					symbol		:标志位
*					IPadd		:ip地址
*					preservation:保留字段，打印机状态时为打印单元序号，订单状态时则为订单序号
*@Return values...: void
*****************************************************************************************/
void SendStatusToWifi(req_type type, u8_t symbol, u32_t IPadd, u32_t preservation);
#endif
