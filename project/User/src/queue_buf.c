
#include "queue_buf.h"
#include "print_queue.h"

ElemType Qbuf[MAXQSIZE];        //循环缓冲区的建立
ElemType Ubuf[MAXUSIZE];        //加急缓冲区的建立
ElemType UsBuf[MAXUSIZE];		//串口缓冲区的建立
#define KB (1024)
ElemType UsWifiBuf[MAXUSIZE + KB * 1];	//wifi缓冲区的建立

SqQueue queue_buf;       //循环缓冲区
SqQueue urgent_buf;      //加急缓冲区
SqQueue usart_buf;		 //串口缓冲区
SqQueue wifi_buf;		 //wifi缓冲区

#define assert(expr, str)			 	\
	do {								\
		if(!(expr)) {					\
			printf((str));				\
			while(1);					\
		}								\
	}while(0)
	
/****************************************************************************
* 名    称：void Init_Queue(void)
* 功    能：初始化循环缓冲区：读、写指针指向起点，设循环缓冲为空
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void Init_Queue(void)
{
	INT8U err;

	assert((queue_buf.mutex = OSMutexCreate(QUEUE_BUF_MUTEX_PRIO, &err)), "CREATE queue_buf.mutex FAILED\n");
	queue_buf.base = Qbuf;
	queue_buf.read = 0;        //指向需要读取的位置
	queue_buf.write = 0;       //指向需要写入的位置
	queue_buf.MAX = MAXQSIZE;  //将最大容量填进MAX字段中
	queue_buf.buf_empty = 1;   //缓冲区为空
	
	assert((urgent_buf.mutex = OSMutexCreate(URGENT_BUF_MUTEX_PRIO, &err)), "CREATE urgent_buf.mutex FAILED\n");
	urgent_buf.base = Ubuf; 
	urgent_buf.read = 0;         //指向需要读取的位置
	urgent_buf.write =0;         //指向需要写入的位置
	urgent_buf.MAX = MAXUSIZE;   //将最大容量填进MAX字段中
	urgent_buf.buf_empty = 1;    //缓冲区为空
	
	usart_buf.base = UsBuf;
	usart_buf.read = 0;         //指向需要读取的位置
	usart_buf.write =0;         //指向需要写入的位置
	usart_buf.MAX = MAXUSIZE;   //将最大容量填进MAX字段中
	usart_buf.buf_empty = 1;    //缓冲区为空
	
	wifi_buf.base = UsWifiBuf;
	wifi_buf.read = 0;         //指向需要读取的位置
	wifi_buf.write =0;         //指向需要写入的位置
	wifi_buf.MAX = MAXUSIZE + KB * 1;//将最大容量填进MAX字段中
	wifi_buf.buf_empty = 1;    //缓冲区为空
	
}

/****************************************************************************
* 名    称：s8_t Write_Buf(SqQueue *buf,ElemType *e,u32_t len)
* 功    能：向缓冲区写入数据
* 入口参数：@e : 指向待写入数据的指针 ； @len : 写入的数据量 
* 出口参数：写入的状态
* 说    明：
* 调用方法：无 
****************************************************************************/
s8_t Write_Buf(SqQueue *buf,ElemType *e,u32_t len)
{
#if OS_CRITICAL_METHOD == 3
		OS_CPU_SR cpu_sr;
#endif
	INT8U err;
	s8_t buf_err = BUF_OK;
	
	DEBUG_PRINT("-----------------------ready to pend a mutex of buf\n");
	OSMutexPend(buf->mutex,0,&err);			//申请缓冲锁
	DEBUG_PRINT("-----------------------get a mutex of buf\n");
	
	if(len > buf->MAX  || len == 0)            //检查参数的正确性
		buf_err = BUF_ARG_ERR;
	 
	//检查是否发生环形缓冲溢出
	if(buf->write > buf->read){
		if(len > (buf->MAX - (buf->write - buf->read)))          
			buf_err = BUF_FULL;
	}
	else if(buf->write < buf->read){
		if(len > (buf->read - buf->write))          
			buf_err = BUF_FULL;
	}
	else if((buf->write == buf->read)&&(buf->buf_empty == 0)){
		buf_err = BUF_FULL;
	}

	if(buf_err != BUF_OK){
		OSMutexPost(buf->mutex);
		return buf_err;
	}
	
	//更新打印队列订单信息表
	
	order_print_table.buf_node.common_buf_remain_capacity = order_print_table.buf_node.common_buf_remain_capacity - len;
	
	
//	printf("Revieve Buf Start \n");
	while(len-- > 0){           //插入队列
		buf->base[buf->write] = *e++;
//		printf("%x ", buf->base [buf->write]);
		buf->write = (buf->write+1)%buf->MAX;
	}
//	printf("Revieve Buf End \n");
	buf->buf_empty = 0;    //缓冲不为空

	OSMutexPost(buf->mutex);
	
	return BUF_OK;
}


