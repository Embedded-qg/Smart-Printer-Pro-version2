#include "print_queue.h"

#define assert(expr, str)			 	\
	do {								\
		if(!(expr)) {					\
			printf((str));				\
			while(1);					\
		}								\
	}while(0)

u32 hour = 0, min = 0, sec = 0,msec = 0;//ʱ���� ���룬���ڻ�ȡ����ʱ��
INT32U StartTime = 0; //��׼ʱ��

/**
 * @name   	Find_Entry
 * @brief	 	���ݶ������ȣ��ҵ����ʵĽڵ㣬���ҷ������������Ҳ�����ڴ������
 * @param	  len 								��������
 *					order_prio_sigal 		�������ȼ�
 * @return	
 */
static s8_t Find_Entry(u16_t len , u8_t order_prio_sigal);


/**
 * @name   	Change_Normal_Order_Seque
 * @brief	 	�����䵽�Ľڵ㣬���뵽�����У����ּӼ�����ͨ��
 * @param	  entry_index ����������
 * @return	
 */
static s8_t Change_Normal_Order_Seque( s8_t entry_index );


/**
 * @name   	Change_Order_Seque
 * @brief	 	��ӡ���нڵ�����ʱ���мӼ�����������δ����ӣ�
						���Ծ��촦�����˳���üӼ�����������ӡ�
 * @param	  entry_index ����������
 * @return	
 */
static s8_t Change_Order_Seque(s8_t entry_index);


/**
 * @name   	Find_Block_Num
 * @brief	 	��Find_Entry���ã����ݶ�Ӧ���С�������ȼ�������������
 * @param	  blocksize 					��Ӧ�ڵ��С
 *					order_prio_sigal 		�������ȼ�
 * @return	
 */
static s8_t Find_Block_Num(u16_t blocksize ,u8_t order_prio_sigal);


/**
 * @name   	Find_Front_And_Next_Index
 * @brief	 	����size�Ľڵ�飬�ҵ���ȴ�С�Ľڵ���ǰ��������
 * @param	  start_index 				��ʼ���
 *					aim_size 						Ŀ���С
 *					front_index					ǰ������
 *					next_index					�������
 *					current_index				��ǰ����
 * @return	
 */
static s8_t Find_Front_And_Next_Index(s8_t start_index , u16_t aim_size,  u8_t *front_index , u8_t *next_index, u8_t *current_index);


/**
 * @fn		OrderEnqueue
 * @brief	�������������
 * @param	buf ������ָ��
 *				entry_index ������
 *				order_len ��������
 *				order_prio_sigal ����������ͨ�ı�־
 * @return	void
 */
static s8_t OrderEnqueue(SqQueue* buf,s8_t entry_index , u16_t order_len,u8_t order_prio_sigal);


/**
 * @brief	����ͨ������ӵ���ӡ������
 * @param	buf ��ͨ����������
			entry_index ��������
 * @return	�������
 */
static s8_t	Add_Order_To_Print_Queue(SqQueue *buf,s8_t entry_index , u8_t order_prio_sigal);

/**
 * @fn		error_order_deal
 * @brief	�������󣬴�������
 * @param	buf	 ������
 * @return	
 */
static s8_t error_order_deal(SqQueue* buf, s8_t order_prio_sigal);


/**
 * @fn		printOrderQueueSeque
 * @brief	���ڵ��������ӡ���е��б���Ϣ��
 * @param	buf	 ������
 * @return	
 */
static void printOrderQueueSeque();
	

//��ӡ���ж�����Ϣ��
order_print_queue_info order_print_table;        
//���α�
batch_info batch_info_table[MAX_BATCH_NUM];				 
//��ͬ��


/**
 * @brief	��ȡ����ʱ��
 * @param	��ӡ��Ԫ������
 * @return	ִ�н��
 */
void GetTime(u8_t *data)
{
	hour = 0;
	min = 0;
	sec = 0;
	msec = 0;
}


/**
 * @brief	����ʱ��
 * @param	time-���ӵĺ���
 * @return	ִ�н��
 */
