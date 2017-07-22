/***************************************************************************************
 *	FileName					:	status_mesg.h
 *	CopyRight					:
 *	ModuleName					:	
 *
 *	CPU							:
 *	RTOS						:
 *
 *	Create Data					:	2016/07/29
 *	Author/Corportation			:	subaideng
 *
 *	Abstract Description		:	
 *
 *--------------------------------Revision History--------------------------------------
 *	No		version		Date		Revised By		Item		Description
 *	1		v1.0		2016/		subaideng					Create this file
 *
 ***************************************************************************************/
 
 
/**************************************************************
*        Multi-Include-Prevent Section
**************************************************************/
#ifndef STATUS_MESG_H
#define STATUS_MESG_H

/**************************************************************
*        Debug switch Section
**************************************************************/


/**************************************************************
*        Include File Section
**************************************************************/
#include "cc.h"
#include "ucos_ii.h"
#include <stdlib.h>
#include "print_queue.h"
#include "app_cfg.h"
#include "wifi_conf.h"

/**************************************************************
*        Macro Define Section
**************************************************************/

/*
	PRINTER_MESG_IP WiFi下单时，打印单元状态反馈的广播IP
*/
#define PRINTER_MESG_IP 0


/*
	消息队列长度
*/
#define MESG_QUEUE_SIZE	20

/*
	MesgQueueNode中使用，填充flag变量，用于标识是哪一类型的消息
*/
#define BATCH_MESG_QUE_FLAG					1
#define ORDER_QUEUE_MESG_QUE_FLAG		2
#define ORDER_PRINT_MESG_QUE_FLAG		3
#define PRINTER_MESG_QUE_FLAG				4

/*
	本地与远程数据宏
*/
#define REMOTE_SOURCE 0 //从网络接收的初始订单
#define LOCAL_SOURCE	1	//从本地初始接收的订单
#define REMOTE_RE_SOURCE 2	//从网络接收的异常修正订单
#define LOCAL_RE_SOURCE	3		//从本地接收的异常修正订单
#define WIFI_SOURCE 4	//从WiFi接受订单
#define WIFI_RE_SOURCE 5 //从WiFi接受异常修正订单



/*
	Batch_Status_Send函数所用宏
	批次状态
*/
#define BATCH_PRINT_SUCCESS  0
#define BATCH_ENUEUE_SUCCESS 1

/* 
	Order_QUEUE_Status_Send 参数
	打印队列状态
	*/
#define ENQUEUE_OK 0   //发送订单进入打印队列成功报文的宏
#define ENQUEUE_ERR 1  //发送订单进入打印队列失败报文的宏
#define ENQUQUE_START 2

/* 
	Order_Print_Status_Send中，判断订单状态的参数
	订单打印状态
*/
#define PRINT_STATUS_OK 						0			//打印成功
#define PRINT_STATUS_DATA_ERR 			1			//打印失败，订单数据有误
#define PRINT_STATUS_MACHINE_ERR 		2			//打印失败，打印机异常
#define PRINT_STATUS_START 					3			//开始打印



/*
	各类报文状态值
	*/

//批次状态
#define BATCH_PRINT_SUC				0		//成功打印批次
#define	BATCH_ENQUE_BUF_SUC		1		//成功进入缓存区
	
//订单状态
#define NOR_ORDER_PRINT_SUC		0		//普通订单打印成功
#define NOR_ORDER_PRINTER_ERR	1		//普通订单打印出错：打印机异常
#define NOR_ORDER_ENQUE_SUC		2		//普通订单进入打印队列
#define NOR_ORDER_PRINT_START	3		//普通订单开始打印
#define NOR_ORDER_DATA_ERR		4		//普通订单订单数据解析错误

#define EX_ORDER_PRINT_SUC		5		//前异常订单打印成功
#define EX_ORDER_PRINTER_ERR	6		//前异常订单打印出错：打印机异常
#define EX_ORDER_ENQUE_SUC		7		//前异常订单进入打印队列
#define EX_ORDER_PRINT_START	8		//前异常订单开始打印
#define EX_ORDER_DATA_ERR			9		//前异常顶大订单数据解析错误

/*
	Printer_Status_Send函数所用宏
	打印机状态
*/
#define PRINTER_CUT_ERR 							0x01		//切刀错误
#define PRINTER_CASE_OPEN 						0x02		//机盒打开
#define PRINTER_PAPER_MAY_RUNOUT 			0x03		//纸将用尽
#define PRINTER_PAPER_RUNOUT 					0x04		//缺纸
#define PRINTER_PAPER_FEED						0x05		//正在进纸
#define PRINTER_HIGH_TEMP							0x06		//机芯高温
#define PRINTER_BURN_DOWN 						0x07		//机芯烧毁
#define PRINTER_NORMAL								0x09		//正常状态
#define PRINTER_BUF_NORMAL_FULL 			0x0c		//普通缓冲区满
#define PRINTER_BUF_URGENT_FULL				0x0d		//紧急缓冲区满
#define PRINTER_HEALTH_HEALTHY				0x0e	//普通状态
#define PRITNER_HEALTH_SUB_HEALTH 		0x0f	//亚健康状态
#define PRINTER_HEALTH_UNHEALTHY			0x10	//不健康状态



