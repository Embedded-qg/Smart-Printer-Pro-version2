#include "print_queue.h"

#define assert(expr, str)			 	\
	do {								\
		if(!(expr)) {					\
			printf((str));				\
			while(1);					\
		}								\
	}while(0)

u32 hour = 0, min = 0, sec = 0,msec = 0;//时分秒 毫秒，用于获取网络时间
INT32U StartTime = 0; //基准时间

/**
 * @name   	Find_Entry
 * @brief	 	根据订单长度，找到合适的节点，并且返回索引，查找插入的内存块索引
 * @param	  len 								订单长度
 *					order_prio_sigal 		订单优先级
 * @return	
 */
static s8_t Find_Entry(u16_t len , u8_t order_prio_sigal);


/**
 * @name   	Change_Normal_Order_Seque
 * @brief	 	将分配到的节点，加入到队列中，区分加急与普通。
 * @param	  entry_index 传入索引号
 * @return	
 */
static s8_t Change_Normal_Order_Seque( s8_t entry_index );


/**
 * @name   	Change_Order_Seque
 * @brief	 	打印队列节点用完时，有加急订单，但是未能入队，
						所以尽快处理队列顺序，让加急订单尽快入队。
 * @param	  entry_index 传入索引号
 * @return	
 */
static s8_t Change_Order_Seque(s8_t entry_index);


/**
 * @name   	Find_Block_Num
 * @brief	 	被Find_Entry调用，根据对应块大小，与优先级，返回索引。
 * @param	  blocksize 					对应节点大小
 *					order_prio_sigal 		订单优先级
 * @return	
 */
static s8_t Find_Block_Num(u16_t blocksize ,u8_t order_prio_sigal);


/**
 * @name   	Find_Front_And_Next_Index
 * @brief	 	给定size的节点块，找到相等大小的节点块的前驱，与后继
 * @param	  start_index 				开始编号
 *					aim_size 						目标大小
 *					front_index					前驱索引
 *					next_index					后继索引
 *					current_index				当前索引
 * @return	
 */
static s8_t Find_Front_And_Next_Index(s8_t start_index , u16_t aim_size,  u8_t *front_index , u8_t *next_index, u8_t *current_index);


/**
 * @fn		OrderEnqueue
 * @brief	缓冲区订单入队
 * @param	buf 缓冲区指针
 *				entry_index 索引号
 *				order_len 订单长度
 *				order_prio_sigal 紧急或者普通的标志
 * @return	void
 */
static s8_t OrderEnqueue(SqQueue* buf,s8_t entry_index , u16_t order_len,u8_t order_prio_sigal);


/**
 * @brief	将普通订单添加到打印队列中
 * @param	buf 普通订单缓冲区
			entry_index 订单索引
 * @return	操作结果
 */
static s8_t	Add_Order_To_Print_Queue(SqQueue *buf,s8_t entry_index , u8_t order_prio_sigal);

/**
 * @fn		error_order_deal
 * @brief	订单错误，处理函数。
 * @param	buf	 缓冲区
 * @return	
 */
static s8_t error_order_deal(SqQueue* buf, s8_t order_prio_sigal);


/**
 * @fn		printOrderQueueSeque
 * @brief	用于调试输出打印队列的列表信息。
 * @param	buf	 缓冲区
 * @return	
 */
static void printOrderQueueSeque();
	

//打印队列订单信息表
order_print_queue_info order_print_table;        
//批次表
batch_info batch_info_table[MAX_BATCH_NUM];				 
//合同表


/**
 * @brief	获取网络时间
 * @param	打印单元数据域
 * @return	执行结果
 */
void GetTime(u8_t *data)
{
	hour = 0;
	min = 0;
	sec = 0;
	msec = 0;
}


/**
 * @brief	增加时间
 * @param	time-增加的毫秒
 * @return	执行结果
 */
void ShowTime(u32_t order_time, INT32U startTime, INT32U OSTime)
{
	u32_t hour,min,sec,msec,tmp;
	u32_t subTime = 0;
	hour = order_time/10000000;//时
	min = (order_time/100000)%100;//分
	sec = (order_time/1000)%100;//秒
	msec = order_time%1000;//毫秒
	
	subTime = OSTime - startTime;
	
	tmp = msec;
	msec = (tmp + subTime)%1000;
	tmp = (tmp + subTime)/1000;
	
	sec = sec + tmp;
	tmp = sec;
	sec = sec%60;
	tmp = tmp/60;
	
	min = min + tmp;
	tmp = min;
	min = min%60;
	tmp = tmp/60;
	
	hour = hour + tmp;
	
	if(hour == 24)
		hour = 0;
	
	DEBUG_PRINT_TIMME("时间为：%u:%u:%u:%u\r\n",hour, min, sec, msec);
}




/**
 * @name   	Find_Entry
 * @brief	 	根据订单长度，找到合适的节点，并且返回索引，查找插入的内存块索引
 * @param	  len 								订单长度
 *					order_prio_sigal 		订单优先级
 * @return	
 */
static s8_t Find_Entry(u16_t len , u8_t order_prio_sigal)
{
	if(len > BLOCK_MAX_SIZE)       //参数检查
	{
		return ORDER_TOO_LARGER;
	}
	
	if(len <= BLOCK_1K_SIZE)
	{
		ORDER_DEBUG_PRINT("QUEUE DEBUG : TRYING TO GOT 1K NODE-----------\r\n");
		return Find_Block_Num(BLOCK_1K_SIZE ,order_prio_sigal);
	}
	else if (len <= BLOCK_2K_SIZE)
	{	
		return Find_Block_Num(BLOCK_2K_SIZE ,order_prio_sigal);
	}
	else if(len <= BLOCK_4K_SIZE)
	{	
		return Find_Block_Num(BLOCK_4K_SIZE ,order_prio_sigal);
	}
	else if (len <= BLOCK_10K_SIZE)
	{	
		return Find_Block_Num(BLOCK_10K_SIZE ,order_prio_sigal);	
	}
	else 
		return ORDER_FIND_INDEX_ERR;
	
}


