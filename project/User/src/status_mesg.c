/***************************************************************************************
 *	FileName					:	status_mesg.c
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
*	Debug switch Section
**************************************************************/
//#define DEBUG_DETECT

/**************************************************************
*	Include File Section
**************************************************************/
#include "status_mesg.h"


/**************************************************************
*	Macro Define Section
**************************************************************/
	
/**************************************************************
*	Struct Define Section
**************************************************************/
//消息队列
void* MesgQueArray [MESG_QUEUE_SIZE];
//消息内容缓存队列
MesgQueueNode MesgQueBuf[MESG_QUEUE_SIZE];
u32_t MesgQueIndex = 0; 

/**************************************************************
*	Prototype Declare Section
**************************************************************/


/**************************************************************
*	Global Variable Declare Section
**************************************************************/
extern OS_EVENT *Mesg_Queue;
extern struct netconn *order_netconn;
extern void ShowTime(u32_t order_time, INT32U startTime, INT32U OSTime);
extern INT32U StartTime; //基准时间
/**************************************************************
*	File Static Variable Define Section
**************************************************************/


/**************************************************************
*	Function Define Section
**************************************************************/


/**
 *  @fn		Batch_Status_Send
 *	@brief 	发送批次状态报文
 *	@param	batch_num 	
						status 			批次状态
											（BATCH_PRINT_SUCCESS ：批次打印成功 
												BATCH_ENUEUE_SUCCESS：批次进入打印队列成功）
 *	@ret	None
 */
void Batch_Status_Send(u16_t batch_num ,u8_t status)
{
	MesgQueBuf[MesgQueIndex].flag = BATCH_MESG_QUE_FLAG;
	MesgQueBuf[MesgQueIndex].mesgQueueData.batch_Status.batch_num = batch_num;
	MesgQueBuf[MesgQueIndex].status = status;
	OSQPost(Mesg_Queue,&MesgQueBuf[MesgQueIndex]);
	MesgQueIndex = (MesgQueIndex + 1) % MESG_QUEUE_SIZE;
	return;
}


/**
 *  @fn		Order_QUEUE_Status_Send
 *	@brief 	发送订单进入打印队列状态报文
 *	@param	order 		订单结构体指针
						status		订单状态
 *	@ret	None
 */
void Order_QUEUE_Status_Send ( order_info* order , u8_t status )
{
	DEBUG_PRINT_TIMME("进入打印队列开始，订单编号为：%u，",order->serial_number);//1ms中断一次*时钟节拍
	ShowTime(order->sever_send_time,StartTime,OSTimeGet()*TIME_INTERVAL);
	MesgQueBuf[MesgQueIndex].flag = ORDER_QUEUE_MESG_QUE_FLAG;
	MesgQueBuf[MesgQueIndex].mesgQueueData.order_QUEUE_Status.batch_num = order->batch_number;
	MesgQueBuf[MesgQueIndex].mesgQueueData.order_QUEUE_Status.order_num = order->batch_within_number;
	MesgQueBuf[MesgQueIndex].mesgQueueData.order_QUEUE_Status.data_source = order->data_source;
	MesgQueBuf[MesgQueIndex].ip = order->sever_send_time;
	MesgQueBuf[MesgQueIndex].status = status;	

	OSQPost(Mesg_Queue,&MesgQueBuf[MesgQueIndex]);
	
	MesgQueIndex = (MesgQueIndex + 1) % MESG_QUEUE_SIZE;
	return;
}


/**
 *  @fn		Order_Print_Status_Send
 *	@brief 	发送订单打印状态报文
 *	@param	order 		订单结构体指针
						status		订单状态
 *	@ret	None
 */