void ShowTime(u32_t order_time, INT32U startTime, INT32U OSTime)
{
	u32_t hour,min,sec,msec,tmp;
	u32_t subTime = 0;
	hour = order_time/10000000;//ʱ
	min = (order_time/100000)%100;//��
	sec = (order_time/1000)%100;//��
	msec = order_time%1000;//����
	
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
	
	DEBUG_PRINT_TIMME("ʱ��Ϊ��%u:%u:%u:%u\r\n",hour, min, sec, msec);
}




/**
 * @name   	Find_Entry
 * @brief	 	���ݶ������ȣ��ҵ����ʵĽڵ㣬���ҷ������������Ҳ�����ڴ������
 * @param	  len 								��������
 *					order_prio_sigal 		�������ȼ�
 * @return	
 */
static s8_t Find_Entry(u16_t len , u8_t order_prio_sigal)
{
	if(len > BLOCK_MAX_SIZE)       //�������
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
 * @brief	 	��Find_Entry���ã����ݶ�Ӧ���С�������ȼ�������������
 * @param	  blocksize 					��Ӧ�ڵ��С
 *					order_prio_sigal 		�������ȼ�
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
			block_index_start = BLOCK_1K_INDEX_START;//��ʼ����
			block_index_end = BLOCK_1K_INDEX_END;//��������
			Lack_Num = Lack_Of_1K;	//ȱ���ڴ����ź�
			Block_Sem = Block_1K_Sem; //�ڴ��������ź���
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
	
	if(order_prio_sigal){//��������
		INT16U Accept_Sigal_num = 0;
		Accept_Sigal_num = OSSemAccept(Block_Sem);
		if( Accept_Sigal_num == 0){//֤���Ѿ�û���ڴ��,�����߳̽�������û���ڴ����źš�
			
			ORDER_DEBUG_PRINT("QUEUE DEBUG :  LACK OF BLOCK ,Lack_Num %d\r\n ",Lack_Num);
			OSTimeDlyHMSM(0, 0, 0, 500);
			return Lack_Num;
		}
	}
	else{//�ǽ�������
		OSSemPend(Block_Sem,0,&os_err);	//���޿����ڴ�飬������
		ORDER_DEBUG_PRINT("QUEUE DEBUG :  GOT Block_Sem-------------\r\n");
	}
	
//	ORDER_DEBUG_PRINT("QUEUE DEBUG :  acquire mutex of printqueue-----------\r\n");
	OSMutexPend(order_print_table.mutex,0,&os_err);//�ö�����Դ����
//	ORDER_DEBUG_PRINT("QUEUE DEBUG :  got mutex of printqueue-----------\r\n");
	entry_index = block_index_start;
	
	ORDER_DEBUG_PRINT("QUEUE DEBUG : entry_index: %u, block_index_start = %u,------- block_index_end = %u\r\n", entry_index, block_index_start, block_index_end);
	
	while(entry_index < block_index_end){//���ҿյ�������
		if(order_print_table.order_node[entry_index].data == NULL){			
			OSMutexPost(order_print_table.mutex);//�ͷŶ�����Դ����	
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
 * @brief	����ͨ������ӵ���ӡ������
 * @param	buf ��ͨ����������
			entry_index ��������
 * @return	�������
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
	order_head = buf->base + buf->read;// ��ȡ����ͷ//  �������ڴ��buf�Ķ���������������ͷ��
	
	OSMutexPend(order_print_table.mutex,0,&os_err);
	
	//��ʼ�����ڴ��
	if(entry_index < BLOCK_1K_INDEX_END){
		ORDER_DEBUG_PRINT("\nQUEUE DEBUG----GOT 1K BLOCK----\r\n");
		order_print_table.order_node[entry_index].data = OSMemGet(queue_1K,&os_err);
		serial_number = ((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +3 )));	//�������
		order_time = ((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +3 )));	//�·�ʱ��		
		
		DEBUG_PRINT_TIMME("�����ڴ��С��1K���������Ϊ��%u\r\n",serial_number);
//		ShowTime(order_time,StartTime,OSTimeGet()*TIME_INTERVAL);
	}
	else if(entry_index < BLOCK_2K_INDEX_END){
		ORDER_DEBUG_PRINT("\nQUEUE DEBUG----GOT 2K BLOCK----\r\n");
		order_print_table.order_node[entry_index].data = OSMemGet(queue_2K,&os_err);
		serial_number = ((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +3 )));	//�������
		order_time = ((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +3 )));	//�·�ʱ��		
		
		DEBUG_PRINT_TIMME("�����ڴ��С��2K���������Ϊ��%u\r\n",serial_number);