/**
 * @name   	Find_Block_Num
 * @brief	 	被Find_Entry调用，根据对应块大小，与优先级，返回索引。
 * @param	  blocksize 					对应节点大小
 *					order_prio_sigal 		订单优先级
 * @return	
 */
static s8_t Find_Block_Num(u16_t blocksize ,u8_t order_prio_sigal)
{	
	u8_t os_err = 0;
	OS_EVENT * Block_Sem = NULL;	
	s8_t Lack_Num = 0;
	
	u8_t entry_index = 0;
	u8_t block_index_start = 0;	
	u8_t block_index_end = 0;
	
	switch(blocksize)
	{	
		case BLOCK_1K_SIZE:{
			block_index_start = BLOCK_1K_INDEX_START;//起始索引
			block_index_end = BLOCK_1K_INDEX_END;//结束索引
			Lack_Num = Lack_Of_1K;	//缺少内存块的信号
			Block_Sem = Block_1K_Sem; //内存块存量的信号量
			break;
		}
		case BLOCK_2K_SIZE:{			
			block_index_start = BLOCK_2K_INDEX_START;
			block_index_end = BLOCK_2K_INDEX_END;
			Lack_Num = Lack_Of_2K;
			Block_Sem = Block_2K_Sem;
			break;
		}
		case BLOCK_4K_SIZE:{			
			block_index_start = BLOCK_4K_INDEX_START;			
			block_index_end = BLOCK_4K_INDEX_END;
			Lack_Num = Lack_Of_4K;
			Block_Sem = Block_4K_Sem;
			break;
		}
		case BLOCK_10K_SIZE:{
			block_index_start = BLOCK_10K_INDEX_START;			
			block_index_end = BLOCK_10K_INDEX_END;
			Lack_Num = Lack_Of_10K;
			Block_Sem = Block_10K_Sem;
			break;
		}
		default :
			ORDER_DEBUG_PRINT("QUEUE DEBUG :  SWITCH ERROR--------------\r\n");
			break;
	}
	
	if(order_prio_sigal){//紧急订单
		INT16U Accept_Sigal_num = 0;
		Accept_Sigal_num = OSSemAccept(Block_Sem);
		if( Accept_Sigal_num == 0){//证明已经没有内存块,告诉线程紧急订单没有内存块的信号。
			
			ORDER_DEBUG_PRINT("QUEUE DEBUG :  LACK OF BLOCK ,Lack_Num %d\r\n ",Lack_Num);
			OSTimeDlyHMSM(0, 0, 0, 500);
			return Lack_Num;
		}
	}
	else{//非紧急订单
		OSSemPend(Block_Sem,0,&os_err);	//若无空闲内存块，则阻塞
		ORDER_DEBUG_PRINT("QUEUE DEBUG :  GOT Block_Sem-------------\r\n");
	}
	
//	ORDER_DEBUG_PRINT("QUEUE DEBUG :  acquire mutex of printqueue-----------\r\n");
	OSMutexPend(order_print_table.mutex,0,&os_err);//拿队列资源的锁
//	ORDER_DEBUG_PRINT("QUEUE DEBUG :  got mutex of printqueue-----------\r\n");
	entry_index = block_index_start;
	
	ORDER_DEBUG_PRINT("QUEUE DEBUG : entry_index: %u, block_index_start = %u,------- block_index_end = %u\r\n", entry_index, block_index_start, block_index_end);
	
	while(entry_index < block_index_end){//查找空的索引号
		if(order_print_table.order_node[entry_index].data == NULL){			
			OSMutexPost(order_print_table.mutex);//释放队列资源的锁	
			ORDER_DEBUG_PRINT("QUEUE DEBUG : Find EmptyBlock =  %u\r\n", entry_index);
			return entry_index;
		}
		entry_index++;
	}	
	
	OSMutexPost(order_print_table.mutex);
	ORDER_DEBUG_PRINT("QUEUE DEBUG :   ORDER_FIND_INDEX_ERR-----------\r\n");
	return ORDER_FIND_INDEX_ERR;
}


/**
 * @brief	将普通订单添加到打印队列中
 * @param	buf 普通订单缓冲区
			entry_index 订单索引
 * @return	操作结果
 */
static s8_t	Add_Order_To_Print_Queue(SqQueue *buf,s8_t entry_index , u8_t order_prio_sigal)
{
	u8_t os_err = 0;
	u8_t *order_head = NULL;
	INT8U err;
	u8_t time_index = 0;
	u8_t hash;
	u16_t bacth_number;
	u32_t current_number;
	u32_t serial_number;
	u32_t order_time;
	order_head = buf->base + buf->read;// 获取订单头//  这里用于存放buf的订单缓冲数据流的头部
	
	OSMutexPend(order_print_table.mutex,0,&os_err);
	
	//开始申请内存块
	if(entry_index < BLOCK_1K_INDEX_END){
		ORDER_DEBUG_PRINT("\nQUEUE DEBUG----GOT 1K BLOCK----\r\n");
		order_print_table.order_node[entry_index].data = OSMemGet(queue_1K,&os_err);
		serial_number = ((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +3 )));	//订单序号
		order_time = ((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +3 )));	//下发时间		
		
		DEBUG_PRINT_TIMME("分配内存大小：1K，订单编号为：%u\r\n",serial_number);
