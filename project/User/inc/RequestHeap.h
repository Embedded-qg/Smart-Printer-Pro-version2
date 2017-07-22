#ifndef REQUEST_HEAP_H
#define REQUEST_HEAP_H

/**************************************************************
*	Include File Section
**************************************************************/
#include "cc.h"
#include "more_infomation.h"
#include "data_form.h"
#include "analyze_data.h"

/* 订单请求结构体 */
typedef struct request{
	u32_t orderLen;	//请求订单长度
	u32_t recTime;	//订单生成时间
	u32_t ipAdd;	//移动端ip地址
}Request;


/* 小顶堆结构体 */
typedef struct heap{
	u8_t flag;		//缓冲区满足订单容量标志
	u8_t count;		//请求数目
	u32_t capcity;	//当前缓冲区剩余容量
	struct os_event *mutex;	//堆结构控制互斥锁
#define REQ_MAX_SIZE 50
	Request req[REQ_MAX_SIZE];	//订单请求结构体数组
}Heap;



/**************************************************************
*	Macro Define Section
**************************************************************/
#define WAIT_FOR_BUF 		0x01
#define BUF_CAP_FULFILL  	0x00
#define MAX_CAPCITY			(1024 * 10)

/**************************************************************
*	Function Define Section
**************************************************************/
/* 插入小顶堆 */
void InsertHeap(Heap *heap, Request *req);

/* 更新堆结构 */
void UpdateHeap(Heap *heap);

/* 删除堆顶 */
void DeleteHeap(Heap *heap);

/* 初始化堆结构 */
void InitHeap(Heap *heap);

/* 处理wifi订单请求 */
void DealWifiReq(Heap *heap);
#endif //REQUEST_HEAP_H~