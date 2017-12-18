/***************************************************************************************
 *	FileName					:	print_cell.c
 *	CopyRight					:
 *	ModuleName					:	
 *
 *	CPU							:
 *	RTOS						:
 *
 *	Create Data					:	2016/07/28
 *	Author/Corportation			:	gaunthan
 *
 *	Abstract Description		:	
 *
 *--------------------------------Revision History--------------------------------------
 *	No		version		Date		Revised By		Item		Description
 *	1		v1.0		2016/		gaunthan					Create this file
 *
 ***************************************************************************************/


/**************************************************************
*	Debug switch Section
**************************************************************/
//#define DEBUG_DETECT

/**************************************************************
*	Include File Section
**************************************************************/
#include "print_cells.h"


extern INT32U StartTime[100];//起始时钟节拍
extern InternetTime current_internet_time[100];
extern u16_t batch_order_already_print;//一个批次订单数目剩下未打印的份数

u16_t Flag_receive_order;//接收批次订单的标志
PrintCellsMgrInfo Prior;//排序后的打印单元数组
u16_t Data_long;//批次订单数目
u16_t exception_printer_set_normal_flag;//打印机从异常恢复标志




/**************************************************************
*	Macro Define Section
**************************************************************/

#ifdef DEBUG_DETECT
/* debug调试宏定义，根据表达式a的真假执行has_bug或no_bug */
#define BUG_DETECT_PRINT(a,has_bug,no_bug)   	\
	do {										\
		if(a)                                   \
			printf("%s",has_bug);               \
		else                                    \
			printf("%s",no_bug);				\
	}while(0)
#else
	
#define	BUG_DETECT_PRINT(a,has_bug,no_bug)		\
		do {									\
		}while(0)						
#endif
		
	
/**************************************************************
*	Struct Define Section
**************************************************************/


/**************************************************************
*	Prototype Declare Section
**************************************************************/
static PrintCellInfo *GetIdlePrintCell(void);
		
/**************************************************************
*	Global Variable Declare Section
**************************************************************/

/**
 *	管理打印单元
 */
PrintCellsMgrInfo PCMgr;


/**
 *	存放因打印机错误而搁置的订单 
 */
static s8_t RestoreOrderTable[MAX_CELL_NUM+1];

/**************************************************************
*	File Static Variable Define Section
**************************************************************/


/**************************************************************
*	Function Define Section
**************************************************************/

/**
 *  @fn		ReadPrintCellsInfo
 *	@brief	从片内ROM读取并恢复所有打印单元的信息
 *	@param	None
 *	@ret	None
 */
void ReadPrintCellsInfo(void)
{
	int i;
	u8_t addr;
	PrintCellInfo *cellp;
	
	for(i = 0; i < MAX_CELL_NUM; i++) {
		//获取存储首地址
		switch(i) {
			case 0:
				addr = DEVICE_ONE_START_ADDR; break;
			case 1:
				addr = DEVICE_TWO_START_ADDR; break;
			default:
				break;
		}
		
		cellp = &PCMgr.cells[i];
		
		ReadEEPROM((u8_t *)&cellp->totalTime, addr, sizeof(cellp->totalTime)); 
		addr += sizeof(cellp->totalTime);
		
		ReadEEPROM((u8_t *)&cellp->cutCnt, addr, sizeof(cellp->cutCnt));
		addr += sizeof(cellp->cutCnt);
		
		ReadEEPROM((u8_t *)&cellp->totalLength, addr, sizeof(cellp->totalLength));
		addr += sizeof(cellp->totalLength);
		
		ReadEEPROM((u8_t *)&cellp->exceptCnt[KNIFE_ERROR_STATE], addr, 
						sizeof(cellp->exceptCnt[KNIFE_ERROR_STATE]));
		addr += sizeof(cellp->exceptCnt[KNIFE_ERROR_STATE]);

		ReadEEPROM((u8_t *)&cellp->exceptCnt[UNEXPECTED_OPENED_STATE], addr, 
						sizeof(cellp->exceptCnt[UNEXPECTED_OPENED_STATE]));
		addr += sizeof(cellp->exceptCnt[UNEXPECTED_OPENED_STATE]);
		
		ReadEEPROM((u8_t *)&cellp->exceptCnt[PAPER_INSUFFICIENT_STATE], addr, 
						sizeof(cellp->exceptCnt[PAPER_INSUFFICIENT_STATE]));
		addr += sizeof(cellp->exceptCnt[PAPER_INSUFFICIENT_STATE]);
		
		ReadEEPROM((u8_t *)&cellp->exceptCnt[FILL_IN_PAPER_STATE], addr, 
						sizeof(cellp->exceptCnt[FILL_IN_PAPER_STATE]));		
	}
}