void Order_Print_Status_Send ( order_info* order ,u8_t status)
{
	MesgQueBuf[MesgQueIndex].flag = ORDER_PRINT_MESG_QUE_FLAG;
	MesgQueBuf[MesgQueIndex].mesgQueueData.order_Print_Status.batch_num = order->batch_number;
	MesgQueBuf[MesgQueIndex].mesgQueueData.order_Print_Status.order_num = order->batch_within_number;
	MesgQueBuf[MesgQueIndex].mesgQueueData.order_Print_Status.data_source = order->data_source;
	MesgQueBuf[MesgQueIndex].ip = order->sever_send_time;
	MesgQueBuf[MesgQueIndex].status = status;		
	OSQPost(Mesg_Queue,&MesgQueBuf[MesgQueIndex]);
	
	MesgQueIndex = (MesgQueIndex + 1) % MESG_QUEUE_SIZE;
	return;
}


/**
 *  @fn		Printer_Status_Send
 *	@brief 	发送订单打印状态报文
 *	@param	printer_num 	打印机号
						status		打印机状态
 *	@ret	None
 */
void Printer_Status_Send(u32_t printer_num , u8_t status)
{
	MesgQueBuf[MesgQueIndex].flag = PRINTER_MESG_QUE_FLAG;
	MesgQueBuf[MesgQueIndex].mesgQueueData.printer_Status.printer_num = printer_num;
	MesgQueBuf[MesgQueIndex].ip = PRINTER_MESG_IP;
	MesgQueBuf[MesgQueIndex].status = status;		
	OSQPost(Mesg_Queue,&MesgQueBuf[MesgQueIndex]);	
	
	MesgQueIndex = (MesgQueIndex + 1) % MESG_QUEUE_SIZE;	
	return ;
}






/**
 *  @fn		Batch_Status_Mesg_Queue_Send
 *	@brief 	发送批次状态报文
 *	@param	batch_num 	
						status 			批次状态
											（BATCH_PRINT_SUCCESS ：批次打印成功 
												BATCH_ENUEUE_SUCCESS：批次进入打印队列成功）
 *	@ret	None
 */
static void Batch_Status_Mesg_Queue_Send(u16_t batch_num ,u8_t status)
{
	switch(status){
		case BATCH_PRINT_SUCCESS:{//批次成功打印报文
			BASE_SEND_STATUS(batch_status, BATCH_PRINT_SUC, (u32_t)batch_num<<16 );		
			break;
		}
		case BATCH_ENUEUE_SUCCESS:{//批次成功进入缓冲区报文
			BASE_SEND_STATUS(batch_status, BATCH_ENQUE_BUF_SUC, (u32_t)batch_num<<16 );		
			break;
		}
	}
}


/**
 *  @fn		Order_QUEUE_Status_Mesg_Queue_Send
 *	@brief 	发送订单进入打印队列状态报文
 *	@param	batch_num 	批次号（若是本地订单，批次号为0）
						order_num		批次内订单号
						status 			订单进入打印队列的状态
											（ENQUEUE_OK ：进入打印队列成功 
												ENQUEUE_ERR： 进入打印队列失败）
						data_source 订单源
						ip          Wifi 下单时的IP
 *	@ret	None
 */
