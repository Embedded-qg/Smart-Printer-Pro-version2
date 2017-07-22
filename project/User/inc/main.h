#ifndef MAIN_H
#define MAIN_H

#include "os_cpu.h"
#include "ucos_ii.h"
#include "lwip/err.h"
/* Extern variables ---------------------------------------------------------*/

//#define ORDER_PRINT_COMPLETE     1
//#define ORDER_PRINT_ERR          3
//#define ORDER_ENTER_BUF          4
//#define ORDER_ENTER_QUEUE        5
//#define ORDER_START_PRINT        6

extern struct tcp_pcb *pcb_global;

extern OS_MEM *queue_1K;
extern OS_MEM *queue_2K;
extern OS_MEM *queue_4K;
extern OS_MEM *queue_10K;

extern OS_EVENT *Block_1K_Sem;             //用于请求1K空闲任务块
extern OS_EVENT *Block_2K_Sem;							//用于请求2K空闲任务块
extern OS_EVENT *Block_4K_Sem;             //用于请求4K空闲任务块
extern OS_EVENT *Block_10K_Sem;             //用于请求10K空闲任务块
extern OS_EVENT *Com_Buf_Idle_Mutex;         //标记普通缓冲区剩余容量大于75%

extern unsigned char step;
extern unsigned char num_order;
extern unsigned char status_order;

extern err_t Post_connected(void *arg,struct tcp_pcb *pcb,err_t err);
extern void Post_init(void);

#endif 