//		ShowTime(order_time,StartTime,OSTimeGet()*TIME_INTERVAL);		
	}
	else if(entry_index < BLOCK_4K_INDEX_END){
		ORDER_DEBUG_PRINT("\nQUEUE DEBUG----GOT 4K BLOCK----\r\n");
		order_print_table.order_node[entry_index].data = OSMemGet(queue_4K,&os_err);
		serial_number = ((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +3 )));	//�������
		order_time = ((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +3 )));	//�·�ʱ��		
		
		DEBUG_PRINT_TIMME("�����ڴ��С��4K���������Ϊ��%u\r\n",serial_number);
//		ShowTime(order_time,StartTime,OSTimeGet()*TIME_INTERVAL);
	}
	else if(entry_index < BLOCK_10K_INDEX_END){	
		ORDER_DEBUG_PRINT("\nQUEUE DEBUG----GOT 10K BLOCK----\r\n");
		order_print_table.order_node[entry_index].data = OSMemGet(queue_10K,&os_err);
		ORDER_DEBUG_PRINT("ORDER DATA POINTER: %p\r\n", order_print_table.order_node[entry_index].data);
		serial_number = ((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +3 )));	//�������
		order_time = ((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +1 )) << 16 ) 						
										+	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +3 )));	//�·�ʱ��		
		
		DEBUG_PRINT_TIMME("�����ڴ��С��10K���������Ϊ��%u\r\n",serial_number);
//		ShowTime(order_time,StartTime,OSTimeGet()*TIME_INTERVAL);	
	}
	
	/*������ȡ����ͷ������*/
	order_print_table.order_node[entry_index].size = 								(*(order_head + ORDER_SIZE_OFFSET) << 8) 						+ *(order_head + ORDER_SIZE_OFFSET + 1);											//��������	
	
	order_print_table.order_node[entry_index].mcu_id = 							((u32_t)(*(order_head + ORDER_MCU_ID_OFFSET)) << 24) 				+ 	((u32_t)(*(order_head + ORDER_MCU_ID_OFFSET +1 )) << 16 ) 						
																																	+	((u32_t)(*(order_head + ORDER_MCU_ID_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_MCU_ID_OFFSET +3 )));													//���ذ�ID
	
	order_print_table.order_node[entry_index].sever_send_time =  		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET)) << 24) 			+ 	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +1 )) << 16 ) 
																																	+	((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SEVER_SEND_TIME_OFFSET +3 )));				//ʱ��
	
	order_print_table.order_node[entry_index].serial_number = 			((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET)) << 24)				+ 	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +1 )) << 16 ) 
																																	+	((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +2 )) << 8) 	+		((u32_t)(*(order_head + ORDER_SERIAL_NUMBER_OFFSET +3 )));						//�������
	ORDER_DEBUG_PRINT("QUEUE DEBUG :id number :%u\r\n", order_print_table.order_node[entry_index].serial_number);
	
	order_print_table.order_node[entry_index].batch_number = 				(*(order_head+ORDER_BATCH_NUMBER_OFFSET)<<8)				+ *(order_head + ORDER_BATCH_NUMBER_OFFSET + 1);							//��������
	order_print_table.order_node[entry_index].batch_within_number =	(*(order_head+ORDER_BATCH_WITHIN_NUMBER_OFFSET)<<8)	+ *(order_head + ORDER_BATCH_WITHIN_NUMBER_OFFSET + 1);				//�����ڶ������
	
	DEBUG_PRINT("QUEUE DEBUG :id number :%u", order_print_table.order_node[entry_index].batch_number);
	DEBUG_PRINT("QUEUE DEBUG :id number :%u", order_print_table.order_node[entry_index].batch_within_number);
	
	bacth_number = order_print_table.order_node[entry_index].batch_number;
	hash = get_batch_hash(bacth_number);
	order_print_table.order_node[entry_index].arrTime = batch_info_table[hash].startTime;;
	order_print_table.order_node[entry_index].errorTime = 0;
	order_print_table.order_node[entry_index].finishTime = 0;
	
	order_print_table.order_node[entry_index].check_sum = 					(*(order_head+ORDER_CHECK_SUM_OFFSET)<<8)						+ *(order_head + ORDER_CHECK_SUM_OFFSET + 1);									//У����						//����
	order_print_table.order_node[entry_index].data_source = 	order_print_table.order_node[entry_index].preservation;
	order_print_table.order_node[entry_index].priority = order_prio_sigal;
  order_print_table.order_node[entry_index].next_print_node = -1;
	
	
	if(order_print_table.empty == 1){//��ԭ�ȴ�ӡ����Ϊ��
		order_print_table.head = entry_index;
		order_print_table.tail = entry_index;
		order_print_table.empty = 0;
	}
	else{
		//���Ĵ�ӡ������Ϣ��ȷ��������λ��˳��
		ORDER_DEBUG_PRINT("QUEUE DEBUG :Before Insert\r\n");
		printOrderQueueSeque();
		Change_Normal_Order_Seque(entry_index);
		ORDER_DEBUG_PRINT("QUEUE DEBUG :After Insert\r\n");
		printOrderQueueSeque();
	}
	
	//���������������ݼ����ӡ����
	Read_Order_Queue(buf,order_print_table.order_node[entry_index].data);
	
	//ʹ�����ӡ���У��ͷ���
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
 * @brief	 	�����䵽�Ľڵ㣬���뵽�����У����ּӼ�����ͨ��
 * @param	  entry_index ����������
 * @return	
 */
