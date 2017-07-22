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

/**************************************************************
*        Macro Define Section
**************************************************************/
#define MAX_CELL_NUM	2	/* 最大打印单元数量，应小于等于4 */

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
	
}PrintCellInfo;	/* 打印单元数据结构 */

typedef struct {
	OS_EVENT *resrcSem;					//标记可用的打印单元资源数	
	PrintCellInfo cells[MAX_CELL_NUM];	//可使用的打印单元表
}PrintCellsMgrInfo;	/* 打印单元管理结构 */

/**************************************************************
*        Prototype Declare Section
**************************************************************/
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
void UART4_Hook(void);

/**
 * DMA Transmission complete Hook Define 
 */
void USART1_DMA_TC_Hook(void);
void USART2_DMA_TC_Hook(void);


extern PrintCellsMgrInfo PCMgr;

/**************************************************************
*        End-Multi-Include-Prevent Section
**************************************************************/
#endif