/********************   (C) COPYRIGHT 2013 www.armjishu.com   ********************
 * 文件名  ：SZ_STM32F107VC_LIB.h
 * 描述    ：提供STM32F107VC神舟IV号开发板UCOSII操作系统
 * 实验平台：STM32神舟开发板
 * 作者    ：www.armjishu.com 
**********************************************************************************/

#include "stm32f2xx.h"
#include "cc.h"
#include "main.h"
#include "printerException.h"
#include "print_cells.h"
#include "app_cfg.h"
#include "local_conf.h"
#include "wifi_conf.h"
#include "RequestHeap.h"


#define GET_STATUS_01 putchar(0x10);putchar(0x04);putchar(0x01);
#define GET_STATUS_02 putchar(0x10);putchar(0x04);putchar(0x02);
#define GET_STATUS_03 putchar(0x10);putchar(0x04);putchar(0x03);
#define GET_STATUS_04 putchar(0x10);putchar(0x04);putchar(0x04);

/******************************************USART,DMA related Area******************/
void USART1_Hook(void);
void UART4_Hook(void);
//void DMA_To_USARTx_Send(uint8_t *SendBuff, uint16_t size, uint8_t deviceNum);
//void Mem_To_USART1_DMA_Config(uint8_t *SendBuff, uint16_t size);
//void Mem_To_USART2_DMA_Config(uint8_t *SendBuff, uint16_t size);
/******************************************USART,DMA related Area******************/

/* Private function prototypes -----------------------------------------------*/
static  OS_STK LWIP_TaskStartStk[LWIP_TASK_START_STK_SIZE];
static  OS_STK PRINT_TaskStk[PRINT_TASK_STK_SIZE];
static  OS_STK REQ_BATCH_TaskStk[REQ_BATCH_TASK_STK_SIZE];
static  OS_STK PRINT_QUEUE_TaskStk[PRINT_QUEUE_TASK_STK_SIZE];
static  OS_STK HEALTH_DETECT_TaskStk[HEALTH_DETECT_TASK_STK_SIZE];
static  OS_STK LOCAL_REC_TaskStk[LOCAL_REC_STK_SIZE];
static  OS_STK WIFI_REC_TaskStk[WIFI_REC_STK_SIZE];
static  OS_STK WIFI_REC_Req_TaskStk[WIFI_REC_REQ_STK_SIZE];
static  OS_STK MESG_QUE_TaskStk[MESG_QUE_STK_SIZE];

// 打印传输单元线程，负责实际的订单传输工作
static  OS_STK TRANSMITTER_TaskStk[MAX_CELL_NUM][TRANSMITTER_STK_SIZE];

/* Private Functions ---------------------------------------------------------*/
static  void Lwip_TaskStart(void *p_arg);
static  void Print_Task(void *p_arg); 
static  void Print_Queue_Task(void *p_arg);
static  void Health_Detect_Task(void * p_arg );
static  void Recv_Batch_Task(void* p_arg);
static  void Local_Rec_Task(void* p_arg);
static  void Wifi_Rec_Task(void* p_arg);
static  void Mesg_Queue_Task(void * p_arg);
static  void Transmit_Task(void * p_arg);
static  void Deal_Wifi_Req_Task(void* p_arg);

/* Public Functions ---------------------------------------------------------*/
extern void System_Setup(void);

/* Pulic variables ---------------------------------------------------------*/
unsigned int system_tick_num = 0;

/* siganl for task syn ---------------------------------------------------------*/
OS_EVENT *Recon_To_Server_Sem;		//标记需要与服务器重新连接
OS_EVENT *Print_Sem;                //用于阻塞打印任务的信号
OS_EVENT *Print_Queue_Sem;          //用于阻塞构建打印队列的任务
OS_EVENT *Block_1K_Sem;             //用于请求1K空闲任务块
OS_EVENT *Block_2K_Sem;				//用于请求2K空闲任务块
OS_EVENT *Block_4K_Sem;             //用于请求4K空闲任务块
OS_EVENT *Block_10K_Sem;            //用于请求10K空闲任务块