static s8_t Change_Normal_Order_Seque( s8_t entry_index )
{
	if(order_print_table.order_node[entry_index].priority == ORDER_PRIO_HIGH){//�Ӽ�����
		u8_t start_index = order_print_table.head;
		u8_t start_index_front = order_print_table.head;
		
		while(order_print_table.order_node[start_index].data != NULL && order_print_table.order_node[start_index].priority == ORDER_PRIO_HIGH){	//������Ӽ����С�
			start_index_front = start_index;
			start_index = order_print_table.order_node[start_index].next_print_node;
		}
		if(start_index == start_index_front){//�����Ӷ�ͷ��ʼ���Ͳ��ǼӼ�����			
			order_print_table.order_node[entry_index].next_print_node = order_print_table.head;
			order_print_table.head = entry_index;							
			return 0;
		}
		else{//�����������мӼ�������
			order_print_table.order_node[entry_index].next_print_node = order_print_table.order_node[start_index_front].next_print_node;
			order_print_table.order_node[start_index_front].next_print_node = entry_index;
			if(order_print_table.order_node[start_index].data == NULL){			//�����ӡ����ֻʣ�¼Ӽ�����
				order_print_table.tail = entry_index;
				order_print_table.order_node[entry_index].next_print_node = -1;
			}
			return 0;			
		}
	}
	else{//��ͨ����
		order_print_table.order_node[order_print_table.tail].next_print_node = entry_index;
		order_print_table.tail = entry_index;
	}
}


/**
 * @name   	Change_Order_Seque
 * @brief	 	��ӡ���нڵ�����ʱ���мӼ�����������δ����ӣ�
						���Ծ��촦�����˳���üӼ�����������ӡ�
 * @param	  entry_index ����������
 * @return	
 */
