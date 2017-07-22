#ifndef DATA_FORM_H
#define DATA_FORM_H

/**************************************************************
*	Include File Section
**************************************************************/
#include "cc.h"

/******************订单头部格式各个字段的偏移量*****************/
#define ORDER_START_SYMBOL_OFFET          0  //开始偏移值
#define ORDER_SIZE_OFFSET                 2	 //订单长度偏移值
#define ORDER_MCU_ID_OFFSET               4  //主控板id偏移值
#define ORDER_SEVER_SEND_TIME_OFFSET	  8  //服务器发送时间偏移值
#define ORDER_SERIAL_NUMBER_OFFSET		  12 //订单序号偏移值
#define ORDER_BATCH_NUMBER_OFFSET		  16 //所属批次偏移值
#define ORDER_BATCH_WITHIN_NUMBER_OFFSET  18 //批次内序号偏移值
#define ORDER_CHECK_SUM_OFFSET			  20 //校验和偏移值
#define ORDER_PRESERVATION_OFFSET		  22 //保留值偏移值
#define ORDER_DATA_OFFSET				  24 //数据域偏移值

/******************批次头部格式各个字段的偏移量*****************/
#define BATCH_START_SYMBOL_OFFSET		  0  //开始偏移值
#define BATCH_ORDER_NUMBER_OFFSET		  2  //订单个数偏移值
#define BATCH_NUMBER_OFFSET				  4  //批次编号偏移值
#define BATCH_TOTAL_LENGTH_OFFSET		  6  //批次长度
#define BATCH_SEVER_SEND_TIME_OFFSET	  8  //服务器发送时间偏移值
#define BATCH_CHECK_SUM_OFFSET			  12 //批次校验和偏移值
#define BATCH_PRESERVATION_OFFSET		  14 //保留值偏移值
#define BATCH_TAIL_OFFSET				  18 //批次尾部偏移值

/***********************状态头部表格长度***********************/
#define SEND_DATA_SIZE 				  	  20 //状态头部长度
#define REC_DATA_SIZE					  20 //状态头部长度
//原本应定义为STATUS_HEAD_SIZE的，但考虑接收与反馈的概念不同，故用这种方法

/**************************************************************
*	Struct Define Section
**************************************************************/
//订单数据表
typedef struct order_information {
	u32_t mcu_id;	 		   //主控板id
	u32_t sever_send_time;     //服务器发送时间
	u32_t serial_number;	   //订单序号
	u16_t size;		  		   //订单长度
	u16_t batch_number;		   //所属批次
	u16_t batch_within_number; //批次内序号
	u16_t check_sum;		   //校验和
	u16_t preservation;		   //保留	
	u16_t data_source;		   //数据来源
	u8_t *data;				   //数据域
	u8_t  priority;			   //优先级，判断是否为加急订单
	u8_t  status;			   //状态
	u8_t  next_print_node;	   //下一打印结点	
}order_info;

//批次数据表
typedef struct batch_information {
#define MAX_BATCH_NUM	10
#define MAX_BATCH_HEAD_LENGTH 20
	u32_t sever_send_time;    //服务器发送时间
	u16_t order_number;		  //订单数量
	u16_t batch_length;		  //批次长度
	u16_t batch_number;		  //所属批次
	u16_t check_sum;		  //校验和
	u16_t preservation;		  //保留字段，最后一位是紧急指针
	u8_t  num_printed_order;  //已打印完成订单数量
	u8_t  num_order_que;
}batch_info;

//状态反馈报文类型
typedef enum Req_Type {
	first_req, 		//初次请求数据链接
	order_req, 		//达到阈值请求订单
	batch_status, 	//批次状态
	order_status, 	//订单状态
	printer_status,	//打印机状态
	wifi_order_req	//wifi订单请求
}req_type;

#define STATUS_TYPE_OFFSET   2	//状态类型偏移值
/**********************状态定义*************************/
#define BATCH_STATUS 	0x00	//批次状态
#define ORDER_STATUS	0x20    //订单状态
#define PRINTER_STATUS  0x40    //打印机状态
#define FIRST_REQ		0x60	//初次请求链接
#define ORDER_REQ		0x80	//达到阈值请求订单
#define ACK_STATUS		0xa0	//本地状态应答
#define WIFI_ORDER_REQ	0xc0	//wifi订单请求应答
#define WIFI_REQ_ACK    0xe0	//应答wifi请求

/**********************标志定义*************************/
#define BATCH_SUCCESS		0x00	//批次打印成功
#define BATCH_ENTER_BUF 	0x01	//批次进入缓冲区
#define	BATCH_FAIL			0x02	//批次打印失败

#define OEDER_SUCCESS		0x00	//订单打印成功
#define ORDER_FAIL			0x01	//订单打印失败
#define ORDER_ENTER_QUEUE	0x02	//订单进入打印队列
#define ORDER_BEGIN_PRINT	0x03	//订单开始打印
#define ORDER_DATA_ERR		0x04	//订单数据错误
#define ORDER_DATA_OK		0x05	//订单数据校验成功