/**
 *  @fn		if_printer_all_error
 *	@brief	检测打印机是否全坏
 *	@param	None
 *	@ret	None
 */
AllPrinterStatus if_printer_all_error(void)
{
  int i;
	for(i = 0; i < MAX_CELL_NUM; i++) {		
		if(PCMgr.cells[i].status != PRINT_CELL_STATUS_ERR) 
		{
			return EXIST_USEFUL_PRINTER;
		}
	}
	return ALL_ERROR;
}


/**
 *  @fn		WritePrintCellInfo
 *	@brief	将打印单元的信息写入ROM
 *	@param	None
 *	@ret	None
 */
void WritePrintCellInfo(PrintCellNum no)
{
	u8_t addr;
	PrintCellInfo *cellp;

	//获取存储首地址
	switch(no) {
		case PRINT_CELL_NUM_ONE:
			addr = DEVICE_ONE_START_ADDR; break;
		case PRINT_CELL_NUM_TWO:
			addr = DEVICE_TWO_START_ADDR; break;
		default:
			addr = 0; break;
	}
		
	cellp = &PCMgr.cells[no-1];
	
	WriteEEPROM((u8_t *)&cellp->totalTime, addr, sizeof(cellp->totalTime)); 
	addr += sizeof(cellp->totalTime);
	
	WriteEEPROM((u8_t *)&cellp->cutCnt, addr, sizeof(cellp->cutCnt));
	addr += sizeof(cellp->cutCnt);
	
	WriteEEPROM((u8_t *)&cellp->totalLength, addr, sizeof(cellp->totalLength));
	addr += sizeof(cellp->totalLength);
	
	WriteEEPROM((u8_t *)&cellp->exceptCnt[KNIFE_ERROR_STATE], addr, 
					sizeof(cellp->exceptCnt[KNIFE_ERROR_STATE]));
	addr += sizeof(cellp->exceptCnt[KNIFE_ERROR_STATE]);

	WriteEEPROM((u8_t *)&cellp->exceptCnt[UNEXPECTED_OPENED_STATE], addr, 
					sizeof(cellp->exceptCnt[UNEXPECTED_OPENED_STATE]));
	addr += sizeof(cellp->exceptCnt[UNEXPECTED_OPENED_STATE]);
	
	WriteEEPROM((u8_t *)&cellp->exceptCnt[PAPER_INSUFFICIENT_STATE], addr, 
					sizeof(cellp->exceptCnt[PAPER_INSUFFICIENT_STATE]));
	addr += sizeof(cellp->exceptCnt[PAPER_INSUFFICIENT_STATE]);
	
	WriteEEPROM((u8_t *)&cellp->exceptCnt[FILL_IN_PAPER_STATE], addr, 
					sizeof(cellp->exceptCnt[FILL_IN_PAPER_STATE]));
					
	DEBUG_PRINT("--------------cell %u ----------\n", no);
	DEBUG_PRINT("------Total Time:    %u---------\n", cellp->totalTime);
	DEBUG_PRINT("------Worked Time:   %u---------\n", cellp->workedTime);
	DEBUG_PRINT("------Cut Count:     %u---------\n", cellp->cutCnt);
	DEBUG_PRINT("------Total length:  %u---------\n", cellp->totalLength);
	DEBUG_PRINT("------Except Knife:  %u---------\n", cellp->exceptCnt[KNIFE_ERROR_STATE]);
	DEBUG_PRINT("------Except Opend:  %u---------\n", cellp->exceptCnt[UNEXPECTED_OPENED_STATE]);
	DEBUG_PRINT("------Except Paper1: %u---------\n", cellp->exceptCnt[PAPER_INSUFFICIENT_STATE]);
	DEBUG_PRINT("------Except Paper2: %u---------\n", cellp->exceptCnt[FILL_IN_PAPER_STATE]);	
	DEBUG_PRINT("-------------------------------\n");
}



