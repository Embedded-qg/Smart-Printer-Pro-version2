#ifndef PRINT_QUEUE_H
#define PRINT_QUEUE_H

#include "cc.h"
#include "queue_buf.h"
#include "data_form.h"
#include "ucos_ii.h"
#include "os_cfg.h"
#include "main.h"
#include "app_cfg.h"
#include "netconf.h"
#include "printerException.h"
#include "print_cells.h"

#define ORDER_QUEUE_OK      	 0
#define ORDER_DATA_ERROR		-1
#define ORDER_PRINTER_ERROR		-2
#define ORDER_EMPTY_BLOCK		-3
#define ORDER_TOO_LARGER    	-100
#define ORDER_FIND_INDEX_ERR  -101


/*
	ReleaseBlock  函数返回值  释放内存块的情况
*/
#define RELEASE_ERR			0	//释放失败
#define RELEASE_OK   		1	//释放成功


/*
	Add_Order_To_Print_Queue 函数返回值
	判断加入到打印队列的状态
*/
#define ORDER_ENTRY_ERR     	-102 	//
#define ORDER_QUEUE_EMPTY   	-103 	//


/*
	各类型内存块数量
*/
#define BLOCK_1K_NUM     4
#define BLOCK_2K_NUM     3
#define BLOCK_4K_NUM     3
#define BLOCK_10K_NUM    1
#define ALL_BLOCK_NUM    (BLOCK_1K_NUM + BLOCK_2K_NUM + BLOCK_4K_NUM + BLOCK_10K_NUM)

/*
	内存块结束  索引
*/
#define BLOCK_1K_INDEX_END	(BLOCK_1K_NUM)
#define BLOCK_2K_INDEX_END	(BLOCK_1K_NUM + BLOCK_2K_NUM)
#define BLOCK_4K_INDEX_END	(BLOCK_1K_NUM + BLOCK_2K_NUM + BLOCK_4K_NUM)
#define BLOCK_10K_INDEX_END	(BLOCK_1K_NUM + BLOCK_2K_NUM + BLOCK_4K_NUM + BLOCK_10K_NUM)

/*
	内存块起始  索引
*/
#define BLOCK_1K_INDEX_START	(0)
#define BLOCK_2K_INDEX_START	(BLOCK_1K_INDEX_END)
#define BLOCK_4K_INDEX_START	(BLOCK_2K_INDEX_END)
#define BLOCK_10K_INDEX_START	(BLOCK_4K_INDEX_END)
/*
	各类型内存块大小
*/
#define BLOCK_1K_SIZE 1024*1
#define BLOCK_2K_SIZE 1024*2
#define BLOCK_4K_SIZE 1024*4
#define BLOCK_10K_SIZE 1024*10

#define BLOCK_MAX_SIZE BLOCK_10K_SIZE

/*
	订单优先级定义：二级优先级
*/
#define ORDER_PRIO_LOW        0
#define ORDER_PRIO_HIGH       1
#define ORDER_PRIO_INVALID    2

/*
	各类型时间长度
*/
#define SENT_TIME_NUM       6
#define REC_TIME_NUM        6
#define START_PRINT_TIME    6
#define END_PRINT_TIME      6
#define ORDER_MAX_TIME_NUM  6      //上述4种类型最大的长度


/*
	紧急订单 缺少内存块
*/
#define	Lack_Of_1K		-1
#define Lack_Of_2K		-2
#define Lack_Of_4K		-4
#define Lack_Of_10K		-5



/**
 * @brief	订单内容类型定义
 */
#define DATA_INVALID		0x00
#define DATA_IS_PLAINTEXT 	0x01
#define DATA_IS_IMAGE		0x02
#define DATA_IS_QRCODE		0x03

#define DATA_HEADER_SIZE 	4
#define DATA_FOOTER_SIZE	4

typedef struct buf_information
{
	u32_t common_buf_size;
	u32_t urgent_buf_size;
	u32_t common_buf_remain_capacity;
	u32_t urgent_buf_remain_capacity;
}buf_info;


typedef struct order_print_queue_information
{
	buf_info buf_node;
	order_info order_node[ALL_BLOCK_NUM];
	struct os_event  *mutex;
	u8_t head;   			//打印队列队首索引
	u8_t tail;   			//打印队列队尾索引
	u8_t empty;  			//打印队列是否为空：1代表空、0代表非空
}order_print_queue_info;



extern order_print_queue_info order_print_table;




void Init_Order_Table(void);

/**
 * @brief	删除一份订单，归还内存至系统
 * @param	订单索引
 * @return	执行结果
 */
s8_t Delete_Order(s8_t entry_index);



/**
 * @brief	检查订单数据域是否有错
 * @param	订单索引
 * @return	ORDER_DATA_ERR	数据域有误
			ORDER_DATA_OK	数据域正确
 */
s8_t CheckOrderData(u8_t entry_index);


/**
 * @brief	打印一份订单，并修改其状态
 * @param	打印单元索引
 * @return	执行结果
 */
s8_t Print_Order(u8_t cellno);
	

/**
 * @fn		GetOrderFromQueue
 * @brief	脱离打印队列头结点并将其索引放置在entryp所指空间
 * @param	entryp	存放获得的结点的索引
 * @return	ORDER_QUEUE_OK	获取成功
			ORDER_QUEUE_EMPTY 队列为空，获取失败
 */
s8_t GetOrderFromQueue(u8_t *entryp);


/**
 *  @fn		Order_QUEUE_Status_Send
 *	@brief 	发送订单进入打印队列状态报文
 *	@param	order 		订单结构体指针
						status		订单状态
 *	@ret	None
 */
void Order_QUEUE_Status_Send( order_info* order , u8_t status);


/**
 * @fn		Print_Queue_Fun
 * @brief	打印队列任务函数。
 * @param	
 * @return	
 */
void Print_Queue_Fun();


#endif