//		ShowTime(order_time,StartTime,OSTimeGet()*TIME_INTERVAL);
	}
	else if(entry_index < BLOCK_2K_INDEX_END){
		ORDER_DEBUG_PRINT("\nQUEUE DEBUG----GOT 2K BLOCK----\r\n");
		order_print_table.order_node[entry_index].data = OSMemGet(queue_2K,&os_err);
		serial_number = ((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +3 )));	//订单序号
		order_time = ((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +3 )));	//下发时间		
		
		DEBUG_PRINT_TIMME("分配内存大小：2K，订单编号为：%u\r\n",serial_number);
//		ShowTime(order_time,StartTime,OSTimeGet()*TIME_INTERVAL);		
	}
	else if(entry_index < BLOCK_4K_INDEX_END){
		ORDER_DEBUG_PRINT("\nQUEUE DEBUG----GOT 4K BLOCK----\r\n");
		order_print_table.order_node[entry_index].data = OSMemGet(queue_4K,&os_err);
		serial_number = ((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +3 )));	//订单序号
		order_time = ((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +3 )));	//下发时间		
		
		DEBUG_PRINT_TIMME("分配内存大小：4K，订单编号为：%u\r\n",serial_number);
//		ShowTime(order_time,StartTime,OSTimeGet()*TIME_INTERVAL);
	}
	else if(entry_index < BLOCK_10K_INDEX_END){	
		ORDER_DEBUG_PRINT("\nQUEUE DEBUG----GOT 10K BLOCK----\r\n");
		order_print_table.order_node[entry_index].data = OSMemGet(queue_10K,&os_err);
		ORDER_DEBUG_PRINT("ORDER DATA POINTER: %p\r\n", order_print_table.order_node[entry_index].data);
		serial_number = ((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +3 )));	//订单序号
		order_time = ((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +3 )));	//下发时间		
		
		DEBUG_PRINT_TIMME("分配内存大小：10K，订单编号为：%u\r\n",serial_number);
//		ShowTime(order_time,StartTime,OSTimeGet()*TIME_INTERVAL);	
	}
	
	/*解析读取订单头部数据*/
	order_print_table.order_node[entry_index].size = 								(*(order_head + ORDER_SIZE_OFFSET) << 8) 						+ *(order_head + ORDER_SIZE_OFFSET + 1);											//订单长度	
	
	order_print_table.order_node[entry_index].mcu_id = 							((u32_t)(*(order_head + ORDER_MCU_ID_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_MCU_ID_OFFSET +1 )) << 16 ) 						
																																	+	((u32_t)(*(order_head + ORDER_MCU_ID_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_MCU_ID_OFFSET +3 )));													//主控板ID
	
	order_print_table.order_node[entry_index].sever_send_time =  		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET)) << 24) 			+ 	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +1 )) << 16 ) 
																																	+	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +3 )));				//时间
	
	order_print_table.order_node[entry_index].serial_number = 			((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET)) << 24)				+ 	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +1 )) << 16 ) 
																																	+	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +3 )));						//订单序号
	ORDER_DEBUG_PRINT("QUEUE DEBUG :id number :%u\r\n", order_print_table.order_node[entry_index].serial_number);
	
	order_print_table.order_node[entry_index].batch_number = 				(*(order_head+ORDER_BATCH_NUMBER_OFFSET)<<8)				+ *(order_head + ORDER_BATCH_NUMBER_OFFSET + 1);							//所属批次
	order_print_table.order_node[entry_index].batch_within_number =	(*(order_head+ORDER_BATCH_WITHIN_NUMBER_OFFSET)<<8)	+ *(order_head + ORDER_BATCH_WITHIN_NUMBER_OFFSET + 1);				//批次内订单序号
	
	DEBUG_PRINT("QUEUE DEBUG :id number :%u", order_print_table.order_node[entry_index].batch_number);
	DEBUG_PRINT("QUEUE DEBUG :id number :%u", order_print_table.order_node[entry_index].batch_within_number);
	
	bacth_number = order_print_table.order_node[entry_index].batch_number;
	hash = get_batch_hash(bacth_number);
	order_print_table.order_node[entry_index].arrTime = batch_info_table[hash].startTime;;
	order_print_table.order_node[entry_index].errorTime = 0;
	order_print_table.order_node[entry_index].finishTime = 0;
	
	order_print_table.order_node[entry_index].check_sum = 					(*(order_head+ORDER_CHECK_SUM_OFFSET)<<8)						+ *(order_head + ORDER_CHECK_SUM_OFFSET + 1);									//校验码						//保留
	order_print_table.order_node[entry_index].data_source = 	order_print_table.order_node[entry_index].preservation;
	order_print_table.order_node[entry_index].priority = order_prio_sigal;
  order_print_table.order_node[entry_index].next_print_node = -1;
	
	
	if(order_print_table.empty == 1){//若原先打印队列为空
		order_print_table.head = entry_index;
		order_print_table.tail = entry_index;
		order_print_table.empty = 0;
	}
	else{
		//更改打印队列信息，确定订单的位置顺序
		ORDER_DEBUG_PRINT("QUEUE DEBUG :Before Insert\r\n");
		printOrderQueueSeque();
		Change_Normal_Order_Seque(entry_index);
		ORDER_DEBUG_PRINT("QUEUE DEBUG :After Insert\r\n");
		printOrderQueueSeque();
	}
	
	//将订单数据域内容加入打印队列
	Read_Order_Queue(buf,order_print_table.order_node[entry_index].data);
	
	//使用完打印队列，释放锁
	OSMutexPost(order_print_table.mutex);
	OSTimeDlyHMSM(0,0,0,100);
	
	return ORDER_QUEUE_OK;
}