/**
 *  @fn		InitRestoreOrderQueue
 *	@brief	初始化恢复订单表
 *	@param	None
 *	@ret	None
 */
void InitRestoreOrderQueue(void)
{
	int i;

	for(i = 0; i < MAX_CELL_NUM; ++i)
		RestoreOrderTable[i] = -1;
}

/**
 *  @fn		GetRestoredOrder
 *	@brief	查询是否有待恢复打印订单，若有则将其编号放置于entryp所指内存
 *	@param	entryp 存放待恢复订单的编号
 *	@ret	1 有待恢复打印订单
			0 无待恢复打印订单
 */
int GetRestoredOrder(u8_t *entryp)
{
#if OS_CRITICAL_METHOD == 3u                               /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0u;
#endif
	int i;
	int ret = 0;
	
	OS_ENTER_CRITICAL();
	for(i = 0; i < MAX_CELL_NUM && RestoreOrderTable[i] < 0; ++i) {}
	if(i != MAX_CELL_NUM) {
		*entryp = RestoreOrderTable[i];
		RestoreOrderTable[i] = -1;
		ret = 1;		
	}
	OS_EXIT_CRITICAL();
	return ret;
}


/**
 *  @fn		InitPrintCellsMgr
 *	@brief	初始化PrintCellsManager以及打印单元
 *	@param	None
 *	@ret	None
 */
void InitPrintCellsMgr(void)
{
	PrintCellNum i;
	PrintCellInfo *cellp;

	//初始化打印单元资源信号量
	PCMgr.resrcSem = OSSemCreate(0); 
	BUG_DETECT_PRINT(PCMgr.resrcSem == NULL, "Create cell resource sem failed.\n", "Create cell resource sem success.\n");
	
	for(i = 0; i < MAX_CELL_NUM; i++) {
		cellp = &PCMgr.cells[i];
	
		cellp->no = PRINT_CELL_NUM_ONE + i;
		cellp->status = PRINT_CELL_STATUS_ERR;	
		cellp->entryIndex = -1;
		cellp->health_status = PRINTER_HEALTH_UNHEALTHY ;
		cellp->sum_grade = 50;
		
		cellp->printBeginSem = OSSemCreate(0);
		cellp->printDoneSem = OSSemCreate(0);
		
	}
}

/**
 *  @fn		Count_Accuracy
 *	@brief	计算每个打印单元的正确率
 *	@param	None
 *	@ret	NULL 
			
 */