OS_EVENT *Batch_Rec_Sem;			//完成一次批次读取的二值信号量
OS_EVENT *Ack_Rec_Order_Sem;		//本地接收对订单状态应答的信号量
OS_EVENT *Ack_Rec_Printer_Sem;		//本地接收对打印机状态应答的信号量
OS_EVENT *Local_Send_Data_Mutex;	//本地发送状态锁
OS_EVENT *Wifi_Send_Data_Mutex;		//wifi发送状态锁
OS_EVENT *Printer_Status_Rec_Sem;	//打印机状态反馈接收信号
OS_EVENT *Print_Done_Sem;			//订单打印完成信号量
OS_EVENT *Local_Rec_Data_Sem;		//本地缓冲区接收到数据
OS_EVENT *Wifi_Rec_Data_Sem;		//Wifi缓冲区接收到数据
OS_EVENT *Capacity_Change_Sem;		//wifi缓冲区容量改变信号
OS_EVENT *Rec_Wifi_Req_Sem;			//获取到wifi订单请求信号

OS_EVENT *Mesg_Queue;				//发送状态消息队列信号

INT8U block_1K[BLOCK_1K_NUM][BLOCK_1K_SIZE];   //1K内存块打印队列
INT8U block_2K[BLOCK_2K_NUM][BLOCK_2K_SIZE];   //2K内存块打印队列
INT8U block_4K[BLOCK_4K_NUM][BLOCK_4K_SIZE];   //4K内存块打印队列
INT8U block_10K[BLOCK_10K_NUM][BLOCK_10K_SIZE];//10K内存块打印队列

Heap heap;//订单请求堆控制结构

OS_MEM *queue_1K;
OS_MEM *queue_2K;
OS_MEM *queue_4K;
OS_MEM *queue_10K;

unsigned char step = 0;
unsigned char num_order = 0;
unsigned char status_order = 0;

#define assert(expr, str)			 	\
	do {								\
		if(!(expr)) {					\
			printf((str));				\
			while(1);					\
		}								\
	}while(0)
	
void InitSemAndMutex(void)
{
	INT8U	 err;
	/*创建信号量*/
	assert((Local_Send_Data_Mutex = OSMutexCreate(LOCAL_SEND_DATA_MUTEX_PRIO, &err)), "CREATE Local_Send_Data_Sem FAILED\n");
	assert((Wifi_Send_Data_Mutex = OSMutexCreate(WIFI_SEND_DATA_MUTEX_PRIO, &err)), "CREATE Wifi_Send_Data_Sem FAILED\n");
	
	assert((Recon_To_Server_Sem = OSSemCreate(0)), "CREATE Recon_To_Server_Sem FAILED\n");
	assert((Print_Sem = OSSemCreate(0)), "CREATE Print_Sem FAILED\n");
	assert((Print_Queue_Sem	= OSSemCreate(0)), "CREATE Print_Queue_Sem FAILED\n");
	assert((Batch_Rec_Sem = OSSemCreate(0)), "CREATE Batch_Rec_Sem FAILED\n");
	assert((Ack_Rec_Order_Sem = OSSemCreate(0)), "CREATE Ack_Rec_Order_Sem FAILED\n");
	assert((Ack_Rec_Printer_Sem = OSSemCreate(0)), "CREATE Ack_Rec_Printer_Sem FAILED\n");
	assert((Printer_Status_Rec_Sem = OSSemCreate(0)), "CREATE Printer_Status_Rec_Sem FAILED\n");
	assert((Print_Done_Sem  = OSSemCreate(0)), "CREATE Print_Done_Sem FAILED\n");
	assert((Local_Rec_Data_Sem = OSSemCreate(0)), "CREATE Local_Rec_Data_Sem FAILED\n");
	assert((Wifi_Rec_Data_Sem = OSSemCreate(0)), "CREATE Wifi_Rec_Data_Sem FAILED\n");
	assert((Capacity_Change_Sem = OSSemCreate(0)), "CREATE Capacity_Change_Sem FAILED\n");
	assert((Rec_Wifi_Req_Sem = OSSemCreate(0)), "CREATE Rec_Wifi_Req_Sem FAILED\n");
	
	assert((Mesg_Queue = OSQCreate(MesgQueArray, MESG_QUEUE_SIZE)), "CREATE Mesg_Queue FAILED\n");
	
	assert((Block_1K_Sem = OSSemCreate(BLOCK_1K_NUM)), "CREATE 1K SEM FAILED\n");   
	assert((Block_2K_Sem = OSSemCreate(BLOCK_2K_NUM)), "CREATE 2K SEM FAILED\n"); 
	assert((Block_4K_Sem = OSSemCreate(BLOCK_4K_NUM)), "CREATE 4K SEM FAILED\n"); 	
	assert((Block_10K_Sem = OSSemCreate(BLOCK_10K_NUM)), "CREATE 10K SEM FAILED\n");
}
	