static void printOrderQueueSeque()
{
	u8_t index = order_print_table.head;
	ORDER_DEBUG_PRINT("QUEUE DEBUG : traverse queue{ \r\n");
	ORDER_DEBUG_PRINT("				node : %u  - >  next: %u\r\n",index , order_print_table.order_node[index].next_print_node);		
	while (index != order_print_table.head)
	{				
		index = order_print_table.order_node[index].next_print_node;
		ORDER_DEBUG_PRINT("				node : %u  - >  next: %u\r\n",index , order_print_table.order_node[index].next_print_node);
	}
			
		
	return ;
}



/**
 * @name   	Change_Normal_Order_Seque
 * @brief	 	将分配到的节点，加入到队列中，区分加急与普通。
 * @param	  entry_index 传入索引号
 * @return	
 */
static s8_t Change_Normal_Order_Seque( s8_t entry_index )
{
	if(order_print_table.order_node[entry_index].priority == ORDER_PRIO_HIGH){//加急订单
		u8_t start_index = order_print_table.head;
		u8_t start_index_front = order_print_table.head;
		
		while(order_print_table.order_node[start_index].data != NULL && order_print_table.order_node[start_index].priority == ORDER_PRIO_HIGH){	//遍历完加急队列。
			start_index_front = start_index;
			start_index = order_print_table.order_node[start_index].next_print_node;
		}
		if(start_index == start_index_front){//表明从队头开始，就不是加急订单			
			order_print_table.order_node[entry_index].next_print_node = order_print_table.head;
			order_print_table.head = entry_index;							
			return 0;
		}
		else{//表明队列中有加急订单。
			order_print_table.order_node[entry_index].next_print_node = order_print_table.order_node[start_index_front].next_print_node;
			order_print_table.order_node[start_index_front].next_print_node = entry_index;
			if(order_print_table.order_node[start_index].data == NULL){			//如果打印队列只剩下加急订单
				order_print_table.tail = entry_index;
				order_print_table.order_node[entry_index].next_print_node = -1;
			}
			return 0;			
		}
	}
	else{//普通订单
		order_print_table.order_node[order_print_table.tail].next_print_node = entry_index;
		order_print_table.tail = entry_index;
	}
}


/**
 * @name   	Change_Order_Seque
 * @brief	 	打印队列节点用完时，有加急订单，但是未能入队，
						所以尽快处理队列顺序，让加急订单尽快入队。
 * @param	  entry_index 传入索引号
 * @return	
 */
static s8_t Change_Order_Seque(s8_t entry_index)
{
	u8_t next_index;	//最近的相同内存块的索引的后一个
	u8_t front_index;	//最近的相同内存块的索引的前一个
	u8_t current_index; //最近的相同内存块的索引
	u16_t block_size = 0;
	s8_t same_block_index = 0;
	u16_t printing_size = 0;
	
	if(entry_index < 0){	//参数判断
		switch (entry_index)
		{
			case Lack_Of_1K :{
				block_size = BLOCK_1K_SIZE;
				break;
			}
			case Lack_Of_2K:{				
				block_size = BLOCK_2K_SIZE;
				break;
			}
			case Lack_Of_4K:{
				block_size = BLOCK_4K_SIZE;
				break;
			}
			case Lack_Of_10K:{
				block_size = BLOCK_10K_SIZE;
				break;
			}
		}
		
		if(order_print_table.order_node[order_print_table.head].size <= BLOCK_1K_SIZE)
		{
			printing_size = BLOCK_1K_SIZE;
		}
		else if(order_print_table.order_node[order_print_table.head].size <= BLOCK_2K_SIZE)
		{
			printing_size = BLOCK_2K_SIZE;
		}
		else if(order_print_table.order_node[order_print_table.head].size <= BLOCK_4K_SIZE)
		{
			printing_size = BLOCK_4K_SIZE;
		}
		else if (order_print_table.order_node[order_print_table.head].size <= BLOCK_10K_SIZE)
		{
			printing_size = BLOCK_10K_SIZE;
		}
		
		if(printing_size != block_size){	//队头就是不相同内存的订单，需要进行替换
			u8_t start_index = order_print_table.head;
			u8_t start_index_front = order_print_table.head;
			u8_t in_urgent_order_flag = 0;
			while(order_print_table.order_node[start_index].priority == ORDER_PRIO_HIGH){	//遍历完加急队列。
				if(order_print_table.order_node[start_index].size == block_size){
					in_urgent_order_flag = 1;
					break;
				}
				if(start_index == order_print_table.tail )
					break;
				start_index_front = start_index;				
				start_index = order_print_table.order_node[start_index].next_print_node;
			}
			
			
			if(start_index == start_index_front && order_print_table.order_node[start_index].priority != ORDER_PRIO_HIGH){//表明从队头开始，就不是加急订单
				
				
				Find_Front_And_Next_Index(start_index, block_size, &next_index , &front_index, &current_index);//找到最靠近队头的那个相同内存的订单索引上下文索引。
				order_print_table.order_node[front_index].next_print_node = next_index;
				order_print_table.order_node[current_index].next_print_node = order_print_table.head;						
				order_print_table.head = current_index;
				
				ORDER_DEBUG_PRINT("QUEUE DEBUG : head change \r\n" );
				return 0;
			}
			else{//表明队列中有加急订单。
				if(in_urgent_order_flag == 0){//相同内存块索引，不在加急订单当中。
					if(start_index == order_print_table.tail)
					{
						
					ORDER_DEBUG_PRINT("QUEUE DEBUG : start_index %u\r\n" , start_index );
						return 0;
					}
					Find_Front_And_Next_Index(start_index, block_size, &next_index , &front_index, &current_index);//找到最靠近队头的那个相同内存的订单索引上下文索引。
					
					if(front_index == current_index){				
						//普通订单打印队列中的第一个就是相同大小的内存块，所以不用进行操作。
						
					ORDER_DEBUG_PRINT("QUEUE DEBUG : first do not change \r\n");
						return 0;
					}
					else{					
						//普通订单打印队列中的第一个不是内存块相同大小的订单，需要把相同大小的订单放在普通打印队列的首部。
						order_print_table.order_node[front_index].next_print_node = next_index;
						order_print_table.order_node[current_index].next_print_node = start_index;						
						order_print_table.order_node[start_index_front].next_print_node = current_index;	
					}				
				}				
				else{
					//相同的内存单元的订单，是加急订单，则不需要进行处理。
					ORDER_DEBUG_PRINT("QUEUE DEBUG : same do not change\r\n" );
					return 0;
				}
			}
		}
		else{		
			return 0; //队头就是相同内存单元订单，等待其打印完成即可。
		}
	}
}


