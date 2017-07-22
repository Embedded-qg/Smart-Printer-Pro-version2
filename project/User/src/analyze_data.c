#include "analyze_data.h"

/**************************************************************
*	Function Define Section
**************************************************************/

/**
 * @brief 	获取批次序号哈希值
 */
u8_t get_batch_hash(u16_t batch_number)
{
	return (u8_t)(batch_number % MAX_BATCH_NUM);
}

/**
 * @brief 	获取批次长度
 */
u16_t get_batch_length(u16_t batch_number)
{
	return batch_info_table[get_batch_hash(batch_number)].batch_length;
}

/* 当数据在不同区域时，需要对两个段进行解析 */
void Analyze_Data_With_Diff_Part(u8_t *src1, int len1, u8_t *src2, int len2, u32_t *data)
{
	int i;
	
	*data = 0;
	
	for(i = 0; i < 2; ++i){
		// int len;
		//u16_t *data = (i == 0) ? (len = len1, src1: len = len2, src2);//可能不是很好看
		
		u8_t *src;
		int len;
		int j;
		
		if (i == 0){
			src = src1;
			len = len1;
		}else{
			src = src2;
			len = len2;
		}
		
		for(j = 0; j < len; ++j){
			*data <<= 8;
			*data |= *src++;
		}
	}
}
/****************************************************************************************
*@Name............: Analyze_Batch_Info_Table
*@Description.....: 对批次数据报解包
*@Parameters......: batch_data		:存放数据的字节流
*					batch_number	:批次号
*@Return values...: void
*****************************************************************************************/
void Analyze_Batch_Info_Table(char *batch_data, u16_t batch_number)
{
	u8_t hash;//批次表哈希值
	
	hash = get_batch_hash(batch_number);
	
	batch_info_table[hash].batch_number = batch_number;
	ANALYZE_DATA_2B((batch_data + BATCH_ORDER_NUMBER_OFFSET), batch_info_table[hash].order_number);//设立订单数目
	ANALYZE_DATA_2B((batch_data + BATCH_TOTAL_LENGTH_OFFSET), batch_info_table[hash].batch_length);//设立批次长度
	ANALYZE_DATA_4B((batch_data + BATCH_SEVER_SEND_TIME_OFFSET), batch_info_table[hash].sever_send_time);//设立服务器发送时间
	ANALYZE_DATA_4B((batch_data + BATCH_CHECK_SUM_OFFSET), batch_info_table[hash].check_sum);//设立校验和
	ANALYZE_DATA_2B((batch_data + BATCH_PRESERVATION_OFFSET), batch_info_table[hash].preservation);//设立保留值
	batch_info_table[hash].num_order_que = 0;
	batch_info_table[hash].num_printed_order = 0;
}

//寻找批次头，减少从网络缓冲读取的长度
void find_substr_head(char **data, char *substr, u16_t *len, u16_t sub_len)
{		
	char *sub_data = *data;
	for(; *len >= sub_len; (*len)--){
		int i = 0;
		for(; i < *len && sub_data[i] == substr[i]; i++)
			if(i == sub_len - 1){
				*data = sub_data;
				return ;
			}	
			
		sub_data++;
	}
	
	*len = 0;
}