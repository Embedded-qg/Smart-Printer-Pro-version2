#include "analyze_data.h"

/**************************************************************
*	Function Define Section
**************************************************************/

extern contract_info contract_information;
extern u8_t batch_flag;
extern u8_t batch_table_hash;
/**
 * @brief 	获取批次序号哈希值
 */
u8_t get_batch_hash(u16_t batch_number)
{
	return (u8_t)(batch_number % MAX_BATCH_NUM);
}

/**
*@brief 获取合同序号哈希值
*/
u8_t get_contract_hash(u16_t contract_number)
{
	return (u8_t)(contract_number % MAX_CONTRACT_NUM);
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
*@name.............:Analyze_Contract_Info_Table
@Description.......:对合同数据报解包
@Parameters........:contract_data   :存放数据的字节流
										contract_number:批次编号
return values.......:void
****************************************************************************************/
void Analyze_Contract_Info_Table(char* contract_data)
{
	ANALYZE_DATA_2B((contract_data + CONTRACT_CONTRACT_NUMBER_OFFSET), contract_information.contract_number);//合同编号
	ANALYZE_DATA_4B((contract_data + CONTRACT_SERVER_SEND_TIME_OFFSET), contract_information.sever_send_time);//设立服务器发送时间
	ANALYZE_DATA_2B((contract_data + CONTRACT_CHECK_SUM_OFFSET), contract_information.check_sum);//设立校验和
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
	printf("批次长度为%d\r\n", batch_info_table[hash].batch_length);
	ANALYZE_DATA_4B((batch_data + BATCH_SEVER_SEND_TIME_OFFSET), batch_info_table[hash].sever_send_time);//设立服务器发送时间
	ANALYZE_DATA_4B((batch_data + BATCH_CHECK_SUM_OFFSET), batch_info_table[hash].check_sum);//设立校验和
	ANALYZE_DATA_2B((batch_data + BATCH_PRESERVATION_OFFSET), batch_info_table[hash].preservation);//设立保留值
	batch_info_table[hash].num_order_que = 0;
	batch_info_table[hash].num_printed_order = 0;
}

//判断数据报文的类型
u8_t find_order_head(char **data,u16_t *len)
{
	u8_t  netbuf_type = 0;
	char *sub_data = *data;
	u16_t i;
	for(i = 0;i < (*len) - 1;i++,(*len)--)
	{
		if(sub_data[i] == '\xbf' && sub_data[i + 1] == '\xfb')
		{
			netbuf_type = NETORDER_CONTRACT;
			printf("这是一个合同网报文\r\n");
			break;
		}			
		if(sub_data[i] == '\xaa' && sub_data[i + 1] == '\x55')
		{
			netbuf_type = NETOREDER_BATCH;
			printf("这是一个批次报文\r\n");
			break;
		}
		if(sub_data[i] == '\x3e' && sub_data[i + 1] == '\x11')
		{
			netbuf_type = NETOREDER_ORDER;
			printf("这是一个订单数据报文\r\n");
			break;
		}
	}
	*data = sub_data + i;
	return netbuf_type;
}

//根据报文类型分析其长度
u16_t anylyze_order_length(u8_t netbuf_type,char *data)
{
	u16_t max_length;
	if(netbuf_type == NETORDER_CONTRACT)
	{
		max_length = MAX_CONTRACT_HEAD_LENGTH;
	}
	else if(netbuf_type == NETOREDER_BATCH)
	{
		max_length = MAX_BATCH_HEAD_LENGTH;
	}
	else if(netbuf_type == NETOREDER_ORDER)
	{
		if(batch_flag == 1) max_length = batch_info_table[batch_table_hash].batch_length - MAX_BATCH_HEAD_LENGTH;
	}
	return max_length;
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