/****************************************************************************
* 名    称：s8_t Read_Order_Length_Queue(u16_t *order_len)
* 功    能：返回订单长度
* 入口参数：@e : 返回的订单长度
* 出口参数：获取订单长度是否成功
* 说    明：
* 调用方法：无 
****************************************************************************/
s8_t Read_Order_Length_Queue(SqQueue buf,u16_t *order_len)
{
	if(!buf.buf_empty)
	{
		*order_len = buf.base[(buf.read + ORDER_SIZE_OFFSET)%buf.MAX] << 8 | buf.base[(buf.read + ORDER_SIZE_OFFSET + 1)%buf.MAX] ;    //返回订单长度 
		return Que_NOT_EMPTY;  
	}	
	else
		return Que_EMPTY;                   //缓冲为空
}


/****************************************************************************
* 名    称：s8_t Check_Buf_Request_Signal(SqQueue buf)
* 功    能：判断缓冲区容量，确定是否需要发送请求信号
* 入口参数：@e : 返回 BUFF_SIZE_Aachieve
								 返回 
* 出口参数：获取订单长度是否成功
* 说    明：
* 调用方法：无 
****************************************************************************/
s8_t Check_Buf_Request_Signal(SqQueue buf)
{
	extern struct netconn *order_netconn;	//全局TCP链接
	extern OS_EVENT *Batch_Rec_Sem;
	INT8U err;
	u16_t empty_buf_size = buf.MAX - (buf.write + buf.MAX -buf.read )%buf.MAX;
	if( empty_buf_size >= THRESHOLD_SIZE)
	{
		if(0 < OSSemAccept(Batch_Rec_Sem))//表明读取完成
		{

			write_connection(order_netconn, order_req, 0, 0); 
				DEBUG_PRINT("<<<<<<<<<<<<<<<<<<<<<<<<<<--------------------------SEND ORDER REQ--------------------------->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		}
		else
			;	
	}
	
	return 0;
}


/****************************************************************************
* 名    称：s8_t Read_Order_Queue(ElemType *e)
* 功    能：读取一份订单数据，除去头尾信息
* 入口参数：@e : 返回指向该份订单数据的指针
* 出口参数：获取订单是否成功
* 说    明：
* 调用方法：无 
****************************************************************************/
/****************************************************************************
* 名    称：s8_t Read_Order_Queue(ElemType *e)
* 功    能：读取一份订单数据，除去头尾信息
* 入口参数：@e : 返回指向该份订单数据的指针
* 出口参数：获取订单是否成功
* 说    明：
* 调用方法：无 
****************************************************************************/
s8_t Read_Order_Queue(SqQueue *buf,ElemType *e)
{
	u16_t order_len = 0;
	u32_t old_read  = buf->read;	
	u8_t leftBytes;
	int i = 0;
	char * orderDataPtr = (char *) e;
	Read_Order_Length_Queue(*buf,&order_len);   //获得该份订单长度
	//去除订单头部
	buf->read = (buf->read+BUF_HEAD) % buf->MAX;//清除订单头
	//复制缓存内容到订单data
	DEBUG_PRINT("--------------ORDER  10 K LENGTH : %d\n", order_len );
	for(i = 0 ; i < order_len ; ++i)
	{
		*orderDataPtr = buf->base[(buf->read + i )% buf->MAX];
		orderDataPtr++;					
	}			
	orderDataPtr[order_len] = '\0';
	//读指针定位到下一份订单开始				
	buf->read = (buf->read + order_len + BUF_END ) % buf->MAX;	
	if(buf->read == buf->write)
	{
		buf->buf_empty = 1;   //缓冲区为空
		DEBUG_PRINT ("Buf is empty\n");
	}

			//更新缓冲区剩余容量
	order_print_table.buf_node.common_buf_remain_capacity = order_print_table.buf_node.common_buf_remain_capacity  + order_len + BUF_HEAD + BUF_END;
	return BUF_OK;
}

//获取批次头部
u8_t *Get_Batch_Head(SqQueue buf)
{
	if(buf.base[buf.read] == 0xaa && buf.base[(buf.read + 1) % buf.MAX] == 0x55)
		return buf.base + buf.read;
	else
		return NULL;
}

//获取批次头部
u8_t *Get_Order_Head(SqQueue buf)
{
	if(buf.base[buf.read] == 0x3e && buf.base[(buf.read + 1) % buf.MAX] == 0x11)
		return buf.base + buf.read;
	else
		return NULL;
}

u8_t Is_Empty_Queue(SqQueue buf)
{
	return buf.buf_empty;
}