static void Order_QUEUE_Status_Mesg_Queue_Send ( u16_t batch_num , u16_t order_num , u8_t status , u8_t data_source, u32_t ip )
{
		if(status == ENQUEUE_OK)
	{																	//成功进入打印队列
		switch(data_source){
			case REMOTE_SOURCE:{
				BASE_SEND_STATUS(order_status, NOR_ORDER_ENQUE_SUC, (u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case REMOTE_RE_SOURCE:{
				BASE_SEND_STATUS(order_status, EX_ORDER_ENQUE_SUC, (u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case LOCAL_SOURCE:{
				SendStatusToLocal(order_status, NOR_ORDER_ENQUE_SUC,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case LOCAL_RE_SOURCE:{
				SendStatusToLocal(order_status, EX_ORDER_ENQUE_SUC,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}		
			case WIFI_SOURCE:{
				SendStatusToWifi(order_status, NOR_ORDER_ENQUE_SUC, ip,(u32_t)batch_num<<16 | (u32_t)order_num);
				break;
			}
			case WIFI_RE_SOURCE:{			
				SendStatusToWifi(order_status, EX_ORDER_ENQUE_SUC, ip,(u32_t)batch_num<<16 |(u32_t)order_num);
			  break;
			}
		}		
	}
	else if(status == ENQUEUE_ERR)
		{																	//失败进入打印队列：数据解析错误
		switch(data_source){
			case REMOTE_SOURCE:{
				BASE_SEND_STATUS(order_status, NOR_ORDER_DATA_ERR, (u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case REMOTE_RE_SOURCE:{
				BASE_SEND_STATUS(order_status, EX_ORDER_DATA_ERR, (u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case LOCAL_SOURCE:{
				SendStatusToLocal(order_status, NOR_ORDER_DATA_ERR,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case LOCAL_RE_SOURCE:{
				SendStatusToLocal(order_status, EX_ORDER_DATA_ERR,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}		
			case WIFI_SOURCE:{
				SendStatusToWifi(order_status, NOR_ORDER_DATA_ERR, ip,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case WIFI_RE_SOURCE:{			
				SendStatusToWifi(order_status, EX_ORDER_DATA_ERR, ip,(u32_t)batch_num<<16 |(u32_t)order_num);
			  break;
			}
		}
	}
}


/**
 *  @fn		Order_Print_Status_Mesg_Queue_Send
 *	@brief 	发送订单打印状态报文
 *	@param	batch_num 	批次号
						order_num		批次内订单号
						status 			订单进打印的状态
												PRINT_STATUS_OK 		： 打印成功
											  PRINT_STATUS_ERR 		： 打印失败
											  PRINT_STATUS_START 	： 开始打印
						data_source 订单源
 *	@ret	None
 */
void Order_Print_Status_Mesg_Queue_Send ( u16_t batch_num , u16_t order_num , u8_t status , u8_t data_source ,u32_t ip)
{
	if(status == PRINT_STATUS_START)							//订单开始打印
	{
		switch(data_source){
			case REMOTE_SOURCE:{
				BASE_SEND_STATUS(order_status, NOR_ORDER_PRINT_START, (u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case REMOTE_RE_SOURCE:{
				BASE_SEND_STATUS(order_status, EX_ORDER_PRINT_START, (u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case LOCAL_SOURCE:{
				SendStatusToLocal(order_status, NOR_ORDER_PRINT_START,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case LOCAL_RE_SOURCE:{
				SendStatusToLocal(order_status, EX_ORDER_PRINT_START,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}		
			case WIFI_SOURCE:{
				SendStatusToWifi(order_status, NOR_ORDER_PRINT_START, ip,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case WIFI_RE_SOURCE:{
				SendStatusToWifi(order_status, EX_ORDER_PRINT_START, ip,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}	
		}
	}
	else if(status == PRINT_STATUS_OK)						//打印成功
	{
		switch(data_source){
			case REMOTE_SOURCE:{

				BASE_SEND_STATUS(order_status, NOR_ORDER_PRINT_SUC, (u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case REMOTE_RE_SOURCE:{
				BASE_SEND_STATUS(order_status, EX_ORDER_PRINT_SUC, (u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case LOCAL_SOURCE:{
				SendStatusToLocal(order_status, NOR_ORDER_PRINT_SUC,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case LOCAL_RE_SOURCE:{
				SendStatusToLocal(order_status, EX_ORDER_PRINT_SUC,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}	
			case WIFI_SOURCE:{
				SendStatusToWifi(order_status, NOR_ORDER_PRINT_SUC, ip,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case WIFI_RE_SOURCE:{
				SendStatusToWifi(order_status, EX_ORDER_PRINT_SUC, ip,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}			
		}
	}
	else if(status == PRINT_STATUS_DATA_ERR)			//订单打印失败，订单数据有误
	{
		switch(data_source){
			case REMOTE_SOURCE:{
				BASE_SEND_STATUS(order_status, NOR_ORDER_DATA_ERR, (u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case REMOTE_RE_SOURCE:{
				BASE_SEND_STATUS(order_status, EX_ORDER_DATA_ERR, (u32_t)batch_num<<16 |(u32_t)order_num);
				break; 
			}
			case LOCAL_SOURCE:{
				SendStatusToLocal(order_status, NOR_ORDER_DATA_ERR,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case LOCAL_RE_SOURCE:{
				SendStatusToLocal(order_status, EX_ORDER_DATA_ERR,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}		
			case WIFI_SOURCE:{
				SendStatusToWifi(order_status, NOR_ORDER_DATA_ERR, ip,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case WIFI_RE_SOURCE:{
				SendStatusToWifi(order_status, EX_ORDER_DATA_ERR, ip,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
		}
	}
	else if(status == PRINT_STATUS_MACHINE_ERR)		//订单打印失败：打印机异常
	{
		switch(data_source){
			case REMOTE_SOURCE:{
				BASE_SEND_STATUS(order_status, NOR_ORDER_PRINTER_ERR, (u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case REMOTE_RE_SOURCE:{
				BASE_SEND_STATUS(order_status, EX_ORDER_PRINTER_ERR, (u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case LOCAL_SOURCE:{
				SendStatusToLocal(order_status, NOR_ORDER_PRINTER_ERR,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case LOCAL_RE_SOURCE:{
				SendStatusToLocal(order_status, EX_ORDER_PRINTER_ERR,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}		
			case WIFI_SOURCE:{
				SendStatusToWifi(order_status, NOR_ORDER_PRINTER_ERR,ip,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}
			case WIFI_RE_SOURCE:{
				SendStatusToWifi(order_status, EX_ORDER_PRINTER_ERR,ip,(u32_t)batch_num<<16 |(u32_t)order_num);
				break;
			}	
		}
	}
	return;
}


/**
 *  @fn		Printer_Status_Mesg_Queue_Send
 *	@brief 	发送订单打印状态报文
 *	@param	printer_num 	打印机号
						status		打印机状态
 *	@ret	None
 */
void Printer_Status_Mesg_Queue_Send(u32_t printer_num , u8_t status)
{
	//status宏，就是对应的发送状态
//	DEBUG_PRINT("SEND MESG : start to send printer status \r\n");
	SendStatusToWifi(printer_status,status, PRINTER_MESG_IP,printer_num+1);
	BASE_SEND_STATUS(printer_status,status,printer_num+1);
	SendStatusToLocal(printer_status,status,printer_num+1); 	
	return ;
}


/**
 *  @fn		MesgQue_Deal
 *	@brief 	分析打印机健康状态
 *	@param			
 *	@ret	ans
 */
void MesgQue_Deal()
{
	INT8U err ;
	MesgQueueNode * receiveMesg ;
	
	while(1){
		receiveMesg = (MesgQueueNode *)OSQPend(Mesg_Queue,0,&err);
//		DEBUG_PRINT("\n order num is %d",receiveMesg->mesgQueueData.order_Print_Status.order_num);
		switch(receiveMesg->flag){
			case BATCH_MESG_QUE_FLAG:{
				Batch_Status_Mesg_Queue_Send(receiveMesg->mesgQueueData.batch_Status.batch_num,receiveMesg->status);				
				break;
			}
			case ORDER_QUEUE_MESG_QUE_FLAG:{
				Order_QUEUE_Status_Mesg_Queue_Send(receiveMesg->mesgQueueData.order_QUEUE_Status.batch_num,
																						receiveMesg->mesgQueueData.order_QUEUE_Status.order_num,
																						receiveMesg->status,
																						receiveMesg->mesgQueueData.order_QUEUE_Status.data_source,
																						receiveMesg->ip);
				break;
			}
			case ORDER_PRINT_MESG_QUE_FLAG:{
				Order_Print_Status_Mesg_Queue_Send(receiveMesg->mesgQueueData.order_Print_Status.batch_num,
																						receiveMesg->mesgQueueData.order_Print_Status.order_num,
																						receiveMesg->status,
																						receiveMesg->mesgQueueData.order_Print_Status.data_source,
																						receiveMesg->ip);
				break;
			}
			case PRINTER_MESG_QUE_FLAG:{
				Printer_Status_Mesg_Queue_Send(receiveMesg->mesgQueueData.printer_Status.printer_num,receiveMesg->status);	
					
				break;
			}		
			default:
				DEBUG_PRINT("Mesg Queue Send Judge  ERROR!!!!!\n");
		}
	}
	return;
}


/**
 *  @fn		getHealthValue
 *	@brief 	分析打印机健康状态
 *	@param	status		打印机状态
 *	@ret	ans	返回健康值：
								HEALTHY ：		健康
								SUB_HEALTHY：	亚健康
 */
u8_t getHealthValue(u32_t printer_num)
{
	u8_t value = PRINTER_HEALTH_HEALTHY;
	
	if(PCMgr.cells[printer_num].totalTime > TOTAL_WORKTIME_THRESHOLD ){		
		value = PRITNER_HEALTH_SUB_HEALTH; 
	}
	else if(PCMgr.cells[printer_num].cutCnt > TOTAL_CUT_NUM_THRESHOLD){
		value = PRITNER_HEALTH_SUB_HEALTH;
	}	
	else if(PCMgr.cells[printer_num].totalLength> TOTAL_PRINT_LEN_THRESHOLD){
		value = PRITNER_HEALTH_SUB_HEALTH;
	}	
	else if(PCMgr.cells[printer_num].workedTime > KEEP_WORKING_TIME_THRESHOLD){
		value = PRITNER_HEALTH_SUB_HEALTH;
	}
	return value;
}


/**
 *  @fn		Health_Detect_Fun
 *	@brief 	健康状态检测任务
 *	@param	
 *	@ret	ans	
 */
void Health_Detect_Fun()
{
	extern OS_EVENT * Printer_Status_Rec_Sem;
	extern  PrintCellsMgrInfo PCMgr;
	unsigned char value = 0;
	INT8U os_err;
	u32_t printer_num = 0;
	
	while(1){
		int i = 0 ;
		//延时
		OSTimeDlyHMSM(0,0,2,0);
		//轮询每台打印单元
		for( i = 0 ; i < MAX_CELL_NUM ; ++i) {
			printer_num = i;
		//状态监测
			//发送要求检测打印机状态
			if(PCMgr.cells[printer_num].status != PRINT_CELL_STATUS_BUSY){	
//				DEBUG_PRINT("Health_Detect_Fun: Sending Status Cmd to PC: %u\r\n", PCMgr.cells[printer_num].no);
				SEND_STATUS_CMD_ONE(PCMgr.cells[printer_num].no);
				//等待状态接受成功信号量。

				OSSemPend(Printer_Status_Rec_Sem,50 * 2,&os_err);		
				//判断是否超时
				if(os_err == OS_ERR_TIMEOUT){	//打印单元离线
					if(PCMgr.cells[printer_num].status == PRINT_CELL_STATUS_ERR){//打印机本来就不可用					
						continue ;//什么都不用干！~报文也不用发了
					}
					else{//打印机由正常变为异常（离线）
						if(printer_num == 0)
						{
							OSSemAccept(PCMgr.resrcSem1);
						}
						else if(printer_num == 1)
						{
							OSSemAccept(PCMgr.resrcSem2);
						}
						
						PCMgr.cells[printer_num].status = PRINT_CELL_STATUS_ERR;
						value = PRINTER_HEALTH_UNHEALTHY;//要斟酌，是否要加个离线状态？
						
//						DEBUG_PRINT("\nPrint Cell %u Off-line\r\n", i+1);
					}
				}
				else{	//打印机没有离线
					if(PCMgr.cells[printer_num].status == PRINT_CELL_STATUS_ERR ){//打印机异常	
						value = PRINTER_HEALTH_UNHEALTHY;
					}
					else{//打印机健康或者亚健康
						//根据长期状态计算健康值
						value = getHealthValue(printer_num);
					}
				}
			//反馈健康状态	
				if( PCMgr.cells[printer_num].health_status != value){				
					Printer_Status_Send(printer_num,value);
					PCMgr.cells[printer_num].health_status = value;
				}
			}
		}
	}
	return ;
}