static s8_t Change_Order_Seque(s8_t entry_index)
{
	u8_t next_index;	//�������ͬ�ڴ��������ĺ�һ��
	u8_t front_index;	//�������ͬ�ڴ���������ǰһ��
	u8_t current_index; //�������ͬ�ڴ�������
	u16_t block_size = 0;
	s8_t same_block_index = 0;
	u16_t printing_size = 0;
	
	if(entry_index < 0){	//�����ж�
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
		
		if(printing_size != block_size){	//��ͷ���ǲ���ͬ�ڴ�Ķ�������Ҫ�����滻
			u8_t start_index = order_print_table.head;
			u8_t start_index_front = order_print_table.head;
			u8_t in_urgent_order_flag = 0;
			while(order_print_table.order_node[start_index].priority == ORDER_PRIO_HIGH){	//������Ӽ����С�
				if(order_print_table.order_node[start_index].size == block_size){
					in_urgent_order_flag = 1;
					break;
				}
				if(start_index == order_print_table.tail )
					break;
				start_index_front = start_index;				
				start_index = order_print_table.order_node[start_index].next_print_node;
			}
			
			
			if(start_index == start_index_front && order_print_table.order_node[start_index].priority != ORDER_PRIO_HIGH){//�����Ӷ�ͷ��ʼ���Ͳ��ǼӼ�����
				
				
				Find_Front_And_Next_Index(start_index, block_size, &next_index , &front_index, &current_index);//�ҵ������ͷ���Ǹ���ͬ�ڴ�Ķ�������������������
				order_print_table.order_node[front_index].next_print_node = next_index;
				order_print_table.order_node[current_index].next_print_node = order_print_table.head;						
				order_print_table.head = current_index;
				
				ORDER_DEBUG_PRINT("QUEUE DEBUG : head change \r\n" );
				return 0;
			}
			else{//�����������мӼ�������
				if(in_urgent_order_flag == 0){//��ͬ�ڴ�����������ڼӼ��������С�
					if(start_index == order_print_table.tail)
					{
						
					ORDER_DEBUG_PRINT("QUEUE DEBUG : start_index %u\r\n" , start_index );
						return 0;
					}
					Find_Front_And_Next_Index(start_index, block_size, &next_index , &front_index, &current_index);//�ҵ������ͷ���Ǹ���ͬ�ڴ�Ķ�������������������
					
					if(front_index == current_index){				
						//��ͨ������ӡ�����еĵ�һ��������ͬ��С���ڴ�飬���Բ��ý��в�����
						
					ORDER_DEBUG_PRINT("QUEUE DEBUG : first do not change \r\n");
						return 0;
					}
					else{					
						//��ͨ������ӡ�����еĵ�һ�������ڴ����ͬ��С�Ķ�������Ҫ����ͬ��С�Ķ���������ͨ��ӡ���е��ײ���
						order_print_table.order_node[front_index].next_print_node = next_index;
						order_print_table.order_node[current_index].next_print_node = start_index;						
						order_print_table.order_node[start_index_front].next_print_node = current_index;	
					}				
				}				
				else{
					//��ͬ���ڴ浥Ԫ�Ķ������ǼӼ�����������Ҫ���д���
					ORDER_DEBUG_PRINT("QUEUE DEBUG : same do not change\r\n" );
					return 0;
				}
			}
		}
		else{		
			return 0; //��ͷ������ͬ�ڴ浥Ԫ�������ȴ����ӡ��ɼ��ɡ�
		}
	}
}


/**
 * @name   	Find_Front_And_Next_Index
 * @brief	 	����size�Ľڵ�飬�ҵ���ȴ�С�Ľڵ���ǰ��������
 * @param	  start_index 				��ʼ���
 *					aim_size 						Ŀ���С
 *					front_index					ǰ������
 *					next_index					�������
 *					current_index				��ǰ����
 * @return	
 */
