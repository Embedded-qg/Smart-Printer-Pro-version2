#include "RequestHeap.h"

/* 交换请求订单 */
static void swapReq(Request *a, Request *b);

/* 调整小顶堆 */
static void AdjustHeap(Heap *heap, u8_t pos);

/* 在删除堆顶后下沉替换最后一个元素的堆顶 */
void DownHeap(Heap *heap, u8_t index);

/* 交换请求订单 */
void swapReq(Request *a, Request *b)
{
	Request temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

/* 插入小顶堆 */
void InsertHeap(Heap *heap, Request *req)
{
	heap->req[++heap->count] = *req;//不使用节点0
	AdjustHeap(heap, heap->count);	//调整
}

/* 删除堆顶 */
void DeleteHeap(Heap *heap)
{
	heap->req[1] = heap->req[heap->count--];
	DownHeap(heap, 1);	//向下调整
}

/* 调整小顶堆 */
void AdjustHeap(Heap *heap, u8_t pos)
{
	if(pos > 1){
		u8_t parent = pos / 2;	//求出父节点
		
		if(heap->req[parent].recTime > heap->req[pos].recTime){
			swapReq(&(heap->req[parent]), &(heap->req[pos])); //交换
			AdjustHeap(heap, parent);	//递归调用
		}
	}
}

/* 在删除堆顶后下沉替换最后一个元素的堆顶 */
void DownHeap(Heap *heap, u8_t index)
{
	u8_t count = heap->count;
	u8_t child = 2 * index;	//若有，则为其左孩子节点
	
	if(index <= count){	//拥有左右节点才调整
		if(2 * index < count){ //左右儿子都有
			if(heap->req[index].recTime > heap->req[index + 1].recTime){
				++child;
			}
		}
		
		if(heap->req[index].recTime > heap->req[child].recTime){
			swapReq(&(heap->req[index]), &(heap->req[child])); //交换
			DownHeap(heap, child);	//递归调用
		}
	}
}

/* 更新堆结构 */
void UpdateHeap(Heap *heap)
{
	if(heap->count > 0){
		heap->flag = WAIT_FOR_BUF;
	}
}

/* 初始化堆结构 */
void InitHeap(Heap *heap)
{
	heap->count = 0;
	heap->flag = BUF_CAP_FULFILL;
	heap->capcity = MAX_CAPCITY;
}

#define WAIT(heap) ((heap->req[1].orderLen) > (heap->capcity))	//等待缓冲区容量充足

void DealWifiReq(Heap *heap)
{
	INT8U err;
	extern OS_EVENT *Capacity_Change_Sem;	//wifi缓冲区容量改变信号
	extern OS_EVENT *Rec_Wifi_Req_Sem;		//获取到wifi订单请求信号
	InitHeap(heap);
	while(1){
		DEBUG_PRINT("-------heap pend heapSem mutex!-------\n");
		OSSemPend(Rec_Wifi_Req_Sem, 0, &err);
		
		/* 判断缓冲区容量是否充足 */
		DEBUG_PRINT("-------heap pend heap mutex!-------\n");
		OSMutexPend(heap->mutex, 0, &err);	//申请堆控制锁
		{
			if(heap->count > 0){
				DEBUG_PRINT("-------------before wait, req[1].orderlen is %lu, heap capcity is %lu---------\n", 
							heap->req[1].orderLen, heap->capcity);
				while(WAIT(heap)){	//等待缓冲区容量充足
					DEBUG_PRINT("-------------req[1].orderlen is %lu, heap capcity is %lu---------\n",
							heap->req[1].orderLen, heap->capcity);
					DEBUG_PRINT("-------heap post heap mutex in wait!-------\n");
					OSMutexPost(heap->mutex);			//释放堆控制锁
					DEBUG_PRINT("-------heap pend capcitySem mutex in wait!-------\n");
					OSSemPend(Capacity_Change_Sem, 0, &err);
					DEBUG_PRINT("-------heap pend heap mutex in wait!-------\n");
					OSMutexPend(heap->mutex, 0, &err);	//申请堆控制锁
				}
			}
			
			SendStatusToWifi(wifi_order_req, 0, heap->req[1].ipAdd, 0);	//反馈应答至wifi端，视为可下单
			heap->capcity -= heap->req[1].orderLen;
			DEBUG_PRINT("-------------After rec, heap capcity is %lu---------\n", heap->capcity);
			DeleteHeap(heap);	//删除堆顶
		}
		DEBUG_PRINT("-------heap post heap mutex in wait!-------\n");
		OSMutexPost(heap->mutex);	//释放堆控制锁
	}
}