#define PRINTER_ACK_LOCAL	1		//本地与打印机之间的应答
#define ORDER_ACK_LOCAL		0		//本地与订单之间的应答

#define ORDER_REQUEST 		0x00 	//达到阈值请求
#define REQ_LINK_OK	 		0x00	//请求链接成功


/*************************************************************************PrinterExcetpion.c*************************************************************************************************/
//Printer-State
#define KNIFE_ERROR_STATE			0x01	//knife error
#define UNEXPECTED_OPENED_STATE		0x02	//unexpectedly open
#define PAPER_INSUFFICIENT_STATE	0x03	//insufficient paper
#define FILL_IN_PAPER_STATE			0x04	//the printer is being filled in the paper
#define PAPER_WILL_BE_IN_INSUFFICIENT_STATE 0x05	//the printer will be in  insufficient paper state

#define NORMAL_STATE				0x09	//the printer is in normal state
#define EXCEPTION_NUM				5

//#define HIGH_TEMPERATURE_STATE		6
//#define SELF_CHECK_STATE					7

//Whether the order is printed successfully or not
#define ORDER_CREATED_OK					0x73     	//the order is printed successfully
#define ORDER_CREATED_FAIL				0x22			//fail to print the order

//Device-Port
#define USART1_PORT 1
#define USART2_PORT 2
#define USART3_PORT 3

//Iamges-Scale
#define NORMAL_SIZE 							0x00            		//普通尺寸
#define DOUBLE_WIDTH 							0x01					  		//倍宽
#define DOUBLE_LENGTH 						0x02								//倍高
#define FOUR_TIMES_AMPLIFICATION 	0x03								//四倍大小
	

////EEPRROM	储存状态宏
//#define EEPROM_CHECK_OK 					0xef								//EEPROM存储正常
//#define EEPROM_CHECK_ERROR				0xee								//EEPROM存储出错

//EEPRROM	存储段的分配（为了方便读写，都用4字节表示）
//设备1的数据空间 （0x00 - 0x1c）

#define DEVICE_ONE_START_ADDR								0x00
#define TOTAL_WORKING_TIME_ADDRESS_DEVICE_ONE 				0x00  		//总打印时间，4字节的长度, 单位：分钟
#define TOTAL_KNIFE_CUT_TIMES_ADDRESS_DEVICE_ONE 			0x04			//总切刀次数，4字节的长度，单位：次数
#define TOTAL_PRINTED_LENGTH_DEVICE_ONE 							0x08			//总打印长度，4字节的长度，单位：厘米

//记录设备1各异常类型的发生次数的地址
#define KNIFE_ERROR_TIMES_ADDRESS_DEVICE_ONE				0x0c			//切刀错误次数，4字节的长度, 单位：次数
#define UNEXPECTED_OPENED_TIMES_ADDRESS_DEVICE_ONE			0x10			//机头被打开次数，4字节的长度, 单位：次数
#define PAPER_INSUFFICIENT_TIMES_ADDRESS_DEVICE_ONE			0x14			//缺纸次数，4字节的长度, 单位：次数

#define FULL_IN_PAPER_TIMES_ADDRESS_DEVICE_ONE				0x18			//进纸次数，4字节的长度, 单位：次数
//#define HIGH_TEMPERATURE_TIMES_ADDRESS_DEVICE_ONE			0x1c			//温度过高次数，4字节的长度, 单位：次数

//设备2的数据空间	(0x32 - 0x4e)

#define DEVICE_TWO_START_ADDR								0x32
#define TOTAL_WORKING_TIME_ADDRESS_DEVICE_TWO 				0x32  			//总打印时间，4字节的长度, 单位：分钟
#define TOTAL_KNIFE_CUT_TIMES_ADDRESS_DEVICE_TWO			0x36			//总切刀次数，4字节的长度，单位：次数
#define TOTAL_PRINTED_LENGTH_DEVICE_TWO 					0x3a			//总打印长度，4字节的长度，单位：厘米

//记录设备2各异常类型的发生次数的地址
#define KNIFE_ERROR_TIMES_ADDRESS_DEVICE_TWO				0x3e			//切刀错误次数，4字节的长度, 单位：次数
#define UNEXPECTED_OPENED_TIMES_ADDRESS_DEVICE_TWO			0x42			//机头被打开次数，4字节的长度, 单位：次数
#define PAPER_INSUFFICIENT_TIMES_ADDRESS_DEVICE_TWO			0x46			//缺纸次数，4字节的长度, 单位：次数

#define FULL_IN_PAPER_TIMES_ADDRESS_DEVICE_TWO				0x4a			//进纸次数，4字节的长度, 单位：次数
//#define HIGH_TEMPERATURE_TIMES_ADDRESS_DEVICE_TWO			0x4e			//温度过高次数，4字节的长度, 单位：次数


#endif