void Count_Accuracy(void)
{
	int i;
	double dispend_number;
	float grade = 0;	 //计算每个批次打印单元所加的分数
	PrintCellInfo *cellp;
	for(i=0;i<MAX_CELL_NUM;i++)//如果连续打印时长超过1小时,打印单元积分减5分
	{
		cellp = &PCMgr.cells[i];
		if(cellp -> sum_grade >= 50)//打印单元积分大于50分
		{
			if(cellp -> workedTime >36000)//连续打印时长超过1小时
			{
				cellp -> sum_grade = cellp -> sum_grade - 5;//积分减5分
				printf("more than 36000\r\n");
			}
		}
	}
	for(i=0;i<MAX_CELL_NUM;i++)//计算打印单元加起来的总分数
	{
		cellp = &PCMgr.cells[i];
		if(cellp -> sum_grade >= 10)
		{
			grade += cellp -> sum_grade;
			printf("cellp -> sum_grade = %d\r\n",cellp -> sum_grade);
		}
	}
	printf("grade = %f\r\n",grade);
	if(grade == 0)//说明打印单元积分都是小于10分的，都小于十分则按照分数占总分数的比例获取订单数目
	{
		for(i=0;i<MAX_CELL_NUM;i++)
		{
			cellp = &PCMgr.cells[i];
			grade += cellp -> sum_grade;
//			printf("cellp -> sum_grade = %d\r\n",cellp -> sum_grade);
		}//计算出总积分
		for(i=0;i<MAX_CELL_NUM;i++)//进行订单分配
		{
			cellp = &PCMgr.cells[i];
			cellp -> accuracy = cellp -> sum_grade / grade;//计算打印单元积分所占总积分的比例
			dispend_number =cellp -> accuracy * Data_long;
			cellp -> dispend_order_number = (int)((cellp -> accuracy) * Data_long);
			if(dispend_number > (cellp -> dispend_order_number+0.5))
			{
				cellp -> dispend_order_number += 1;
			}
		}//进行订单分配
	}
	else{//说明至少有一台积分大于10分
		for(i=0;i<MAX_CELL_NUM;i++)
		{
			cellp = &PCMgr.cells[i];
			if(cellp -> sum_grade < 10)//积分小于10分，只能获得一份订单
			{
				cellp -> accuracy = 0;
				cellp -> dispend_order_number = 1;
			}
			else if(cellp -> sum_grade >= 10)
			{
				cellp -> accuracy = cellp -> sum_grade / grade;
				dispend_number =cellp -> accuracy * Data_long;
				cellp -> dispend_order_number = (int)((cellp -> accuracy) * Data_long);
				if(dispend_number > (cellp -> dispend_order_number+0.5))
				{
					cellp -> dispend_order_number += 1;
				}
			}
			DEBUG_PRINT_STATEGY("PCMgr.cells[%d].sum_grade = %d\r\n",i,PCMgr.cells[i].sum_grade);
			DEBUG_PRINT_STATEGY("PCMgr.cells[%d].accuracy = %f\r\n",i,PCMgr.cells[i].accuracy);
		}
	}
	for(i = 0; i < MAX_CELL_NUM; i++)//将各打印单元打印的订单数目重置为0
	{
		PCMgr.cells[i].print_order_count = 0;
	}
}

/**
 *  @fn		PrioritySort
 *	@brief	将每个打印单元按精确度高低进行排序
 *	@param	None
 *	@ret	NULL 
			
 */
void PrioritySort(void)
{
	int i,k,j;
	PrintCellInfo t;
	for(i=0;i<MAX_CELL_NUM;i++)
	{
		Prior.cells[i]=PCMgr.cells[i];
	}
	for(i=0;i<MAX_CELL_NUM;i++) 
	{
		k=i;
		for(j=i+1;j<MAX_CELL_NUM;j++)
//			if(Prior.cells[k].accuracy<Prior.cells[j].accuracy)
				k=j;
		if(k!=j)
		{
			t=Prior.cells[k];
			Prior.cells[k]=Prior.cells[i];
			Prior.cells[i]=t;
		}		
	}
}
/**
 *  @fn		GetIdlePrintCell
 *	@brief	查找mgrp，获得一个空闲的打印单元
 *	@param	None
 *	@ret	NULL 没有空闲的打印单元
			其他 空闲打印单元
 */
static PrintCellInfo *GetIdlePrintCell(void)
{
#if OS_CRITICAL_METHOD == 3u 
    OS_CPU_SR  cpu_sr = 0u;
#endif
	
	PrintCellNum i;
	INT8U err;
	
	DEBUG_PRINT("GetIdlePrintCell: Trying to get a usable print cell.\n");
	OSSemPend(PCMgr.resrcSem, 0, &err);
	DEBUG_PRINT("GetIdlePrintCell: Got a usable print cell.\n");
	OS_ENTER_CRITICAL();
	for(i = 0; i < MAX_CELL_NUM; i++) {		
		if(PCMgr.cells[i].status == PRINT_CELL_STATUS_IDLE) {
			PCMgr.cells[i].status = PRINT_CELL_STATUS_BUSY;
			OS_EXIT_CRITICAL();
			DEBUG_PRINT("GetIdlePrintCell: Print Cell %u: Set Busy\n", i+1);
			return &PCMgr.cells[i];
		}	
	}
	OS_EXIT_CRITICAL();
	return NULL;
}
 