int main(void)
{
	u8_t os_err;
	u8_t i;

	System_Setup();    	//系统初始化
	OSInit();			/* UCOSII 操作系统初始化 */
	InitSemAndMutex();	//初始化信号量
	 

	/*任务功能：LWIP初始化*/
	os_err = OSTaskCreate((void (*) (void *)) Lwip_TaskStart,               		 //指向任务代码的指针
                          (void *) 0,												 //任务开始执行时，传递给任务的参数的指针
               		      (OS_STK *) &LWIP_TaskStartStk[LWIP_TASK_START_STK_SIZE - 1],//分配给任务的堆栈的栈顶指针   从顶向下递减
                          (INT8U) LWIP_TASK_START_PRIO);								 //分配给任务的优先级		

	/*任务功能：建立订单打印*/
	os_err = OSTaskCreate((void (*) (void *))Print_Task,               		    //指向任务代码的指针
                          (void *) 0,											//任务开始执行时，传递给任务的参数的指针
               		      (OS_STK *) &PRINT_TaskStk[PRINT_TASK_STK_SIZE - 1],	//分配给任务的堆栈的栈顶指针   从顶向下递减
                          (INT8U) PRINT_TASK_PRIO);								//分配给任务的优先级															
					
	/*任务功能：建立订次接收*/
	os_err = OSTaskCreate((void (*) (void *))Recv_Batch_Task,               		//指向任务代码的指针
                          (void *) 0,												//任务开始执行时，传递给任务的参数的指针
               		      (OS_STK *) &REQ_BATCH_TaskStk[REQ_BATCH_TASK_STK_SIZE - 1],//分配给任务的堆栈的栈顶指针   从顶向下递减
                          (INT8U) REQ_BATCH_TASK_PRIO);								//分配给任务的优先级	
	
	/*任务功能：构建打印队列*/
	os_err = OSTaskCreate((void (*) (void *))Print_Queue_Task,               		    //指向任务代码的指针
                          (void *) 0,													//任务开始执行时，传递给任务的参数的指针
               		      (OS_STK *) &PRINT_QUEUE_TaskStk[PRINT_QUEUE_TASK_STK_SIZE - 1],//分配给任务的堆栈的栈顶指针   从顶向下递减
                          (INT8U) PRINT_QUEUE_TASK_PRIO);								//分配给任务的优先级	
						  
	/*任务功能：构建本地接收数据*/
	os_err = OSTaskCreate((void (*) (void *))Local_Rec_Task,               		    //指向任务代码的指针
                          (void *) 0,												//任务开始执行时，传递给任务的参数的指针
               		      (OS_STK *) &LOCAL_REC_TaskStk[LOCAL_REC_STK_SIZE - 1],		//分配给任务的堆栈的栈顶指针   从顶向下递减
                          (INT8U) LOCAL_REC_TASK_PRIO);								//分配给任务的优先级	
						  
	/*任务功能：构建本地接收数据*/
	os_err = OSTaskCreate((void (*) (void *))Wifi_Rec_Task,               		    //指向任务代码的指针
                          (void *) 0,												//任务开始执行时，传递给任务的参数的指针
               		      (OS_STK *) &WIFI_REC_TaskStk[WIFI_REC_STK_SIZE - 1],		//分配给任务的堆栈的栈顶指针   从顶向下递减
                          (INT8U) WIFI_REC_TASK_PRIO);								//分配给任务的优先级	
													
	/*任务功能：构建健康监测线程*/				
	os_err = OSTaskCreate((void (*) (void *))Health_Detect_Task,					//指向任务代码的指针
						  (void *) 0,												//任务开始执行时，传递给任务的参数的指针	
						  (OS_STK *) &HEALTH_DETECT_TaskStk[HEALTH_DETECT_TASK_STK_SIZE - 1],//分配给任务的堆栈的栈顶指针   从顶向下递减
						  (INT8U) HEALTH_DETECT_TASK_PRIO);//分配给任务的优先级	
	
	/*任务功能：发送消息队列内容*/
	os_err = OSTaskCreate((void (*) (void *))Mesg_Queue_Task,               		//指向任务代码的指针
                          (void *) 0,												//任务开始执行时，传递给任务的参数的指针
               		      (OS_STK *) &MESG_QUE_TaskStk[MESG_QUE_STK_SIZE - 1],		//分配给任务的堆栈的栈顶指针   从顶向下递减
                          (INT8U) MESG_QUE_TASK_PRIO);								//分配给任务的优先级	
	
	/*任务功能：发送消息队列内容*/
	os_err = OSTaskCreate((void (*) (void *))Deal_Wifi_Req_Task,               		//指向任务代码的指针
                          (void *) 0,												//任务开始执行时，传递给任务的参数的指针
               		      (OS_STK *) &WIFI_REC_Req_TaskStk[WIFI_REC_REQ_STK_SIZE - 1],		//分配给任务的堆栈的栈顶指针   从顶向下递减
                          (INT8U) WIFI_REC_REQ_TASK_PRIO);								//分配给任务的优先级	
	
	// 根据打印单元数目设置相应数目的传输线程
	for(i = 0; i < MAX_CELL_NUM; i++) 
	{
		/*任务功能：发送消息队列内容*/
		os_err = OSTaskCreate((void (*) (void *))Transmit_Task,               					//指向任务代码的指针
							  (void *)(PRINT_CELL_NUM_ONE + i),														//任务开始执行时，传递给任务的参数的指针
							  (OS_STK *) &TRANSMITTER_TaskStk[i][TRANSMITTER_STK_SIZE - 1],		//分配给任务的堆栈的栈顶指针   从顶向下递减
							  (INT8U) TRANSMITTER_TASK_PRIO + i);									//分配给任务的优先级	
	}		
						  
	OSTimeSet(0);
	
	InitPrintCellsMgr();	//初始化打印单元管理器
						  
	InitRestoreOrderQueue();// 初始化订单恢复队列
						  
	Init_Queue();          //初始化缓冲区
													
	Init_Order_Table();    //初始化打印队列订单信息表
	
	ReadPrintCellsInfo();	// 读取片内ROM，恢复打印单元的属性值
						
//	ClearEEPROM();
						  
	/* 新建内存块 */
					
	assert((	queue_1K = OSMemCreate(block_1K,BLOCK_1K_NUM,BLOCK_1K_SIZE,&os_err)), "CREATE 1K MEM FAILED\n"); 
	assert((	queue_2K = OSMemCreate(block_2K,BLOCK_2K_NUM,BLOCK_2K_SIZE,&os_err)), "CREATE 2K MEM FAILED\n"); 
	assert((	queue_4K = OSMemCreate(block_4K,BLOCK_4K_NUM,BLOCK_4K_SIZE,&os_err)), "CREATE 4K MEM FAILED\n"); 
	assert((	queue_10K = OSMemCreate(block_10K,BLOCK_10K_NUM,BLOCK_10K_SIZE,&os_err)), "CREATE 10K MEM FAILED\n"); 
	OSStart();	 	/* 启动UCOSII操作系统 */

	while(1);  //一般不会到这里，到这里表明错误

}


