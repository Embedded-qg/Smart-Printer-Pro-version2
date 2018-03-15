#include "more_infomation.h"

u16_t cell_cutCnt[MAX_CELL_NUM] = {0};
extern INT32U StartTime[100]; //基准时间
extern void ShowTime(u32_t order_time, u32_t startTime1, u32_t OSTime);

//udp校验和算法
u16_t Check_Sum(u16_t *data, int len)
{
	u32_t sum = 0;
	
	while(len > 1){
		sum += *data++;
		len -= 2;
	}
	
	/*
	//理论上表头都是对齐4字节，所以没有len == 0的情况
	if(len){
		sum += *(u8_t*)data;
	}
	*/
	
	sum  = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	
	return (u16_t)(~sum);
}

void Add_Sum(u32_t *sum, u16_t *src, int len)
{
	while(len > 1){
		*sum += *src++;
		len -= 2;
	}
	
	if(len){
		*sum += *(u8_t*)src;
	}
}

/* 当数据在不同区域时，需要对两个段进行校验和 */
u16_t Check_Sum_With_Diff_Part(u16_t *src1, int len1, u16_t *src2, int len2)
{
	u32_t sum = 0;
	int i;
	
	for(i = 0; i < 2; ++i){
		// int len;
		//u16_t *data = (i == 0) ? (len = len1, src1: len = len2, src2);//可能不是很好看
		
		u16_t *data;
		int len;
		
		if (i == 0){
			data = src1;
			len = len1;
		}else{
			data = src2;
			len = len2;
		}
		
		Add_Sum(&sum, data, len); //校验和累加
	}
	
	sum  = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	
	return (u16_t)(~sum);
}


//获取主控板id
u32_t Get_MCU_ID(void)
{
	return 3;
}

//获取主控板速度
u16_t Get_MCU_Speed(void)
{
	u16_t mcu_Speed = 0,i;
	for(i = 0;i < MAX_CELL_NUM;i++)
	{
		 if(PCMgr.cells[i].status != PRINT_CELL_STATUS_ERR) mcu_Speed++;
	}
//	printf("MCU_SPEED = %d\r\n",mcu_Speed);
	return mcu_Speed;
}

u16_t Get_MCU_MaxBufSize(void)
{
	return MAXQSIZE;//获取主控板缓冲区的最大大小
}

//获取主控板状态
u32_t Get_MCU_Status(void)
{
	u8_t i,health_cellNum = 0;
	u32_t mcu_health;
	u32_t totalTime,workedTime,cutCnt;
	u32_t total = 3 * 3600 * 10;
	extern  PrintCellsMgrInfo PCMgr;
	for(i = 0,mcu_health = 0;i < MAX_CELL_NUM;i++)
	{
		if(PCMgr.cells[i].status == PRINT_CELL_STATUS_ERR) continue;
		workedTime = PCMgr.cells[i].workedTime;
		mcu_health += (total - workedTime) * (100 - 5 * cutCnt);
		health_cellNum++;
//		mcu_health += (total - totalTime / 20 - 5 * workedTime / 20) / MAX_CELL_NUM; /** (100 - cell_unhealth[i]);*/
	}
	mcu_health = mcu_health/health_cellNum;
	return mcu_health;
}

//获取时间戳
u32_t Get_Current_Unix_Time(void)
{
	return 0;
}

//字节填充0
void Fill_Blank(char *add, u8_t len)
{
	int i;
	for(i = 0; i < len; ++i)
		*add++ = 0;
}

//获取批次时间戳
u32_t Get_Batch_Unix_Time(u16_t batch_number)
{
	return 0;
}

//获取订单时间戳
u32_t Get_Order_Unix_Time(u32_t order_number)
{
	return 0;
}


/*
	判断缓冲区数据格式是否正确（起始位，订单长度）
	若返回0，则表示正确
	若返回1，则表示错误
*/
s8_t checkBufData(SqQueue *buf , u32_t writePtr)
{
	s8_t err = 0;
	u32_t index = writePtr;
	u32_t bufLength  = 0;
	u32_t serial_number = 0;
	u32_t order_time = 0;
	u8_t *order_head = NULL;
	u16_t order_length = 0;
	u16_t order_batch_number = 0;
	
	order_head = buf->base + buf->read;// 获取订单头
	ANALYZE_DATA_4B((order_head + ORDER_SERIAL_NUMBER_OFFSET), serial_number);//获取订单编号
	ANALYZE_DATA_4B((order_head + ORDER_SEVER_SEND_TIME_OFFSET), order_time);//获取订单下发时间
	ANALYZE_DATA_2B((order_head + ORDER_BATCH_NUMBER_OFFSET), order_batch_number);//获取订单所属批次号

	DEBUG_PRINT_TIMME("批次号为：[%u]，校验订单数据，",order_batch_number);
	ShowTime(order_time,StartTime[order_batch_number%100],OSTimeGet()*TIME_INTERVAL);

	while(index != buf->write){
		if(buf->base[index] != 0x3e && buf->base[index+1]%buf->MAX != 0x11){			
			DEBUG_PRINT("BUF Head is not the Order Head ( 0x3e , 0x11)\n");
			err =1;
			break;
		}
		
		bufLength = buf->base[(index + ORDER_SIZE_OFFSET)%buf->MAX] << 8 | buf->base[(index+ ORDER_SIZE_OFFSET + 1)%buf->MAX];
		
		if(buf->base[(index + BUF_HEAD + bufLength + BUF_END - 2) % buf->MAX] != 0x11 || 
			buf->base[(index + BUF_HEAD + bufLength + BUF_END - 1) % buf->MAX] != 0xe3)
		{		
			DEBUG_PRINT("BUF Tail is not the Order Tail ( 0x11 , 0xe3)\n");
			err = 1;
			break;
		}
		index = (index + BUF_HEAD + bufLength + BUF_END) % buf->MAX;
	}
	return err ;
}