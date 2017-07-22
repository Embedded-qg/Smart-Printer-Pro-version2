#ifndef APP_CFG_H
#define APP_CFG_H
/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/
#define  MESG_QUE_TASK_PRIO       	 4
#define  HEALTH_DETECT_TASK_PRIO	 5
#define  LWIP_TASK_START_PRIO        3
#define  PRINT_TASK_PRIO             9
#define  LOCAL_REC_TASK_PRIO       	 8
#define  WIFI_REC_REQ_TASK_PRIO      6
#define  WIFI_REC_TASK_PRIO       	 7
#define  REQ_BATCH_TASK_PRIO         10
#define  PRINT_QUEUE_TASK_PRIO       11
// 优先级区间[TRANSMITTER_TASK_PRIO, TRANSMITTER_TASK_PRIO + MAX_CELL_NUM]被传输线程占用
#define  TRANSMITTER_TASK_PRIO		 12
#define  MIN_TASK_PRIO 				 15	//最低任务优先级
#define	 LOCAL_SEND_DATA_MUTEX_PRIO	 16
#define  WIFI_SEND_DATA_MUTEX_PRIO	 17
#define  URGENT_BUF_MUTEX_PRIO		 18
#define  QUEUE_BUF_MUTEX_PRIO		 19
#define  ORDER_PRINT_TABLE_MUTEX_PRIO 20
/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/
#define  LWIP_TASK_START_STK_SIZE    	512
#define  PRINT_TASK_STK_SIZE         	256
#define  REQ_BATCH_TASK_STK_SIZE     	256
#define  PRINT_QUEUE_TASK_STK_SIZE   	256
#define  HEALTH_DETECT_TASK_STK_SIZE  	256
#define  LOCAL_REC_STK_SIZE   		 	256
#define  WIFI_REC_STK_SIZE   		 	256
#define  WIFI_REC_REQ_STK_SIZE   		256
#define  MESG_QUE_STK_SIZE   		 	256
#define  TRANSMITTER_STK_SIZE   		256

#define APP_DEBUG 0

#define DEBUG_PRINT_ON 0

#if DEBUG_PRINT_ON
	#define DEBUG_PRINT(fmt,args...) printf (fmt ,##args)
#else
	#define DEBUG_PRINT(fmt,args...)
#endif

#endif