/**
 * @name   	Find_Front_And_Next_Index
 * @brief	 	给定size的节点块，找到相等大小的节点块的前驱，与后继
 * @param	  start_index 				开始编号
 *					aim_size 						目标大小
 *					front_index					前驱索引
 *					next_index					后继索引
 *					current_index				当前索引
 * @return	
 */
static s8_t Find_Front_And_Next_Index(s8_t start_index , u16_t aim_size,  u8_t *front_index , u8_t *next_index ,u8_t *current_index)
{
	u8_t front = start_index;
	u8_t next = order_print_table.order_node[start_index].next_print_node; 
	u8_t current = start_index;
	while(next != order_print_table.tail){		 
		if(order_print_table.order_node[current].size == aim_size ){//结束
			break;	
		}
		else{
			front = current;
			current = next;
			next = order_print_table.order_node[current].next_print_node;
		}
		OSTimeDlyHMSM(0, 0, 1, 0);
	}
	*front_index = front;
	*next_index = next;
	*current_index = current;
	return 0;
}


/**
 * @brief	删除一份订单，归还内存至系统
 * @param	订单索引
 * @return	执行结果
 */
s8_t Delete_Order(s8_t entry_index)
{
	u8 err;
	order_info *orderp = &order_print_table.order_node[entry_index];
	u8_t *data = orderp->data;
	u32_t current_number = orderp->mcu_id;
	
	OSMutexPend(order_print_table.mutex, 0, &err);	
	orderp->data = NULL;
	if(NULL == data) {
		ORDER_DEBUG_PRINT("Delete_Order: Internal Error: Order %u has null block!\r\n", 
										orderp->serial_number);
		OSMutexPost(order_print_table.mutex);
		return ORDER_EMPTY_BLOCK;
	}
	ORDER_DEBUG_PRINT("Delete_Order: Deleting order %u.\r\n", orderp->serial_number);
	OSMutexPost(order_print_table.mutex);	
	/*释放内存块*/		
	if(entry_index < BLOCK_1K_INDEX_END) {
		ORDER_DEBUG_PRINT("Delete_Order: PUT 1K BLOCK\r\n");
		OSMemPut(queue_1K, data);
		OSSemPost(Block_1K_Sem);
	}else if(entry_index < BLOCK_2K_INDEX_END){
		ORDER_DEBUG_PRINT("Delete_Order: PUT 2K BLOCK\r\n");
		OSMemPut(queue_2K, data);
		OSSemPost(Block_2K_Sem);
	}else if(entry_index < BLOCK_4K_INDEX_END){
		ORDER_DEBUG_PRINT("Delete_Order: PUT 4K BLOCK\r\n");
		OSMemPut(queue_4K, data);			
		OSSemPost(Block_4K_Sem);
	}else if(entry_index < BLOCK_10K_INDEX_END){
		ORDER_DEBUG_PRINT("Delete_Order: PUT 10K BLOCK\r\n");
		OSMemPut(queue_10K, data);			
		OSSemPost(Block_10K_Sem);
	}
//	DEBUG_PRINT_TIMME("释放内存块，订单编号为，");//1ms中断一次*时钟节拍
//	ShowTime(orderp->sever_send_time,StartTime,OSTimeGet()*TIME_INTERVAL);
	return ORDER_QUEUE_OK;
}



static u8_t GetDataType(const u8_t *datap)
{
	if(0x7e == datap[0] && 0xff == datap[1])
		return DATA_IS_PLAINTEXT;
	else if(0xff == datap[0] && 0x7e == datap[1])
		return DATA_IS_IMAGE;
	else if(0x7f == datap[0] && 0xff == datap[1])
		return DATA_IS_QRCODE;
	return DATA_INVALID;
}

static u16_t GetDataLength(const u8_t *datap)
{
	return (datap[2] << 8) + datap[3];
}

static u8_t *MoveToNextData(u8_t *datap, u16_t length)
{
	u8_t *nextp = datap + length + DATA_HEADER_SIZE + DATA_FOOTER_SIZE;
	return nextp;
}

static u16_t GetImageBytes(const u8_t *datap, u16_t length)
{
	return (datap[DATA_HEADER_SIZE+length] << 8) + datap[DATA_HEADER_SIZE+length+1];
}