/**
 *  @fn		DispensePrintJob
 *	@brief	将打印任务分配给打印单元，并执行打印
 *	@param	
			entryIndex 需打印订单的入口
 *	@ret	None
 */
void DispensePrintJob(u8_t entryIndex)
{
	PrintCellInfo *cellp;
	
	if((cellp = GetIdlePrintCell()) == NULL) {	//获取一个打印单元
//		DEBUG_PRINT("BUG DETECT: DispensePrintJob: None Usable Print Cell Exits\n");
		return;
	}
	cellp->entryIndex = (OrderEntry)entryIndex;
	OSSemPost(cellp->printBeginSem);	// 开始订单传输工作
}


/**
 *  @fn		PutRestoredOrder
 *	@brief	将异常订单挂起到异常恢复表
 *	@param	no 打印单元编号
			entryIndex 异常订单索引
 *	@ret	None
 */
void PutRestoredOrder(PrintCellNum no, OrderEntry entryIndex)
{
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR  cpu_sr = 0u;
#endif
	PrintCellInfo *cellp = &PCMgr.cells[no - 1];
	extern OS_EVENT *Print_Sem;	
	
	OS_ENTER_CRITICAL();
	RestoreOrderTable[no-1] =  entryIndex;
	OS_EXIT_CRITICAL();
	OSSemPost(Print_Sem);	
}

/**
 *  @fn		PutPrintCell
 *	@brief	将编号为no的打印单元设置为空闲，并将其归还管理器
 *	@param	no 打印单元编号
			status 打印单元的状态
 *	@ret	None
 */

