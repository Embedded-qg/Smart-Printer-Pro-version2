/***************************************************************************************
*	File Name              :	print_cells.h
* 	CopyRight              :	gaunthan
*	ModuleName             :
*
*	CPU                    :    
*	RTOS                   :    
*
*	Create Data            :
* 	Author/Corportation    :   
*
*	Abstract Description   :
*
*--------------------------------Revision History--------------------------------------
*       No      version     Date        Revised By      Item        Description
*       1       v1.0        2016/       gaunthan	                Create this file
*
***************************************************************************************/

/**************************************************************
*        Multi-Include-Prevent Section
**************************************************************/
#ifndef PRINT_CELLS_H
#define PRINT_CELLS_H

/**************************************************************
*        Debug switch Section
**************************************************************/


/**************************************************************
*        Include File Section
**************************************************************/
#include "cc.h"
#include "ucos_ii.h"
#include <stdlib.h>
#include "print_queue.h"
#include "status_mesg.h"
#include "print_error.h"

/**************************************************************
*        Macro Define Section
**************************************************************/
#define MAX_CELL_NUM	3/* 最大打印单元数量，应小于等于4 */

/**
 * 打印单元编号定义
 */
#define PRINT_CELL_NUM_ONE		1
#define PRINT_CELL_NUM_TWO		2
#define PRINT_CELL_NUM_THREE	3
#define PRINT_CELL_NUM_FOUR		4

/**
 * 驱动板缓冲区状态定义
 */
#define Xoff    0x13           //驱动板缓冲区满
#define Xon     0x11           //驱动板缓冲区空


/**************************************************************
*        Struct Define Section
**************************************************************/
typedef u8_t PrintCellNum;	//打印单元编号类型
typedef s8_t OrderEntry;	//订单信息结构入口

typedef enum {
	PRINT_CELL_STATUS_IDLE,		//空闲
	PRINT_CELL_STATUS_BUSY,		//忙碌
	PRINT_CELL_STATUS_ERR		//不可用
}PrintCellStatus;	/* 打印单元状态类型 */

typedef enum {
	ALL_ERROR,				//打印单元全部不可用
	EXIST_USEFUL_PRINTER		//有可用的打印单元
}AllPrinterStatus;	/* 打印单元状态类型 */

typedef struct {

	PrintCellNum no;					/* 单元编号 */
	OrderEntry entryIndex;				/* 回指订单 */
	PrintCellStatus status;				/* 指示打印单元的状态 */
	u8_t health_status;
	
	u32_t totalTime;					// 打印总时长，单位为0.1s	
	u32_t cutCnt;						// 总切刀次数
	u32_t totalLength;					// 总打印长度
	u8_t exceptCnt[EXCEPTION_NUM+1];	// 异常与故障发生次数
	
	u32_t workedTime;					// 连续打印时长，单位为0.1s
	u32_t beginTick;					// 一次打印开始时的滴答
	u32_t endTick;						// 一次打印结束时的滴答	
	
	OS_EVENT *printBeginSem;			// 标记该打印单元的打印线程是否需要开始工作
	OS_EVENT *printDoneSem;				// 标记该打印单元的打印线程是否完成了打印工作
	
	u16_t print_order_count;		//计算各自打印单元每个批次分配到的订单的数目
	s16_t sum_grade;							//该打印单元的积分
	double accuracy;						//打印单元的精确度
	u16_t dispend_order_number;	 	//打印单元分配到的订单
	
}PrintCellInfo;	/* 打印单元数据结构 */

typedef struct {
//	OS_EVENT *resrcSem1;					//打印单元1
//	OS_EVENT *resrcSem2;					//打印单元2
	OS_EVENT *resrcSem;
	PrintCellInfo cells[MAX_CELL_NUM];	//可使用的打印单元表
}PrintCellsMgrInfo;	/* 打印单元管理结构 */



/**************************************************************
*        Prototype Declare Section
**************************************************************/
/**
 *  @fn		PrioritySort
 *	@brief	将打印单元数组按精确度高低排序
 *	@param	None
 *	@ret	None
 */
void PrioritySort(void);

/**
 *  @fn		if_printer_all_error
 *	@brief	检测打印机是否全坏
 *	@param	None
 *	@ret	None
 */
AllPrinterStatus if_printer_all_error(void);

/**
 *  @fn		Count_Accuracy
 *	@brief	计算打印单元的精确度和各打印单元所分配到的订单数目
 *	@param	None
 *	@ret	None
 */
void Count_Accuracy(void);


/**
 *  @fn		ReadPrintCellsInfo
 *	@brief	从片内ROM读取并恢复所有打印单元的信息
 *	@param	None
 *	@ret	None
 */
void ReadPrintCellsInfo(void);


/**
 *  @fn		WritePrintCellInfo
 *	@brief	将打印单元的信息写入ROM
 *	@param	None
 *	@ret	None
 */
void WritePrintCellInfo(PrintCellNum no);


/**
 *  @fn		InitRestoreOrderQueue
 *	@brief	初始化恢复订单表
 *	@param	None
 *	@ret	None
 */
void InitRestoreOrderQueue(void);


/**
 *  @fn		RestoreOrder
 *	@brief	查询是否有待恢复打印订单，若有则将其编号放置于entryp所指内存
 *	@param	entryp 存放待恢复订单的编号
 *	@ret	1 有待恢复打印订单
			0 无待恢复打印订单
 */
int GetRestoredOrder(u8_t *entryp);


/**
 *  @fn		InitPrintCellsMgr
 *	@brief	初始化PrintCellsManager以及打印单元
 *	@param	None
 *	@ret	None
 */
void InitPrintCellsMgr(void);


/**
 *  @fn		DispensePrintJob
 *	@brief	将打印任务分配给打印单元，并执行打印
 *	@param	
			entryIndex 需打印订单的入口
 *	@ret	None
 */
void DispensePrintJob(u8_t entryIndex);


/**
 *  @fn		PutPrintCell
 *	@brief	将编号为no的打印单元设置为空闲，并将其归还管理器
 *	@param	no 打印单元编号
			status 打印单元的状态
 *	@ret	None
 */
void PutPrintCell(PrintCellNum no, PrintCellStatus status);

/**
 *  @fn		OutputErrorTag
 *	@brief	打印错误标记
 *	@param	no 打印单元编号
			status 打印单元的状态
 *	@ret	None
 */
void OutputErrorTag(PrintCellNum cellno);


/**
 * UART Receive Interrupt Hook Define 
 */
void USART1_Hook(void);
void USART3_Hook(void);
void UART4_Hook(void);
void UART6_Hook(void);
/**
 * DMA Transmission complete Hook Define 
 */
void USART1_DMA_TC_Hook(void);
void USART2_DMA_TC_Hook(void);


extern PrintCellsMgrInfo Prior;
extern PrintCellsMgrInfo PCMgr;

/**************************************************************
*        End-Multi-Include-Prevent Section
**************************************************************/
#endif