/**
 * @fn		GetOrderFromQueue
 * @brief	脱离打印队列头结点并将其索引放置在entryp所指空间
 * @param	entryp	存放获得的结点的索引
 * @return	ORDER_QUEUE_OK	获取成功
			ORDER_QUEUE_EMPTY 队列为空，获取失败
 */
s8_t GetOrderFromQueue(u8_t *entryp)
{
	INT8U err;
	
	OSMutexPend(order_print_table.mutex, 0, &err);
	
	if(order_print_table.empty == 1) {
		OSMutexPost(order_print_table.mutex);		
		return ORDER_QUEUE_EMPTY;
	}
	*entryp = order_print_table.head;	//取队列第一份订单
	if(order_print_table.head == order_print_table.tail) {	//队列中仅有一份订单，取走后队列为空
		order_print_table.empty = 1;
		order_print_table.head = order_print_table.tail = 0;
	}else {		//队列中有多于一份订单
		order_print_table.head = order_print_table.order_node[order_print_table.head].next_print_node;
	}
	order_print_table.order_node[*entryp].next_print_node = 0;
	ORDER_DEBUG_PRINT("GetOrderFromQueue: Got print order %u\r\n", 
		order_print_table.order_node[*entryp].serial_number);
	if(order_print_table.empty == 1) {
		ORDER_DEBUG_PRINT("GetOrderFromQueue: No more job appending.\r\n");
	}else {
		ORDER_DEBUG_PRINT("GetOrderFromQueue: Next job's num is %u\r\n", 
			order_print_table.order_node[order_print_table.head].serial_number);		
	}
	OSMutexPost(order_print_table.mutex);
	return ORDER_QUEUE_OK;
}


/**
 * @brief	检查订单数据域是否有错
 * @param	订单索引
 * @return	ORDER_DATA_ERR	数据域有误
			ORDER_DATA_OK	数据域正确
 */
s8_t CheckOrderData(u8_t entry_index)
{
	u16_t pbytes, length;
	u8_t type;
	order_info *orderp = &order_print_table.order_node[entry_index];
	u8_t *datap = orderp->data;	
	
	if(NULL == datap) {
		orderp->status = PRINT_STATUS_DATA_ERR;	/* 订单数据错误 */
		DEBUG_PRINT_TIMME("打印失败，订单数据错误\r\n");
//		ShowTime(orderp->sever_send_time,StartTime,OSTimeGet()*TIME_INTERVAL);
		Order_Print_Status_Send(orderp,PRINT_STATUS_DATA_ERR);
		ORDER_DEBUG_PRINT("CheckOrderData: checking empty block!\r\n");
		return ORDER_DATA_ERR;		
	}
	pbytes = 0;
	while(pbytes < orderp->size) {	// 解析订单内容
		length = GetDataLength(datap);
		type = GetDataType(datap);
 
		if(type == DATA_INVALID){	// 数据错误			
			orderp->status = PRINT_STATUS_DATA_ERR;	/* 订单数据错误 */
			Order_Print_Status_Send(orderp,PRINT_STATUS_DATA_ERR);

			ORDER_DEBUG_PRINT("CheckOrderData: Invalid type of data field!\r\n");
			
			return ORDER_DATA_ERR;
		}
		datap = MoveToNextData(datap, length);	// 移动至下一份数据
		pbytes += length + DATA_HEADER_SIZE + DATA_FOOTER_SIZE;
	}
	if(pbytes == 0)	// 空数据订单
		return ORDER_DATA_ERR;
	return ORDER_DATA_OK;
}


/**
 * @brief	打印一份订单，并修改其状态
 * @param	打印单元索引
 * @return	执行结果
 */