void PutPrintCell(PrintCellNum no, PrintCellStatus status)
{
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR  cpu_sr = 0u;
#endif
	
	PrintCellInfo *cellp = &PCMgr.cells[no - 1];
	u8_t entryIndex;
	int i;
	static int kill_PCMgr_resrcSem_flag;//标志是否打印单元未PostPCMgr_resrcSem信号量
	int Useful_printer;

	OS_ENTER_CRITICAL();
	cellp->status = status;
	entryIndex = cellp->entryIndex;
	cellp->entryIndex = -1;
	OS_EXIT_CRITICAL();
	
	
	
	if(status == PRINT_CELL_STATUS_ERR) { // 打印机硬件故障，订单需要分发其他单元重新打印
		DEBUG_PRINT("--------PutPrintCell: Print Cell %u: Hardware Fault----------\n", no);
		PutRestoredOrder(no, entryIndex);
		return;
	}
	
//	DEBUG_PRINT("--------PutPrintCell: Print Cell %u: Set Idle----------\n", no);
	OS_ENTER_CRITICAL();
//	printf("PCMgr.cells[0].print_order_count = %ld  and  PCMgr.cells[1].print_order_count = %ld\r\n",PCMgr.cells[0].print_order_count,PCMgr.cells[1].print_order_count);
//	printf("   no = %d      !!!!!!!!\r\n",no);
//	printf("Prior[0].no = %d  and  Prior[1].no = %d      !!!!!!!!\r\n",Prior.cells[0].no,Prior.cells[1].no);
//	printf("Prior[0].accuracy = %f  and  Prior[1].accuracy = %f      !!!!!!!!\r\n",Prior.cells[0].accuracy,Prior.cells[1].accuracy);
//  printf("Prior[0].dispend_order_number = %ld and Prior[1].dispend_order_number = %ld\r\n",Prior.cells[0].dispend_order_number,Prior.cells[0].dispend_order_number);
	if(PCMgr.cells[0].status != PRINT_CELL_STATUS_ERR && PCMgr.cells[1].status != PRINT_CELL_STATUS_ERR)//判断是否有打印单元不可用
	{
		Useful_printer = 1;//为1表示两台打印单元都可用
	}
	else
	{
		Useful_printer = 0;//为0表示只有一台打印单元可用或者都不可用
	}
	if(Flag_receive_order == 1)
	{
		if(PCMgr.cells[0].print_order_count == 0 && PCMgr.cells[1].print_order_count == 0)//证明刚开始要打印订单
		{
			if(Useful_printer == 1 && kill_PCMgr_resrcSem_flag == 1)//证明两台打印单元均可用且之前有台打印单元未PostPCMgr_resrcSem
			{
				kill_PCMgr_resrcSem_flag = 2;//post两个PCMgr.resrcSem信号量
				OSSemPost(PCMgr.resrcSem);
				OSSemPost(PCMgr.resrcSem);
				if(PCMgr.resrcSem->OSEventCnt >= 3)
				{
					OSSemAccept(PCMgr.resrcSem);
				}
			}
		}
		for(i = 0; i < MAX_CELL_NUM; i++)
		{	
			DEBUG_PRINT_STATEGY("\r\n before exception_printer_set_normal_flag = %d\r\n",exception_printer_set_normal_flag);
			if(exception_printer_set_normal_flag > 2 && Useful_printer == 1)//如果打印机台数大于等于2台的话，打印机单元从异常恢复正常不用订单计数增加
			{
				exception_printer_set_normal_flag --;
			}
			else if(PCMgr.cells[i].no == no)//判断是哪台打印单元
			{
				PCMgr.cells[i].print_order_count++;//一个批次该打印单元已经打印的份数
			}
			DEBUG_PRINT_STATEGY("\r\n after exception_printer_set_normal_flag = %d\r\n",exception_printer_set_normal_flag);
		}//计算打印单元各自打印的订单数目

		for(i = 0; i < MAX_CELL_NUM; i++)//记录各个打印单元打印的订单数目
		{
			if(Prior.cells[i].no == 1)
			{
				Prior.cells[i].print_order_count = PCMgr.cells[0].print_order_count;
			}
			else
			{
				Prior.cells[i].print_order_count = PCMgr.cells[1].print_order_count;
			}
		}
	}
	DEBUG_PRINT_STATEGY("Data_long = %d\r\n",Data_long);
	DEBUG_PRINT_STATEGY("Prior.cells[0].print_order_count = %d  and  Prior.cells[0].dispend_order_number = %d\r\n",Prior.cells[0].print_order_count,Prior.cells[0].dispend_order_number);
	DEBUG_PRINT_STATEGY("Prior.cells[1].print_order_count = %d  and  Prior.cells[1].dispend_order_number = %d\r\n",Prior.cells[1].print_order_count,Prior.cells[1].dispend_order_number);
	if(Prior.cells[1].print_order_count >= Prior.cells[1].dispend_order_number  && Prior.cells[1].no == no && (Prior.cells[1].dispend_order_number != (Data_long/2)) && Flag_receive_order == 1 && Useful_printer == 1)//打印成功率低的打印单元打印完自己的份数后不发送信号量，前提是订单数目不为总订单的一半且两个打印单元都可用
	{
		kill_PCMgr_resrcSem_flag = 1;//这里未postPCMgr_resrcSem
		printf("here kill one PCMgr.resrcSem!!!!!\r\n");
	}
	else if(batch_order_already_print == 0 && Prior.cells[0].no == no && Flag_receive_order == 1 && kill_PCMgr_resrcSem_flag == 1 && Useful_printer == 1)//精确度高的打印单元会打印大于等于一半的订单，即精确度高的打印单元打印完一个批次中的最后一份订单，释放两个信号量，前提是分配到的订单数目不为总订单数目的一半且两个打印单元均可用
	{
		kill_PCMgr_resrcSem_flag = 2;
		OSSemPost(PCMgr.resrcSem);
		OSSemPost(PCMgr.resrcSem);
		if(PCMgr.resrcSem->OSEventCnt >= 3)
		{
			OSSemAccept(PCMgr.resrcSem);
		}
		DEBUG_PRINT_STATEGY("here post 2 signal post_resrcSem_OSEventCnt\r\n");
	}
	else
	{
		OSSemPost(PCMgr.resrcSem);
		if(PCMgr.resrcSem->OSEventCnt >= 3)
		{
			OSSemAccept(PCMgr.resrcSem);
		}
		DEBUG_PRINT_STATEGY("here post 1 signal post_resrcSem_OSEventCnt!!!!!!!!! = %d\r\n",PCMgr.resrcSem->OSEventCnt);
	}
	DEBUG_PRINT_STATEGY("post_resrcSem_OSEventCnt = %d\r\n",PCMgr.resrcSem->OSEventCnt);
	OS_EXIT_CRITICAL();
}


