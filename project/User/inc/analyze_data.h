#ifndef ANALYZE_DATA_H
#define ANALYZE_DATA_H

/**************************************************************
*	Include File Section
**************************************************************/
#include "cc.h"
#include "more_infomation.h"
#include "data_form.h"

extern batch_info batch_info_table[];//批次表
/**************************************************************
*	Macro Define Section
**************************************************************/
//add起始的4个字节放置在32位数据中
#define ANALYZE_DATA_4B(add, data) 	\
	do {							\
		data = 0;					\
		data = *(add);				\
		data = data << 8;			\
		data |= *((add) + 1);		\
		data = data << 8;			\
		data |= *((add) + 2);		\
		data = data << 8;			\
		data |= *((add) + 3);		\
	}while(0)

//add起始的2个字节放置在16位数据中
#define ANALYZE_DATA_2B(add, data) 	\
	do {							\
		data = 0;					\
		data = *(add);				\
		data = data << 8;			\
		data |= *((add) + 1);		\
	}while(0)
	

/**************************************************************
*	Function Define Section
**************************************************************/
	
/**
 * @brief 	获取批次序号哈希值
 */
u8_t get_batch_hash(u16_t batch_number);

/* 当数据在不同区域时，需要对两个段进行解析 */
void Analyze_Data_With_Diff_Part(u8_t *src1, int len1, u8_t *src2, int len2, u32_t *data);
	
/**
 * @brief 	获取批次长度
 */
u16_t get_batch_length(u16_t batch_number);
/****************************************************************************************
*@Name............: Analyze_Batch_Info_Table
*@Description.....: 对批次数据报解包
*@Parameters......: batch_data		:存放数据的字节流
*					batch_number	:批次号
*@Return values...: void
*****************************************************************************************/
void Analyze_Batch_Info_Table(char *batch_data, u16_t batch_number);

void find_substr_head(char **data, char *substr, u16_t *len, u16_t sub_len);
#endif