s8_t Print_Order(u8_t cellno)
{
	u16_t length, pbytes;
	order_info *orderp;
	u8_t *datap;
	u8_t type;
	INT8U err;
	PrintCellInfo *cellp = &PCMgr.cells[cellno-1];
	static int needCutPaper[MAX_CELL_NUM+1] = { 0 };
	int i;
	
	orderp = &order_print_table.order_node[cellp->entryIndex];
	datap = orderp->data;	
	orderp->mcu_id = cellno;
	length = GetDataLength(datap);

	if(needCutPaper[cellno]) {	// 切除打印过程中由于打印机连接断开或电源故障等异常而打印的错误订单
		OutputErrorTag(cellno);
		cutPaper(cellno);
		needCutPaper[cellno] = 0;
	}
	
	ORDER_DEBUG_PRINT("-------------Print_Order: ---------------\r\n");
	ORDER_DEBUG_PRINT("-ORDER NUM:   %u\r\n", orderp->serial_number);
	ORDER_DEBUG_PRINT("-ORDER SIZE : %u\r\n", orderp->size);
	ORDER_DEBUG_PRINT("-DATA HEADER: %x%x\r\n", *orderp->data, *(orderp->data+1));
	ORDER_DEBUG_PRINT("-DATA LENGTH: %u\r\n", length);
	ORDER_DEBUG_PRINT("-DATA TAIL  : %x%x\r\n", *(datap + 4 + length + 2), *(datap + 4 + length + 3));
	ORDER_DEBUG_PRINT("-------------------------------------\r\n");	
	
	
	orderp->status = PRINT_STATUS_START;
	Order_Print_Status_Send(orderp,	PRINT_STATUS_START);
	
	cellp->beginTick = sys_now();
	
	// 作业间隔时间大于2s则认为打印单元没有连续工作
	if(cellp->beginTick - cellp->endTick > 2000/20)
		cellp->workedTime = 0;

	pbytes = 0;		
	while(pbytes < orderp->size) {	//解析订单内容并打印
		length = GetDataLength(datap);
		type = GetDataType(datap);
 
		if(DATA_IS_PLAINTEXT == type) {		// 文字内容
			ORDER_DEBUG_PRINT("Print_Order: Printing Paint Text\r\n");
			//DMA_To_USARTx_Send(datap + DATA_HEADER_SIZE, length, cellno);
			mySendData(datap + DATA_HEADER_SIZE, length, cellno);
		}
		else if(DATA_IS_IMAGE == type) {	// 图片内容		

			ORDER_DEBUG_PRINT("Print_Order: Printing Picture\r\n");
			OSTimeDlyHMSM(0,0,2,0);	
			printImages(datap + DATA_HEADER_SIZE, GetImageBytes(datap, length), cellno);
			
		}
		else if(DATA_IS_QRCODE == type) {	// 二维码链接		
//			OSTimeDlyHMSM(0,0,0,500);
			printQRCode(datap + DATA_HEADER_SIZE, length, cellno);
			ORDER_DEBUG_PRINT("Print_OrderPrinting: QR Code\r\n");
		}else {
			ORDER_DEBUG_PRINT("BUG DETECT: Print_Order: DATA CHECKING WRONG.\r\n");
		}
		pbytes += length + DATA_HEADER_SIZE + DATA_FOOTER_SIZE; //已经处理的字节数
		PCMgr.cells[cellno-1].totalLength += pbytes;
		
		datap = MoveToNextData(datap, length);
	}
	orderp->status = ORDER_DATA_OK;	// 订单数据解析正确
	
	// 发送检测打印机状态的指令，以决定本订单是否打印成功
	OSTimeDlyHMSM(0,0,(u8)((orderp->size / 512) + (orderp->size / 2048)), 500);
	ORDER_DEBUG_PRINT("Print_Order: Waiting for Printer's status.\r\n");
	SEND_STATUS_CMD_ONE(cellno);
	OSTimeDlyHMSM(0,0,1,0);//发送状态后延时
	
	// 正常情况下Pend请求会立刻返回，若超时则代表打印过程中传输线异常
	OSSemPend(cellp->printDoneSem, 20, &err);
	if(err == OS_ERR_TIMEOUT){	// 打印过程中出现异常，使得打印机离线
		ORDER_DEBUG_PRINT("Print_Order: Printer %u Off-line while Printing.\r\n", cellno);
		needCutPaper[cellno] = 1;
		Order_Print_Status_Send(orderp, PRINT_CELL_STATUS_ERR);	
		PutPrintCell(cellno, PRINT_CELL_STATUS_ERR);
	}
	DEBUG_PRINT_TIMME("\r\n");
	return ORDER_DATA_OK;
}


void Init_Order_Table(void)
{
	u8_t entry_index = 0;
	INT8U  err;
	order_info *orderp;
	
	/*初始化信息表中的缓冲区记录*/
	order_print_table.buf_node.common_buf_size = MAXQSIZE;
	order_print_table.buf_node.common_buf_remain_capacity = MAXQSIZE;
	
	order_print_table.buf_node.urgent_buf_size = MAXUSIZE;
	order_print_table.buf_node.urgent_buf_remain_capacity = MAXUSIZE;	
	
	/* 初始化互斥量 */
	order_print_table.mutex = OSMutexCreate(ORDER_PRINT_TABLE_MUTEX_PRIO, &err);
	assert(NULL != order_print_table.mutex, 
		"Init_Order_Table: Create mutex for print table failed.");
	
	/* 初始化队列管理结构 */
	order_print_table.head = 0;   //打印队列队首索引
	order_print_table.tail = 0;   //打印队列队尾索引
	order_print_table.empty = 1;  //打印队列是否为空：1代表空、0代表非空

	
	/*初始化信息表中的订单记录*/
	while(entry_index < ALL_BLOCK_NUM)
	{

		orderp = &order_print_table.order_node[entry_index];

		orderp->data = NULL;
		orderp->size = 0;
		orderp->serial_number = 0;
		orderp->batch_number = 0;
		orderp->batch_within_number = 0;
		orderp->priority = ORDER_PRIO_INVALID;
		orderp->status = 100;
		orderp->mcu_id = 0;
		orderp->next_print_node = 0;
		
		entry_index++;
	}		
}


/**
 * @fn		OrderEnqueue
 * @brief	缓冲区订单入队
 * @param	buf 缓冲区指针
 *				entry_index 索引号
 *				order_len 订单长度
 *				order_prio_sigal 紧急或者普通的标志
 * @return	void
 */
static s8_t OrderEnqueue(SqQueue* buf,s8_t entry_index , u16_t order_len,u8_t order_prio_sigal)
{		
	extern OS_EVENT *Print_Sem;    
	s8_t print_err = 0;	
	u16_t order_num = 0;
	order_info *orderp = &order_print_table.order_node[entry_index];
	u8_t *data = orderp->data;
	
	Add_Order_To_Print_Queue(buf,entry_index,order_prio_sigal); //分配内存					
//	DEBUG_PRINT_TIMME("订单批次内序号为：%u，订单编号为：%lu，订单长度为：%u，",orderp->batch_within_number,orderp->serial_number,orderp->size);//1ms中断一次*时钟节拍
//	ShowTime(orderp->sever_send_time,StartTime,OSTimeGet()*TIME_INTERVAL);
	//发送订单进入打印队列的报文	
	Order_QUEUE_Status_Send(&(order_print_table.order_node[entry_index]),ENQUEUE_OK);						
	ORDER_DEBUG_PRINT("-------ONE ORDER ENQUEUE---------\r\n");	

	OSSemPost(Print_Sem);//产生打印信号
		
	return 0;
}