/**
 *  @fn		OutputErrorTag
 *	@brief	打印错误标记
 *	@param	no 打印单元编号
			status 打印单元的状态
 *	@ret	None
 */
void OutputErrorTag(PrintCellNum cellno)
{
	u8_t errMsg[] = "\n\n"
					"          This was a invalid order :(          \n"
					" ;) But don't worry.I may try it again lagter  \n\n";
	
	outputData(errMsg, sizeof(errMsg), cellno); 	
}

/**
 *  @fn		DealwithOrder
 *	@brief	处理串口接收中断，完成订单的打印状态判断等功能
 *	@param	no 打印单元编号
 *	@ret	None
 */
/**
 *  @fn		DealwithOrder
 *	@brief	处理串口接收中断，完成订单的打印状态判断等功能
 *	@param	no 打印单元编号
 *	@ret	None
 */
static void DealwithOrder(PrintCellNum cellno,u8_t *tmp)
{
	extern OS_EVENT *Printer_Status_Rec_Sem;

	u8_t status;
	PrintCellStatus cellStatus;
	PrintCellInfo *cellp = &PCMgr.cells[cellno-1];
	order_info *orderp;
	extern OS_EVENT *Printer_Status_Rec_Sem;
	
	//接收够4B的状态反馈数据，进行状态解析工作 
		status = ResolveStatus(tmp);
////		printf("PCMgr.cells = %d\r\n",cellno);
////		printf("second_PCMgr.cells[%d].status = %d\r\n",cellno,PCMgr.cells[cellno-1].status);
		if(cellp->status == PRINT_CELL_STATUS_BUSY) {	// 打印单元忙碌，则根据当前打印机的状态决定订单是否打印成功
			orderp = &order_print_table.order_node[cellp->entryIndex];
			if(orderp->status == ORDER_DATA_OK) {		
				if(status == NORMAL_STATE) {	// 打印机状态正常，成功打印
					cellStatus = PRINT_CELL_STATUS_IDLE;
					orderp->status = PRINT_STATUS_OK;				
					DEBUG_PRINT_TIMME("打印成功，订单编号为%lu，",orderp->serial_number);
					ShowTime(orderp->sever_send_time,StartTime,OSTimeGet()*TIME_INTERVAL);								
					Delete_Order(cellp->entryIndex);
					
					PCMgr.cells[cellno-1].sum_grade++;//打印正确分数加1
					if(PCMgr.cells[cellno-1].sum_grade >= 100)//积分上限为100分
					{
						PCMgr.cells[cellno-1].sum_grade = 100;
					}
					batch_order_already_print--;
					DEBUG_PRINT("DealwithOrder: Order Print OK.\n");
				}else {							// 打印机状态异常，订单打印失败
					cellStatus = PRINT_CELL_STATUS_ERR;
					orderp->status = PRINT_STATUS_MACHINE_ERR;
					DEBUG_PRINT_TIMME("打印失败，订单编号为%lu，",orderp->serial_number);
					ShowTime(orderp->sever_send_time,StartTime,OSTimeGet()*TIME_INTERVAL);				
					cellp->exceptCnt[status]++;															
					Printer_Status_Send(cellno, status);	// 打印机异常，发送打印机状态
					WritePrintCellInfo(cellno);
					OutputErrorTag(cellno);
					
					PCMgr.cells[cellno-1].sum_grade = PCMgr.cells[cellno-1].sum_grade - 10 ;//打印出错一份订单减10分
					if(PCMgr.cells[cellno-1].sum_grade <= 0)//积分不能少于0分，除非打印单元坏了
					{
						PCMgr.cells[cellno-1].sum_grade = 1;
					}
					
					DEBUG_PRINT("DealwithOrder: Order Print Failedly.\n");
					printf("\nDealwithOrder: Order Print Failedly.");
				
					if(if_printer_all_error() == ALL_ERROR){
							printf("sssss");
							printf(ERROR_PRINTER_ALL_ERROR);
//							Init_Queue();
						printf("\n--------xxxxxxxx-------\n batch_number:%u \n", orderp->batch_number);
							transf_task(order_netconn,order_status,TRANSFER_BATCH_STARTORDER,Get_TARGET_ID(),
							(u32_t)orderp->batch_number<<16|(u32_t)orderp->batch_within_number);
							return ;
				}
			}					
				cutPaper(cellno);
				PutPrintCell(cellno, cellStatus);					

				Order_Print_Status_Send(orderp,orderp->status);	
				OSSemPost(cellp->printDoneSem);	// 发送订单打印完成信号
			}
		}else {	// 打印单元非忙碌，状态检测指令来自健康检测线程
			if(cellp->status == PRINT_CELL_STATUS_ERR) {
				if(status == NORMAL_STATE) {	// 打印机从异常恢复
//					DEBUG_PRINT("\nPrint Cell %u Restore from exception.\n", cellp->no);
					PutPrintCell(cellno, PRINT_CELL_STATUS_IDLE);
					exception_printer_set_normal_flag++;//
					printf("exception_printer_set_normal_flag = %d\r\n",exception_printer_set_normal_flag);
					printf("exception_PRINT_CELL_STATUS_IDLE\r\n");
				}
			}else if(cellp->status == PRINT_CELL_STATUS_IDLE) {	
				if(status != NORMAL_STATE){		// 打印机从正常到异常
//					DEBUG_PRINT("\nPrint Cell %u fell into exception.\n", cellp->no);
					cellp->status = PRINT_CELL_STATUS_ERR;
					OSSemAccept(PCMgr.resrcSem);
					printf("printer_failed_and kill one sem\r\n");
				}
			}	
			OSSemPost(Printer_Status_Rec_Sem);
		}	
}