static void Lwip_TaskStart(void* p_arg)
{
	INT8U err;
	extern struct netconn *order_netconn;	//全局TCP链接
	(void) p_arg;                          
#if (OS_TASK_STAT_EN > 0)   //若使能ucos 的统计任务
	OSStatInit(); 	//----统计任务初始化函数                                 
#endif
	
	LwIP_Init();
	while(1)
	{
		con_to_server();
		OSSemPend(Recon_To_Server_Sem, 0, &err);
	}
}


/****************************************************************************
* 名    称：u32_t sys_now(void)
* 功    能：时钟函数，用于获取系统时钟滴答，20ms时间system_tick_num加一
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
u32_t sys_now(void)
{
	return system_tick_num;
}



/****************************************************************************
* 名    称：static  void Print_Task(void* p_arg)
* 功    能：打印处于打印队列头的订单，并在完成后将其移出队列和释放空间
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static  void Print_Task(void* p_arg)
{
	INT8U err;
	u8_t entry;
	order_info *orderp;
	(void) p_arg;

	while(1)
	{
		DEBUG_PRINT("Print_Task: ORDER  WAITING\n");			
		OSSemPend(Print_Sem, 0, &err);		
		DEBUG_PRINT("Print_Task: ORDER  GET\n");
		
		if(GetRestoredOrder(&entry) == 1 || GetOrderFromQueue(&entry) ==  ORDER_QUEUE_OK) {	// 成功获取订单
			if(CheckOrderData(entry) == ORDER_DATA_OK) {	// 订单数据正确，下发打印任务
				DEBUG_PRINT("Print_Task: ORDER DATA CHECK OK\n");
				DispensePrintJob(entry);	
			}else {		// 订单数据错误，丢弃订单
				Delete_Order(entry);
			}
		}else {
			DEBUG_PRINT("Print_Task: SEM BUG: Print_Task: None Print Order.\n");
		}
	}
}

/****************************************************************************
* 名    称static  void Recv_Batch_Task(void* p_arg)
* 功    能：订单接收任务，当缓冲区满足一定条件时
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static  void Recv_Batch_Task(void* p_arg)
{
	extern struct netconn *order_netconn;	//全局TCP链接

	(void) p_arg;                          

	while(1)
	{	
		if(order_netconn != NULL)
		{
			receive_connection(order_netconn);
#if APP_DEBUG
		//	write_connection(order_netconn, first_req, REQ_LINK_OK, 0);//初次请求建立
#endif
		}else{
			OSTimeDlyHMSM(0, 0, 1, 0);
		}

	}
}

/****************************************************************************
* 名    称static  void Wifi_Rec_Task(void* p_arg)
* 功    能：本地接收订单及应答任务
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static  void Wifi_Rec_Task(void* p_arg)
{
	(void) p_arg; 
	
	while(1)
	{
		wifi_receive();
	}
}


/****************************************************************************
* 名    称static  void Local_Rec_Task(void* p_arg)
* 功    能：本地接收订单及应答任务
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static  void Local_Rec_Task(void* p_arg)
{
	(void) p_arg; 
	
	while(1)
	{
		local_receive();
	}
}

/****************************************************************************
* 名    称static  void Deal_Wifi_Req_Task(void* p_arg)
* 功    能：本地接收订单及应答任务
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static  void Deal_Wifi_Req_Task(void* p_arg)
{
	(void) p_arg; 
	
	while(1)
	{
		DealWifiReq(&heap);
	}
}

/****************************************************************************
* 名    称static  void Print_Queue_Task(void* p_arg)
* 功    能：申请内存块，构建打印队列
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static  void Print_Queue_Task(void* p_arg)
{
	Print_Queue_Fun();
}

/****************************************************************************
* 名    称static  void Health_Detect_Task(void* p_arg)
* 功    能：打印机状态监测任务
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static  void Health_Detect_Task(void * p_arg )
{
	Health_Detect_Fun();
	return ;
}

/****************************************************************************
* 名    称static  void Mesg_Queue_Task(void* p_arg)
* 功    能：完成消息队列内容发送
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static void Mesg_Queue_Task(void * p_arg)
{
	MesgQue_Deal();
	return ;
}


/****************************************************************************
* 名    称static  void Transmit_Task(void* p_arg)
* 功    能：负责打印单元的实际传输工作
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static void Transmit_Task(void * p_arg)
{
	u8_t cellno = (u8_t)p_arg;
	PrintCellInfo *cellp = &PCMgr.cells[cellno-1];
	u8_t err;
	
	while(1) 
	{
		OSSemPend(cellp->printBeginSem, 0, &err);
		Print_Order(cellno);
	}
	
	return ;
}