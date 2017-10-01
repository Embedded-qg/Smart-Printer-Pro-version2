#include "more_infomation.h"


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
u32_t Get_Printer_ID(void)
{
	return 1;
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