/**
 *  @fn		CheckPrintStatue
 *	@brief 发送打印机状态查询4字节指令，并在第4字节获取后调用DealwithOrder
 *	@param	no 打印单元编号
 *	@ret	1：获取前三个字节返回
					0：获取到第4个字节
					-1：其他错误情况
 */
static s8_t CheckPrintStatue(PrintCellNum cellno)
{
	static u8_t tmp[4];
	static u8_t cnt = 0;
	
	tmp[cnt++] = PRINTER_GET(cellno);

	// 在串口中断中接收数据只能按中断一次接收一Byte来进行，下面每接收到一个字节，就会发送下一状态指令，
	// 这样，下一次进入该中断处理函数时，状态反馈数据会被读入缓冲区。
	switch(cnt) {
		case 1:
			SEND_STATUS_CMD_TWO(cellno);
			return 1;
		case 2:
			SEND_STATUS_CMD_THREE(cellno);
			return 1;		
		case 3:
			SEND_STATUS_CMD_FOUR(cellno);
			return 1;
		case 4:
			DealwithOrder(cellno,tmp);
			cnt = 0;
			return 0;
		default:
			cnt = 0;
			return -1;
	}	
}


/**
 * UART Receive Interrupt Hook Define 
 */
void USART1_Hook(void)
{
	CheckPrintStatue(PRINT_CELL_NUM_ONE);
}


void UART4_Hook(void)
{
	CheckPrintStatue(PRINT_CELL_NUM_TWO);
}



static void DealwithDMA(PrintCellNum cellno)
{
	PrintCellInfo *cellp = &PCMgr.cells[cellno-1];
	
	cellp->endTick = sys_now();
	cellp->totalTime += (cellp->endTick - cellp->beginTick) * 20 / 100;		//单位0.1s
	cellp->workedTime += (cellp->endTick - cellp->beginTick) * 20 / 100;
}

/**
 * DMA Transmission complete Hook Define 
 */
void USART1_DMA_TC_Hook(void)
{
	DealwithDMA(PRINT_CELL_NUM_ONE);
}


void USART2_DMA_TC_Hook(void)
{
	DealwithDMA(PRINT_CELL_NUM_TWO);
}


 #ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8 *file, u32 line)
{
          /* User can add his own implementation to report the file name and line number,
             ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
			
          while (1)
          {}
}
#endif