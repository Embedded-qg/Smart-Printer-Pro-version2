#include "analyze_data.h"

/**************************************************************
*	Function Define Section
**************************************************************/
extern u16_t Data_long;
extern u16_t Flag_receive_order;
u16_t batch_order_already_print;

extern contract_info contract_information;
extern u8_t batch_flag;
extern u8_t batch_table_hash;

extern INT32U StartTime; //��׼ʱ��
/**
 * @brief 	��ȡ������Ź�ϣֵ
 */
u8_t get_batch_hash(u16_t batch_number)
{
	return (u8_t)(batch_number % MAX_BATCH_NUM);
}

/**
*@brief ��ȡ��ͬ��Ź�ϣֵ
*/
u8_t get_contract_hash(u16_t contract_number)
{
	return (u8_t)(contract_number % MAX_CONTRACT_NUM);
}
/**
 * @brief 	��ȡ���γ���
 */
u16_t get_batch_length(u16_t batch_number)
{
	return batch_info_table[get_batch_hash(batch_number)].batch_length;
}

/* �������ڲ�ͬ����ʱ����Ҫ�������ν��н��� */
void Analyze_Data_With_Diff_Part(u8_t *src1, int len1, u8_t *src2, int len2, u32_t *data)
{
	int i;
	
	*data = 0;
	
	for(i = 0; i < 2; ++i){
		// int len;
		//u16_t *data = (i == 0) ? (len = len1, src1: len = len2, src2);//���ܲ��Ǻܺÿ�
		
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
@Description.......:�Ժ�ͬ���ݱ����
@Parameters........:contract_data   :������ݵ��ֽ���
										contract_number:���α��
return values.......:void
****************************************************************************************/
void Analyze_Contract_Info_Table(char* contract_data)
{
	ANALYZE_DATA_2B((contract_data + CONTRACT_CONTRACT_NUMBER_OFFSET), contract_information.contract_number);//��ͬ��
	ANALYZE_DATA_4B((contract_data + CONTRACT_SERVER_SEND_TIME_OFFSET), contract_information.sever_send_time);//��������������ʱ��
	ANALYZE_DATA_2B((contract_data + CONTRACT_CHECK_SUM_OFFSET), contract_information.check_sum);//����У���
}
/****************************************************************************************
*@Name............: Analyze_Batch_Info_Table
*@Description.....: ���������ݱ����
*@Parameters......: batch_data		:������ݵ��ֽ���
*					batch_number	:���κ�
*@Return values...: void
*****************************************************************************************/
void Analyze_Batch_Info_Table(char *batch_data, u16_t batch_number)
{
	u8_t hash;//���α��ϣֵ
	
	hash = get_batch_hash(batch_number);
	batch_info_table[hash].batch_number = batch_number;
	ANALYZE_DATA_2B((batch_data + BATCH_ORDER_NUMBER_OFFSET), batch_info_table[hash].order_number);//����������Ŀ
	ANALYZE_DATA_2B((batch_data + BATCH_TOTAL_LENGTH_OFFSET), batch_info_table[hash].batch_length);//�������γ���
	DEBUG_PRINT_STATEGY("before batch_order_already_print = %d\r\n",batch_order_already_print);
	Data_long = batch_info_table[hash].order_number + batch_order_already_print;//��ȡ��������Ŀ
	batch_order_already_print = Data_long;//�����¼��batch_order_already_print����ӡ��һ�ݾͼ�1
	Flag_receive_order = 1;//˵���ѻ�ȡ���ζ�����Ŀ
//	DEBUG_PRINT_STATEGY("Data_long = %d\r\n",Data_long);
	DEBUG_PRINT_STATEGY("after  batch_order_already_print = %d\r\n",batch_order_already_print);
	ANALYZE_DATA_4B((batch_data + BATCH_SEVER_SEND_TIME_OFFSET), batch_info_table[hash].sever_send_time);//��������������ʱ��
	ANALYZE_DATA_4B((batch_data + BATCH_CHECK_SUM_OFFSET), batch_info_table[hash].check_sum);//����У���
	ANALYZE_DATA_2B((batch_data + BATCH_PRESERVATION_OFFSET), batch_info_table[hash].preservation);//��������ֵ
	batch_info_table[hash].startTime = OSTimeGet();
	batch_info_table[hash].num_order_que = 0;
	batch_info_table[hash].num_printed_order = 0;
}

//�ж����ݱ��ĵ�����
u8_t find_order_head(char **data,u16_t *len)
{
	u8_t  netbuf_type = 0;
	char *sub_data = *data;
	u16_t i;
	for(i = 0;i < (*len) - 1;i++,(*len)--)
	{
		if(sub_data[i] == '\xbf' && sub_data[i + 1] == '\xfb')
		{
			netbuf_type = NETORDER_TYPE_CONTRACT;
//			ANALYZE_DEBUG_PRINT("����һ����ͬ������\r\n");
			break;
		}			
		if(sub_data[i] == '\xaa' && sub_data[i + 1] == '\x55')
		{
			netbuf_type = NETORDER_TYPE_BATCH;
//			ANALYZE_DEBUG_PRINT("����һ�����α���\r\n");
			break;
		}
		if(sub_data[i] == '\x3e' && sub_data[i + 1] == '\x11')
		{
			netbuf_type = NETORDER_TYPE_ORDER;
			ANALYZE_DEBUG_PRINT("����һ���������ݱ���\r\n");
//			StartTime = OSTimeGet()*TIME_INTERVAL; //��¼��׼ʱ��ֵ
			break;
		}
	}
	if(!netbuf_type)  (*len) = 0; 
	*data = sub_data + i;
	return netbuf_type;
}

//Ѱ������ͷ�����ٴ����绺���ȡ�ĳ���
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