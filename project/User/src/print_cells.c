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


extern INT32U StartTime[100];//��ʼʱ�ӽ���

PrintCellsMgrInfo Prior;//�����Ĵ�ӡ��Ԫ����
u16_t exception_printer_set_normal_flag;//��ӡ�����쳣�ָ���־
u16_t allOrderNum = 0;


/**************************************************************
*	Macro Define Section
**************************************************************/

#ifdef DEBUG_DETECT
/* debug���Ժ궨�壬���ݱ��ʽa�����ִ��has_bug��no_bug */
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
 *	�����ӡ��Ԫ
 */
PrintCellsMgrInfo PCMgr;
extern u16_t cell_cutCnt[MAX_CELL_NUM];

/**
 *	������ӡ����������õĶ��� 
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
 *	@brief	��Ƭ��ROM��ȡ���ָ����д�ӡ��Ԫ����Ϣ
 *	@param	None
 *	@ret	None
 */
void ReadPrintCellsInfo(void)
{
	int i;
	u8_t addr;
	PrintCellInfo *cellp;
	
	for(i = 0; i < MAX_CELL_NUM; i++) {
		//��ȡ�洢�׵�ַ
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
 *	@brief	����ӡ���Ƿ�ȫ��
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
 *	@brief	����ӡ��Ԫ����Ϣд��ROM
 *	@param	None
 *	@ret	None
 */
void WritePrintCellInfo(PrintCellNum no)
{
	u8_t addr;
	PrintCellInfo *cellp;

	//��ȡ�洢�׵�ַ
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
 *	@brief	��ʼ���ָ�������
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
 *	@brief	��ѯ�Ƿ��д��ָ���ӡ���������������ŷ�����entryp��ָ�ڴ�
 *	@param	entryp ��Ŵ��ָ������ı��
 *	@ret	1 �д��ָ���ӡ����
			0 �޴��ָ���ӡ����
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
 *	@brief	��ʼ��PrintCellsManager�Լ���ӡ��Ԫ
 *	@param	None
 *	@ret	None
 */
void InitPrintCellsMgr(void)
{
	PrintCellNum i;
	PrintCellInfo *cellp;

	//��ʼ����ӡ��Ԫ��Դ�ź���
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
 *	@brief	����ÿ����ӡ��Ԫ����ȷ��
 *	@param	None
 *	@ret	NULL 
			
 */
void Count_Accuracy(void)
{
	int i,all_dispend_number = 0,remain_order_num = 0;
	double dispend_number;
	float grade = 0;	 //����ÿ�����δ�ӡ��Ԫ���ӵķ���
	PrintCellInfo *cellp;

	for(i = 0;i < MAX_CELL_NUM;i++)//�����ӡ��Ԫ���������ܷ���
	{
		cellp = &PCMgr.cells[i];
		if(cellp->status == PRINT_CELL_STATUS_ERR) continue;
		grade += cellp -> sum_grade;
	}
	for(i=0;i<MAX_CELL_NUM;i++)
	{
			cellp = &PCMgr.cells[i];
			if(cellp->status == PRINT_CELL_STATUS_ERR) continue;
		  if(cellp -> sum_grade < 10 && grade >= MAX_CELL_NUM * 10) 
			{
				cellp -> dispend_order_number = 1;
				all_dispend_number++;
				allOrderNum--;
				continue;
			}
			cellp -> accuracy = cellp -> sum_grade / grade;
			dispend_number = cellp -> accuracy * allOrderNum;
			cellp -> dispend_order_number = (int)((cellp -> accuracy) * allOrderNum);
			all_dispend_number += cellp -> dispend_order_number;
	}
	if(all_dispend_number < allOrderNum) remain_order_num = allOrderNum - all_dispend_number;
	for(i = 0;i < MAX_CELL_NUM && remain_order_num > 0;i++)
	{
			cellp = &PCMgr.cells[i];	
			if(cellp->status == PRINT_CELL_STATUS_ERR) continue;
			cellp->dispend_order_number++;
			remain_order_num--;
	}
	for(i = 0;i < MAX_CELL_NUM && remain_order_num > 0;i++)
	{
			cellp = &PCMgr.cells[i];	
			printf("��ӡ��%d��Ҫ��ӡ�ķ���Ϊ%d\r\n",i + 1,cellp->dispend_order_number);
	}

}

/**
 *  @fn		orderOfCellsNull
 *	@brief	�ж��ǲ������д�ӡ���Ĵ���ӡ����������Ϊ0
 *	@param	None
 *	@ret	������д�ӡ���Ĵ���ӡ����������Ϊ0����ô����1�����򷵻�0
 */
int orderOfCellsNull(void)
{
	int i;
	PrintCellInfo *cellp;
	for(i = 0;i < MAX_CELL_NUM;i++)
	{
		cellp = &PCMgr.cells[i];
		if(cellp->dispend_order_number) return 0;
	}
	return 1;
}

/**
 *  @fn		GetIdlePrintCell
 *	@brief	����mgrp�����һ�����еĴ�ӡ��Ԫ
 *	@param	None
 *	@ret	NULL û�п��еĴ�ӡ��Ԫ
			���� ���д�ӡ��Ԫ
 */
static PrintCellInfo *GetIdlePrintCell(void)
{
#if OS_CRITICAL_METHOD == 3u 
    OS_CPU_SR  cpu_sr = 0u;
#endif
	
	PrintCellNum i;
	INT8U err;
	
//	printf("GetIdlePrintCell: Trying to get a usable print cell.\n");
	OSSemPend(PCMgr.resrcSem, 0, &err);
//	printf("GetIdlePrintCell: Got a usable print cell.\n");
	OS_ENTER_CRITICAL();

	for(i = 0; i < MAX_CELL_NUM; i++) {		
		if(PCMgr.cells[i].status == PRINT_CELL_STATUS_IDLE && PCMgr.cells[i].dispend_order_number) {
//		if(PCMgr.cells[i].status == PRINT_CELL_STATUS_IDLE) {
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
 *	@brief	����ӡ����������ӡ��Ԫ����ִ�д�ӡ
 *	@param	
			entryIndex ���ӡ���������
 *	@ret	None
 */
void DispensePrintJob(u8_t entryIndex)
{
	PrintCellInfo *cellp;
	if((cellp = GetIdlePrintCell()) == NULL) {	//��ȡһ����ӡ��Ԫ
		DEBUG_PRINT("BUG DETECT: DispensePrintJob: None Usable Print Cell Exits\n");
		return;
	}
	
	cellp->entryIndex = (OrderEntry)entryIndex;
	OSSemPost(cellp->printBeginSem);	// ��ʼ�������乤��
}

/**
 *  @fn		PutRestoredOrder
 *	@brief	���쳣���������쳣�ָ���
 *	@param	no ��ӡ��Ԫ���
			entryIndex �쳣��������
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
 *	@brief	�����Ϊno�Ĵ�ӡ��Ԫ����Ϊ���У�������黹������
 *	@param	no ��ӡ��Ԫ���
			status ��ӡ��Ԫ��״̬
 *	@ret	None
 */
void PutPrintCell(PrintCellNum no, PrintCellStatus status)
{
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR  cpu_sr = 0u;
#endif
	
	PrintCellInfo *cellp = &PCMgr.cells[no - 1];
	u8_t entryIndex;

	OS_ENTER_CRITICAL();
	cellp->status = status;
	entryIndex = cellp->entryIndex;
	cellp->entryIndex = -1;
	OS_EXIT_CRITICAL();
	
	if(status == PRINT_CELL_STATUS_ERR) { // ��ӡ��Ӳ�����ϣ�������Ҫ�ַ�������Ԫ���´�ӡ
		DEBUG_PRINT("--------PutPrintCell: Print Cell %u: Hardware Fault----------\n", no);
		PutRestoredOrder(no, entryIndex);
		return;
	}
	
	DEBUG_PRINT("--------PutPrintCell: Print Cell %u: Set Idle----------\n", no);
	OSSemPost(PCMgr.resrcSem);
}



/**
 *  @fn		OutputErrorTag
 *	@brief	��ӡ������
 *	@param	no ��ӡ��Ԫ���
			status ��ӡ��Ԫ��״̬
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
 *	@brief	�����ڽ����жϣ���ɶ����Ĵ�ӡ״̬�жϵȹ���
 *	@param	no ��ӡ��Ԫ���
 *	@ret	None
 */
/**
 *  @fn		DealwithOrder
 *	@brief	�����ڽ����жϣ���ɶ����Ĵ�ӡ״̬�жϵȹ���
 *	@param	no ��ӡ��Ԫ���
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
	
	//���չ�4B��״̬�������ݣ�����״̬�������� 
		status = ResolveStatus(tmp);
////		printf("PCMgr.cells = %d\r\n",cellno);
////		printf("second_PCMgr.cells[%d].status = %d\r\n",cellno,PCMgr.cells[cellno-1].status);
		if(cellp->status == PRINT_CELL_STATUS_BUSY) {	// ��ӡ��Ԫæµ������ݵ�ǰ��ӡ����״̬���������Ƿ��ӡ�ɹ�
			orderp = &order_print_table.order_node[cellp->entryIndex];
			if(orderp->status == ORDER_DATA_OK) {		
				if(status == NORMAL_STATE) {	// ��ӡ��״̬�������ɹ���ӡ
					cellStatus = PRINT_CELL_STATUS_IDLE;
	
					PCMgr.cells[cellno-1].dispend_order_number--;
					allOrderNum--;
					if(PCMgr.cells[cellno-1].sum_grade < 100)  PCMgr.cells[cellno-1].sum_grade++;
		
					orderp->status = PRINT_STATUS_OK;				
					orderp->finishTime = OSTimeGet();
					printf("%d,%d,%d,%d,%d,%d\r\n",orderp->serial_number,orderp->size,orderp->mcu_id,orderp->arrTime,orderp->finishTime,orderp->errorTime);
					Delete_Order(cellp->entryIndex);
					DEBUG_PRINT("DealwithOrder: Order Print OK.\n");
				
				}else {							// ��ӡ��״̬�쳣��������ӡʧ��
					cellStatus = PRINT_CELL_STATUS_ERR;
					orderp->status = PRINT_STATUS_MACHINE_ERR;
					if(orderp->errorTime == 0) orderp->errorTime = OSTimeGet();	
					cellp->exceptCnt[status]++;															
					Printer_Status_Send(cellno, status);	// ��ӡ���쳣�����ʹ�ӡ��״̬
					
					WritePrintCellInfo(cellno);
					OutputErrorTag(cellno);
					
					
					if(PCMgr.cells[cellno-1].sum_grade >= 10) PCMgr.cells[cellno-1].sum_grade = PCMgr.cells[cellno-1].sum_grade - 10 ;//��ӡ����һ�ݶ�����10��	
					
					DEBUG_PRINT("DealwithOrder: Order Print Failedly.\n");
				
					if(if_printer_all_error() == ALL_ERROR){
							DEBUG_PRINT(ERROR_PRINTER_ALL_ERROR);
//							Init_Queue();
							DEBUG_PRINT("\n--------xxxxxxxx-------\n batch_number:%u \n", orderp->batch_number);
							transf_task(order_netconn,order_status,TRANSFER_BATCH_STARTORDER,Get_TARGET_ID(),
							(u32_t)orderp->batch_number<<16|(u32_t)orderp->batch_within_number);
							return ;
				}
			}					
				cutPaper(cellno);
				PutPrintCell(cellno, cellStatus);					
				Order_Print_Status_Send(orderp,orderp->status);	
				OSSemPost(cellp->printDoneSem);	// ���Ͷ�����ӡ����ź�
			}
		}else {	// ��ӡ��Ԫ��æµ��״̬���ָ�����Խ�������߳�
			if(cellp->status == PRINT_CELL_STATUS_ERR) {
				if(status == NORMAL_STATE) {	// ��ӡ�����쳣�ָ�
//					DEBUG_PRINT("\nPrint Cell %u Restore from exception.\n", cellp->no);
					PutPrintCell(cellno, PRINT_CELL_STATUS_IDLE);
					exception_printer_set_normal_flag++;//
//					DEBUG_PRINT("exception_printer_set_normal_flag = %d\r\n",exception_printer_set_normal_flag);
//					DEBUG_PRINT("exception_PRINT_CELL_STATUS_IDLE\r\n");
				}
			}else if(cellp->status == PRINT_CELL_STATUS_IDLE) {	
				if(status != NORMAL_STATE){		// ��ӡ�����������쳣
//					DEBUG_PRINT("\nPrint Cell %u fell into exception.\n", cellp->no);
					cellp->status = PRINT_CELL_STATUS_ERR;
					OSSemAccept(PCMgr.resrcSem);
					DEBUG_PRINT("printer_failed_and kill one sem\r\n");
				}
			}	
			OSSemPost(Printer_Status_Rec_Sem);
		}	
}


/**
 *  @fn		CheckPrintStatue
 *	@brief ���ʹ�ӡ��״̬��ѯ4�ֽ�ָ����ڵ�4�ֽڻ�ȡ�����DealwithOrder
 *	@param	no ��ӡ��Ԫ���
 *	@ret	1����ȡǰ�����ֽڷ���
					0����ȡ����4���ֽ�
					-1�������������
 */
s8_t CheckPrintStatue(PrintCellNum cellno)
{
	static u8_t tmp[4];
	static u8_t cnt[5] = {0};
	
	tmp[cnt[cellno]] = PRINTER_GET(cellno);
	cnt[cellno]++;
	// �ڴ����ж��н�������ֻ�ܰ��ж�һ�ν���һByte�����У�����ÿ���յ�һ���ֽڣ��ͻᷢ����һ״ָ̬�
	// ��������һ�ν�����жϴ�����ʱ��״̬�������ݻᱻ���뻺������
	switch(cnt[cellno]) {
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
			cnt[cellno] = 0;
			return 0;
		default:
			cnt[cellno] = 0;
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

void UART6_Hook(void)
{
		CheckPrintStatue(PRINT_CELL_NUM_THREE);
}

void USART3_Hook(void)
{
		CheckPrintStatue(PRINT_CELL_NUM_FOUR);
}

static void DealwithDMA(PrintCellNum cellno)
{
	PrintCellInfo *cellp = &PCMgr.cells[cellno-1];
	
	cellp->endTick = sys_now();
	cellp->totalTime += (cellp->endTick - cellp->beginTick) * 20 / 100;		//��λ0.1s
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