static s8_t Find_Front_And_Next_Index(s8_t start_index , u16_t aim_size,  u8_t *front_index , u8_t *next_index ,u8_t *current_index)
{
	u8_t front = start_index;
	u8_t next = order_print_table.order_node[start_index].next_print_node; 
	u8_t current = start_index;
	while(next != order_print_table.tail){		 
		if(order_print_table.order_node[current].size == aim_size ){//����
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
 * @brief	ɾ��һ�ݶ������黹�ڴ���ϵͳ
 * @param	��������
 * @return	ִ�н��
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
	/*�ͷ��ڴ��*/		
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
//	DEBUG_PRINT_TIMME("�ͷ��ڴ�飬�������Ϊ��");//1ms�ж�һ��*ʱ�ӽ���
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
 * @brief	�����ӡ����ͷ��㲢��������������entryp��ָ�ռ�
 * @param	entryp	��Ż�õĽ�������
 * @return	ORDER_QUEUE_OK	��ȡ�ɹ�
			ORDER_QUEUE_EMPTY ����Ϊ�գ���ȡʧ��
 */
s8_t GetOrderFromQueue(u8_t *entryp)
{
	INT8U err;
	
	OSMutexPend(order_print_table.mutex, 0, &err);
	
	if(order_print_table.empty == 1) {
		OSMutexPost(order_print_table.mutex);		
		return ORDER_QUEUE_EMPTY;
	}
	*entryp = order_print_table.head;	//ȡ���е�һ�ݶ���
	if(order_print_table.head == order_print_table.tail) {	//�����н���һ�ݶ�����ȡ�ߺ����Ϊ��
		order_print_table.empty = 1;
		order_print_table.head = order_print_table.tail = 0;
	}else {		//�������ж���һ�ݶ���
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
 * @brief	��鶩���������Ƿ��д�
 * @param	��������
 * @return	ORDER_DATA_ERR	����������
			ORDER_DATA_OK	��������ȷ
 */
s8_t CheckOrderData(u8_t entry_index)
{
	u16_t pbytes, length;
	u8_t type;
	order_info *orderp = &order_print_table.order_node[entry_index];
	u8_t *datap = orderp->data;	
	
	if(NULL == datap) {
		orderp->status = PRINT_STATUS_DATA_ERR;	/* �������ݴ��� */
		DEBUG_PRINT_TIMME("��ӡʧ�ܣ��������ݴ���\r\n");
//		ShowTime(orderp->sever_send_time,StartTime,OSTimeGet()*TIME_INTERVAL);
		Order_Print_Status_Send(orderp,PRINT_STATUS_DATA_ERR);
		ORDER_DEBUG_PRINT("CheckOrderData: checking empty block!\r\n");
		return ORDER_DATA_ERR;		
	}
	pbytes = 0;
	while(pbytes < orderp->size) {	// ������������
		length = GetDataLength(datap);
		type = GetDataType(datap);
 
		if(type == DATA_INVALID){	// ���ݴ���			
			orderp->status = PRINT_STATUS_DATA_ERR;	/* �������ݴ��� */
			Order_Print_Status_Send(orderp,PRINT_STATUS_DATA_ERR);

			ORDER_DEBUG_PRINT("CheckOrderData: Invalid type of data field!\r\n");
			
			return ORDER_DATA_ERR;
		}
		datap = MoveToNextData(datap, length);	// �ƶ�����һ������
		pbytes += length + DATA_HEADER_SIZE + DATA_FOOTER_SIZE;
	}
	if(pbytes == 0)	// �����ݶ���
		return ORDER_DATA_ERR;
	return ORDER_DATA_OK;
}


/**
 * @brief	��ӡһ�ݶ��������޸���״̬
 * @param	��ӡ��Ԫ����
 * @return	ִ�н��
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

	if(needCutPaper[cellno]) {	// �г���ӡ���������ڴ�ӡ�����ӶϿ����Դ���ϵ��쳣����ӡ�Ĵ��󶩵�
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
	
	// ��ҵ���ʱ�����2s����Ϊ��ӡ��Ԫû����������
	if(cellp->beginTick - cellp->endTick > 2000/20)
		cellp->workedTime = 0;

	pbytes = 0;		
	while(pbytes < orderp->size) {	//�����������ݲ���ӡ
		length = GetDataLength(datap);
		type = GetDataType(datap);
 
		if(DATA_IS_PLAINTEXT == type) {		// ��������
			ORDER_DEBUG_PRINT("Print_Order: Printing Paint Text\r\n");
			//DMA_To_USARTx_Send(datap + DATA_HEADER_SIZE, length, cellno);
			mySendData(datap + DATA_HEADER_SIZE, length, cellno);
		}
		else if(DATA_IS_IMAGE == type) {	// ͼƬ����		

			ORDER_DEBUG_PRINT("Print_Order: Printing Picture\r\n");
			OSTimeDlyHMSM(0,0,2,0);	
			printImages(datap + DATA_HEADER_SIZE, GetImageBytes(datap, length), cellno);
			
		}
		else if(DATA_IS_QRCODE == type) {	// ��ά������		
//			OSTimeDlyHMSM(0,0,0,500);
			printQRCode(datap + DATA_HEADER_SIZE, length, cellno);
			ORDER_DEBUG_PRINT("Print_OrderPrinting: QR Code\r\n");
		}else {
			ORDER_DEBUG_PRINT("BUG DETECT: Print_Order: DATA CHECKING WRONG.\r\n");
		}
		pbytes += length + DATA_HEADER_SIZE + DATA_FOOTER_SIZE; //�Ѿ�������ֽ���
		PCMgr.cells[cellno-1].totalLength += pbytes;
		
		datap = MoveToNextData(datap, length);
	}
	orderp->status = ORDER_DATA_OK;	// �������ݽ�����ȷ
	
	// ���ͼ���ӡ��״̬��ָ��Ծ����������Ƿ��ӡ�ɹ�
	OSTimeDlyHMSM(0,0,(u8)((orderp->size / 512) + (orderp->size / 2048)), 500);
	ORDER_DEBUG_PRINT("Print_Order: Waiting for Printer's status.\r\n");
	SEND_STATUS_CMD_ONE(cellno);
	OSTimeDlyHMSM(0,0,1,0);//����״̬����ʱ
	
	// ���������Pend��������̷��أ�����ʱ������ӡ�����д������쳣
	OSSemPend(cellp->printDoneSem, 20, &err);
	if(err == OS_ERR_TIMEOUT){	// ��ӡ�����г����쳣��ʹ�ô�ӡ������
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
	
	/*��ʼ����Ϣ���еĻ�������¼*/
	order_print_table.buf_node.common_buf_size = MAXQSIZE;
	order_print_table.buf_node.common_buf_remain_capacity = MAXQSIZE;
	
	order_print_table.buf_node.urgent_buf_size = MAXUSIZE;
	order_print_table.buf_node.urgent_buf_remain_capacity = MAXUSIZE;	
	
	/* ��ʼ�������� */
	order_print_table.mutex = OSMutexCreate(ORDER_PRINT_TABLE_MUTEX_PRIO, &err);
	assert(NULL != order_print_table.mutex, 
		"Init_Order_Table: Create mutex for print table failed.");
	
	/* ��ʼ�����й���ṹ */
	order_print_table.head = 0;   //��ӡ���ж�������
	order_print_table.tail = 0;   //��ӡ���ж�β����
	order_print_table.empty = 1;  //��ӡ�����Ƿ�Ϊ�գ�1����ա�0����ǿ�

	
	/*��ʼ����Ϣ���еĶ�����¼*/
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
 * @brief	�������������
 * @param	buf ������ָ��
 *				entry_index ������
 *				order_len ��������
 *				order_prio_sigal ����������ͨ�ı�־
 * @return	void
 */
static s8_t OrderEnqueue(SqQueue* buf,s8_t entry_index , u16_t order_len,u8_t order_prio_sigal)
{		
	extern OS_EVENT *Print_Sem;    
	s8_t print_err = 0;	
	u16_t order_num = 0;
	order_info *orderp = &order_print_table.order_node[entry_index];
	u8_t *data = orderp->data;
	
	Add_Order_To_Print_Queue(buf,entry_index,order_prio_sigal); //�����ڴ�					
//	DEBUG_PRINT_TIMME("�������������Ϊ��%u���������Ϊ��%lu����������Ϊ��%u��",orderp->batch_within_number,orderp->serial_number,orderp->size);//1ms�ж�һ��*ʱ�ӽ���
//	ShowTime(orderp->sever_send_time,StartTime,OSTimeGet()*TIME_INTERVAL);
	//���Ͷ��������ӡ���еı���	
	Order_QUEUE_Status_Send(&(order_print_table.order_node[entry_index]),ENQUEUE_OK);						
	ORDER_DEBUG_PRINT("-------ONE ORDER ENQUEUE---------\r\n");	

	OSSemPost(Print_Sem);//������ӡ�ź�
		
	return 0;
}


/**
 * @fn		error_order_deal
 * @brief	�������󣬴�������
 * @param	buf	 ������
 * @return	
 */
static s8_t error_order_deal(SqQueue* buf, s8_t order_prio_sigal)
{
	u32_t count = 1;
	//�������������
	while(buf->read != buf->write && (buf->base[buf->read] != 0x3e || buf->base[(buf->read+1)%buf->MAX] != 0X11 ))
	{
		buf->read = (buf->read +1) % buf->MAX;
		count++;
	}					
	buf->read = (buf->read +2) % buf->MAX;
  count ++;
	//���»�������
	
	if(order_prio_sigal == 1){//�Ӽ����Ͷ���
		order_print_table.buf_node.urgent_buf_remain_capacity = order_print_table.buf_node.urgent_buf_remain_capacity  + count;
		if(order_print_table.buf_node.urgent_buf_remain_capacity == order_print_table.buf_node.urgent_buf_size)
		{
			buf->buf_empty = 1;
		}
	}
	else{//�ǼӼ����Ͷ���
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
 * @brief	��ӡ������������
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
	u16_t order_len = 0;    //��Ҫ��ӡ�Ķ�������
	s8_t entry_index = 0;
	SqQueue *buf;
	u8_t order_prio_sigal = 0;
	u8_t data_source = 0;
	extern OS_EVENT *Print_Queue_Sem; 
	
	while(1){	
		OSSemPend(Print_Queue_Sem,0,&os_err);
		
		OSMutexPend(urgent_buf.mutex,0,&os_err);		//�������������
		OSMutexPend(queue_buf.mutex,0,&os_err);			//������ͨ������
		
		while(!Is_Empty_Queue(urgent_buf) || !Is_Empty_Queue(queue_buf))
		{
//			//�������7K�жϣ���Ϊÿ���һ�����������п��ܻ��û���������һ�㡣
//			Check_Buf_Request_Signal(queue_buf);
			
			if(!Is_Empty_Queue(urgent_buf)){	//�Ӽ�����
				buf = &urgent_buf;
				order_prio_sigal = 1;
			}
			else{															//��ͨ����
				buf = &queue_buf;
				order_prio_sigal = 0;
			}
			
			Read_Order_Length_Queue(*buf,&order_len);//��ȡ��������			
			ORDER_DEBUG_PRINT("ORDER DEBUG : Order Length: %u------------\r\n", order_len);
			
//			�ҵ����������д�Ŷ�����λ��			
//			����ֵ����:		ORDER_TOO_LARGER�� ORDER_FIND_INDEX_ERR
//									��������������ֵ��
//									�����Ǹ�������Ӽ���ȱ���ڴ��				
			entry_index = Find_Entry(order_len,order_prio_sigal);
			ORDER_DEBUG_PRINT("ORDER DEBUG : Order Index: %d------------\r\n", entry_index);
			if(entry_index == ORDER_TOO_LARGER){									//������봦������С���������
				ORDER_DEBUG_PRINT("ORDER_ERR: ORDER_TOO_LARGER\r\n");
				error_order_deal(buf, order_prio_sigal);
			}
			else if( entry_index == ORDER_FIND_INDEX_ERR){				//������С����4K,������û�г�����������ƣ������ڶ�������ƾ�Ϊ4K�����������ϲ��ɴ
				ORDER_DEBUG_PRINT("ORDER_ERR: ORDER_FIND_INDEX_ERR,  occur error when find the order index \r\n");	
				error_order_deal(buf , order_prio_sigal);
			}
			else if (entry_index < 0 ){														//������������û�����뵽�ڴ��
				ORDER_DEBUG_PRINT("ORDER_WARNNING: NON BLOCK FOR UGRGENT ORDER\r\n");					
				OSSemPost(Print_Queue_Sem);	//��Ϊ�Ӽ�������û��ɾ�������Ի��Ǵ��ڻ�����					
				
				ORDER_DEBUG_PRINT("QUEUE DEBUG :Before Non block Change\r\n");
				printOrderQueueSeque();						
				Change_Order_Seque(entry_index);//�ı䶩��˳�����üӼ�����������뵽������)	
				ORDER_DEBUG_PRINT("QUEUE DEBUG :After  Non block Change\r\n");
				printOrderQueueSeque();				
				
				OSTimeDlyHMSM(0, 0, 1, 0);
			}
			else{																									//����ȡ������
				ORDER_DEBUG_PRINT("ORDER_SUCCESS: ENQUEUE SUCCESS \r\n");					
				OrderEnqueue(buf,entry_index , order_len,order_prio_sigal);
				//�������7K�жϣ���Ϊÿ���һ�����������п��ܻ��û���������һ�㡣
				Check_Buf_Request_Signal(queue_buf);
			}		
		}		
		OSMutexPost(urgent_buf.mutex);		
		OSMutexPost(queue_buf.mutex);		
	}
}