/**
 * @fn		error_order_deal
 * @brief	订单错误，处理函数。
 * @param	buf	 缓冲区
 * @return	
 */
static s8_t error_order_deal(SqQueue* buf, s8_t order_prio_sigal)
{
	u32_t count = 1;
	//清除缓冲区内容
	while(buf->read != buf->write && (buf->base[buf->read] != 0x3e || buf->base[(buf->read+1)%buf->MAX] != 0X11 ))
	{
		buf->read = (buf->read +1) % buf->MAX;
		count++;
	}					
	buf->read = (buf->read +2) % buf->MAX;
  count ++;
	//更新缓冲容量
	
	if(order_prio_sigal == 1){//加急类型订单
		order_print_table.buf_node.urgent_buf_remain_capacity = order_print_table.buf_node.urgent_buf_remain_capacity  + count;
		if(order_print_table.buf_node.urgent_buf_remain_capacity == order_print_table.buf_node.urgent_buf_size)
		{
			buf->buf_empty = 1;
		}
	}
	else{//非加急类型订单
		order_print_table.buf_node.common_buf_remain_capacity = order_print_table.buf_node.common_buf_remain_capacity  + count;
		if(order_print_table.buf_node.common_buf_remain_capacity == order_print_table.buf_node.common_buf_size)
		{
			buf->buf_empty = 1;
		}
	
	}
		
	ORDER_DEBUG_PRINT("QUEUE_DEBUG : DELETE AN ERROR ORDER! ------\r\n");
}


/**
 * @fn		Print_Queue_Fun
 * @brief	打印队列任务函数。
 * @param	
 * @return	
 */
void Print_Queue_Fun()
{
	#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR  cpu_sr = 0u;
	#endif
	u8_t os_err = 0;
	s8_t print_err = 0;
	u16_t order_len = 0;    //需要打印的订单长度
	s8_t entry_index = 0;
	SqQueue *buf;
	u8_t order_prio_sigal = 0;
	u8_t data_source = 0;
	extern OS_EVENT *Print_Queue_Sem; 
	
	while(1){	
		OSSemPend(Print_Queue_Sem,0,&os_err);
		
		OSMutexPend(urgent_buf.mutex,0,&os_err);		//申请紧急缓冲锁
		OSMutexPend(queue_buf.mutex,0,&os_err);			//申请普通缓冲锁
		
		while(!Is_Empty_Queue(urgent_buf) || !Is_Empty_Queue(queue_buf))
		{
//			//这里进行7K判断，因为每入队一个订单，都有可能会让缓冲区减少一点。
//			Check_Buf_Request_Signal(queue_buf);
			
			if(!Is_Empty_Queue(urgent_buf)){	//加急订单
				buf = &urgent_buf;
				order_prio_sigal = 1;
			}
			else{															//普通订单
				buf = &queue_buf;
				order_prio_sigal = 0;
			}
			
			Read_Order_Length_Queue(*buf,&order_len);//获取订单长度			
			ORDER_DEBUG_PRINT("ORDER DEBUG : Order Length: %u------------\r\n", order_len);
			
//			找到订单队列中存放订单的位置			
//			返回值四种:		ORDER_TOO_LARGER、 ORDER_FIND_INDEX_ERR
//									或者正常的索引值，
//									或者是负数代表加急紧缺的内存块				
			entry_index = Find_Entry(order_len,order_prio_sigal);
			ORDER_DEBUG_PRINT("ORDER DEBUG : Order Index: %d------------\r\n", entry_index);
			if(entry_index == ORDER_TOO_LARGER){									//这里必须处理订单大小过大的问题
				ORDER_DEBUG_PRINT("ORDER_ERR: ORDER_TOO_LARGER\r\n");
				error_order_deal(buf, order_prio_sigal);
			}
			else if( entry_index == ORDER_FIND_INDEX_ERR){				//订单大小大于4K,但是又没有超过最长订单限制，（现在订单最长限制就为4K，所以理论上不可达）
				ORDER_DEBUG_PRINT("ORDER_ERR: ORDER_FIND_INDEX_ERR,  occur error when find the order index \r\n");	
				error_order_deal(buf , order_prio_sigal);
			}
			else if (entry_index < 0 ){														//表明紧急订单没有申请到内存块
				ORDER_DEBUG_PRINT("ORDER_WARNNING: NON BLOCK FOR UGRGENT ORDER\r\n");					
				OSSemPost(Print_Queue_Sem);	//因为加急订单并没有删除，所以还是存在缓冲中					
				
				ORDER_DEBUG_PRINT("QUEUE DEBUG :Before Non block Change\r\n");
				printOrderQueueSeque();						
				Change_Order_Seque(entry_index);//改变订单顺序，以让加急订单尽快加入到队列中)	
				ORDER_DEBUG_PRINT("QUEUE DEBUG :After  Non block Change\r\n");
				printOrderQueueSeque();				
				
				OSTimeDlyHMSM(0, 0, 1, 0);
			}
			else{																									//正常取得索引
				ORDER_DEBUG_PRINT("ORDER_SUCCESS: ENQUEUE SUCCESS \r\n");					
				OrderEnqueue(buf,entry_index , order_len,order_prio_sigal);
				//这里进行7K判断，因为每入队一个订单，都有可能会让缓冲区减少一点。
				Check_Buf_Request_Signal(queue_buf);
			}		
		}		
		OSMutexPost(urgent_buf.mutex);		
		OSMutexPost(queue_buf.mutex);		
	}
}


