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
#define  WIFI_REC_TASK_PRIO       	 16
#define  REQ_BATCH_TASK_PRIO         10
#define  PRINT_QUEUE_TASK_PRIO       11
#define  UDP_TASK_PRIO                7
//#define  TCP_TASK_PRIO                13
// 优先级区间[TRANSMITTER_TASK_PRIO, TRANSMITTER_TASK_PRIO + MAX_CELL_NUM]被传输线程占用
#define  TRANSMITTER_TASK_PRIO		 12
#define  MIN_TASK_PRIO 				 17	//最低任务优先级
#define	 LOCAL_SEND_DATA_MUTEX_PRIO	 18
#define  WIFI_SEND_DATA_MUTEX_PRIO	 19
#define  URGENT_BUF_MUTEX_PRIO		 20
#define  QUEUE_BUF_MUTEX_PRIO		 21
#define  ORDER_PRINT_TABLE_MUTEX_PRIO 22
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
#define  UDP_STK_SIZE 256
#define  TCP_STK_SIZE 256

#define APP_DEBUG 0
#define TIME_INTERVAL 20//时间间隔
#define DEBUG_PRINT_ON 0
#define DEBUG_PRINT_TIMME_ON 0
#define DEBUG_PRINT_STATEGY_ON 0

#define GRADE_SYSTEM_ON 1

#if DEBUG_PRINT_STATEGY_ON 
	#define DEBUG_PRINT_STATEGY(fmt,args...) printf (fmt ,##args)
#else
	#define DEBUG_PRINT_STATEGY(fmt,args...)
#endif

#if DEBUG_PRINT_TIMME_ON 
	#define DEBUG_PRINT_TIMME(fmt,args...) printf (fmt ,##args)
#else
	#define DEBUG_PRINT_TIMME(fmt,args...)
#endif

#if DEBUG_PRINT_ON
	#define DEBUG_PRINT(fmt,args...) printf (fmt ,##args)
#else
	#define DEBUG_PRINT(fmt,args...)
#endif

#endif