/*
	getHealthValue 返回值
	*/
#define HEALTHY						PRINTER_HEALTH_HEALTHY				//健康
#define SUB_HEALTHY				PRITNER_HEALTH_SUB_HEALTH			//亚健康

/*
	打印机长期状态阈值
	*/
#define TOTAL_WORKTIME_THRESHOLD	 						100000000 //总工作时长阈值：2年 单位0.1s	待测
#define TOTAL_CUT_NUM_THRESHOLD								1000000		//总切刀次数阈值：100万次
#define TOTAL_PRINT_LEN_THRESHOLD							10000000	//总打印长度阈值：100km,单位cm
#define ERR_NUM_THRESHOLD											10000			//总错误发生次数：1万次   			待测
#define KEEP_WORKING_TIME_THRESHOLD						10000			//最长工作时间：单位0.1s				待测

/**************************************************************
*        Struct Define Section
**************************************************************/

//批次消息数据结构体
typedef struct{
	u16_t batch_num;
}Batch_Status;

//批次消息数据结构体
typedef struct{
	u16_t batch_num;
	u16_t order_num; 
	u8_t data_source;
}Order_QUEUE_Status;

//批次消息数据结构体
typedef struct{
	u16_t batch_num;
	u16_t order_num; 
	u8_t data_source;
}Order_Print_Status;

//批次消息数据结构体
typedef struct{
	u32_t printer_num;
}Printer_Status;


typedef union{
	Batch_Status				batch_Status;
	Order_QUEUE_Status	order_QUEUE_Status;
	Order_Print_Status 	order_Print_Status;
	Printer_Status			printer_Status;
}MesgQueueData;

/*
	消息队列中指针所指内容节点
	flag，表示是何种类型消息
mesgQueueData :消息内容
status ：状态
*/
typedef struct{
	u8_t flag ;
	MesgQueueData mesgQueueData;
	u32_t ip;
	u8_t status;		
}MesgQueueNode;




/**************************************************************
*        Prototype Declare Section
**************************************************************/


extern void* MesgQueArray [MESG_QUEUE_SIZE];
extern MesgQueueNode MesgQueBuf[MESG_QUEUE_SIZE];
extern u32_t MesgQueIndex ; 

/**
 *  @fn		Batch_Status_Send
 *	@brief 	发送批次状态报文
 *	@param	batch_num 	
						status 			批次状态
											（BATCH_PRINT_SUCCESS ：批次打印成功 
												BATCH_ENUEUE_SUCCESS：批次进入打印队列成功）
 *	@ret	None
 */
void Batch_Status_Send(u16_t batch_num ,u8_t status);

/**
 *  @fn		Order_QUEUE_Status_Send
 *	@brief 	发送订单进入打印队列状态报文
 *	@param	order 		订单结构体指针
						status		订单状态
 *	@ret	None
 */
void Order_QUEUE_Status_Send( order_info* order , u8_t status);

/**
 *  @fn		Order_Print_Status_Send
 *	@brief 	发送订单打印状态报文
 *	@param	order 		订单结构体指针
						status		订单状态
 *	@ret	None
 */
void Order_Print_Status_Send ( order_info* order, u8_t status);

/**
 *  @fn		Printer_Status_Send
 *	@brief 	发送订单打印状态报文
 *	@param	printer_num 	打印机号
						status		打印机状态
 *	@ret	None
 */
void Printer_Status_Send(u32_t printer_num , u8_t status);


/**
 *  @fn		MesgQue_Deal
 *	@brief 	分析打印机健康状态
 *	@param			
 *	@ret	ans
 */
void MesgQue_Deal();

/**
 *  @fn		getHealthValue
 *	@brief 	分析打印机健康状态
 *	@param	status		打印机状态
 *	@ret	ans	返回健康值：
								健康
								亚健康
								不健康
 */
u8_t getHealthValue(u32_t printer_num);


/**
 *  @fn		Health_Detect_Fun
 *	@brief 	健康状态检测任务
 *	@param	
 *	@ret	ans	
 */
void Health_Detect_Fun();
/**************************************************************
*        End-Multi-Include-Prevent Section
**************************************************************/
#endif