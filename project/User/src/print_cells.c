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
extern InternetTime current_internet_time[100];
extern u16_t batch_order_already_print;//һ�����ζ�����Ŀʣ��δ��ӡ�ķ���

u16_t Flag_receive_order;//�������ζ����ı�־
PrintCellsMgrInfo Prior;//�����Ĵ�ӡ��Ԫ����
u16_t Data_long;//���ζ�����Ŀ
u16_t exception_printer_set_normal_flag;//��ӡ�����쳣�ָ���־




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
	int i;
	double dispend_number;
	float grade = 0;	 //����ÿ�����δ�ӡ��Ԫ���ӵķ���
	PrintCellInfo *cellp;
	for(i=0;i<MAX_CELL_NUM;i++)//���������ӡʱ������1Сʱ,��ӡ��Ԫ���ּ�5��
	{
		cellp = &PCMgr.cells[i];
		if(cellp -> sum_grade >= 50)//��ӡ��Ԫ���ִ���50��
		{
			if(cellp -> workedTime >36000)//������ӡʱ������1Сʱ
			{
				cellp -> sum_grade = cellp -> sum_grade - 5;//���ּ�5��
				printf("more than 36000\r\n");
			}
		}
	}
	for(i=0;i<MAX_CELL_NUM;i++)//�����ӡ��Ԫ���������ܷ���
	{
		cellp = &PCMgr.cells[i];
		if(cellp -> sum_grade >= 10)
		{
			grade += cellp -> sum_grade;
			printf("cellp -> sum_grade = %d\r\n",cellp -> sum_grade);
		}
	}
	printf("grade = %f\r\n",grade);
	if(grade == 0)//˵����ӡ��Ԫ���ֶ���С��10�ֵģ���С��ʮ�����շ���ռ�ܷ����ı�����ȡ������Ŀ
	{
		for(i=0;i<MAX_CELL_NUM;i++)
		{
			cellp = &PCMgr.cells[i];
			grade += cellp -> sum_grade;
//			printf("cellp -> sum_grade = %d\r\n",cellp -> sum_grade);
		}//������ܻ���
		for(i=0;i<MAX_CELL_NUM;i++)//���ж�������
		{
			cellp = &PCMgr.cells[i];
			cellp -> accuracy = cellp -> sum_grade / grade;//�����ӡ��Ԫ������ռ�ܻ��ֵı���
			dispend_number =cellp -> accuracy * Data_long;
			cellp -> dispend_order_number = (int)((cellp -> accuracy) * Data_long);
			if(dispend_number > (cellp -> dispend_order_number+0.5))
			{
				cellp -> dispend_order_number += 1;
			}
		}//���ж�������
	}
	else{//˵��������һ̨���ִ���10��
		for(i=0;i<MAX_CELL_NUM;i++)
		{
			cellp = &PCMgr.cells[i];
			if(cellp -> sum_grade < 10)//����С��10�֣�ֻ�ܻ��һ�ݶ���
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
	for(i = 0; i < MAX_CELL_NUM; i++)//������ӡ��Ԫ��ӡ�Ķ�����Ŀ����Ϊ0
	{
		PCMgr.cells[i].print_order_count = 0;
	}
}

/**
 *  @fn		PrioritySort
 *	@brief	��ÿ����ӡ��Ԫ����ȷ�ȸߵͽ�������
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
 *	@brief	����ӡ����������ӡ��Ԫ����ִ�д�ӡ
 *	@param	
			entryIndex ���ӡ���������
 *	@ret	None
 */
void DispensePrintJob(u8_t entryIndex)
{
	PrintCellInfo *cellp;
	
	if((cellp = GetIdlePrintCell()) == NULL) {	//��ȡһ����ӡ��Ԫ
//		DEBUG_PRINT("BUG DETECT: DispensePrintJob: None Usable Print Cell Exits\n");
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
	int i;
	static int kill_PCMgr_resrcSem_flag;//��־�Ƿ��ӡ��ԪδPostPCMgr_resrcSem�ź���
	int Useful_printer;

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
	
//	DEBUG_PRINT("--------PutPrintCell: Print Cell %u: Set Idle----------\n", no);
	OS_ENTER_CRITICAL();
//	printf("PCMgr.cells[0].print_order_count = %ld  and  PCMgr.cells[1].print_order_count = %ld\r\n",PCMgr.cells[0].print_order_count,PCMgr.cells[1].print_order_count);
//	printf("   no = %d      !!!!!!!!\r\n",no);
//	printf("Prior[0].no = %d  and  Prior[1].no = %d      !!!!!!!!\r\n",Prior.cells[0].no,Prior.cells[1].no);
//	printf("Prior[0].accuracy = %f  and  Prior[1].accuracy = %f      !!!!!!!!\r\n",Prior.cells[0].accuracy,Prior.cells[1].accuracy);
//  printf("Prior[0].dispend_order_number = %ld and Prior[1].dispend_order_number = %ld\r\n",Prior.cells[0].dispend_order_number,Prior.cells[0].dispend_order_number);
	if(PCMgr.cells[0].status != PRINT_CELL_STATUS_ERR && PCMgr.cells[1].status != PRINT_CELL_STATUS_ERR)//�ж��Ƿ��д�ӡ��Ԫ������
	{
		Useful_printer = 1;//Ϊ1��ʾ��̨��ӡ��Ԫ������
	}
	else
	{
		Useful_printer = 0;//Ϊ0��ʾֻ��һ̨��ӡ��Ԫ���û��߶�������
	}
	if(Flag_receive_order == 1)
	{
		if(PCMgr.cells[0].print_order_count == 0 && PCMgr.cells[1].print_order_count == 0)//֤���տ�ʼҪ��ӡ����
		{
			if(Useful_printer == 1 && kill_PCMgr_resrcSem_flag == 1)//֤����̨��ӡ��Ԫ��������֮ǰ��̨��ӡ��ԪδPostPCMgr_resrcSem
			{
				kill_PCMgr_resrcSem_flag = 2;//post����PCMgr.resrcSem�ź���
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
			if(exception_printer_set_normal_flag > 2 && Useful_printer == 1)//�����ӡ��̨�����ڵ���2̨�Ļ�����ӡ����Ԫ���쳣�ָ��������ö�����������
			{
				exception_printer_set_normal_flag --;
			}
			else if(PCMgr.cells[i].no == no)//�ж�����̨��ӡ��Ԫ
			{
				PCMgr.cells[i].print_order_count++;//һ�����θô�ӡ��Ԫ�Ѿ���ӡ�ķ���
			}
			DEBUG_PRINT_STATEGY("\r\n after exception_printer_set_normal_flag = %d\r\n",exception_printer_set_normal_flag);
		}//�����ӡ��Ԫ���Դ�ӡ�Ķ�����Ŀ

		for(i = 0; i < MAX_CELL_NUM; i++)//��¼������ӡ��Ԫ��ӡ�Ķ�����Ŀ
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
	if(Prior.cells[1].print_order_count >= Prior.cells[1].dispend_order_number  && Prior.cells[1].no == no && (Prior.cells[1].dispend_order_number != (Data_long/2)) && Flag_receive_order == 1 && Useful_printer == 1)//��ӡ�ɹ��ʵ͵Ĵ�ӡ��Ԫ��ӡ���Լ��ķ����󲻷����ź�����ǰ���Ƕ�����Ŀ��Ϊ�ܶ�����һ����������ӡ��Ԫ������
	{
		kill_PCMgr_resrcSem_flag = 1;//����δpostPCMgr_resrcSem
		printf("here kill one PCMgr.resrcSem!!!!!\r\n");
	}
	else if(batch_order_already_print == 0 && Prior.cells[0].no == no && Flag_receive_order == 1 && kill_PCMgr_resrcSem_flag == 1 && Useful_printer == 1)//��ȷ�ȸߵĴ�ӡ��Ԫ���ӡ���ڵ���һ��Ķ���������ȷ�ȸߵĴ�ӡ��Ԫ��ӡ��һ�������е����һ�ݶ������ͷ������ź�����ǰ���Ƿ��䵽�Ķ�����Ŀ��Ϊ�ܶ�����Ŀ��һ����������ӡ��Ԫ������
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
					orderp->status = PRINT_STATUS_OK;				
					DEBUG_PRINT_TIMME("��ӡ�ɹ����������Ϊ%lu��",orderp->serial_number);
					ShowTime(orderp->sever_send_time,StartTime,OSTimeGet()*TIME_INTERVAL);								
					Delete_Order(cellp->entryIndex);
					
					PCMgr.cells[cellno-1].sum_grade++;//��ӡ��ȷ������1
					if(PCMgr.cells[cellno-1].sum_grade >= 100)//��������Ϊ100��
					{
						PCMgr.cells[cellno-1].sum_grade = 100;
					}
					batch_order_already_print--;
					DEBUG_PRINT("DealwithOrder: Order Print OK.\n");
				}else {							// ��ӡ��״̬�쳣��������ӡʧ��
					cellStatus = PRINT_CELL_STATUS_ERR;
					orderp->status = PRINT_STATUS_MACHINE_ERR;
					DEBUG_PRINT_TIMME("��ӡʧ�ܣ��������Ϊ%lu��",orderp->serial_number);
					ShowTime(orderp->sever_send_time,StartTime,OSTimeGet()*TIME_INTERVAL);				
					cellp->exceptCnt[status]++;															
					Printer_Status_Send(cellno, status);	// ��ӡ���쳣�����ʹ�ӡ��״̬
					WritePrintCellInfo(cellno);
					OutputErrorTag(cellno);
					
					PCMgr.cells[cellno-1].sum_grade = PCMgr.cells[cellno-1].sum_grade - 10 ;//��ӡ����һ�ݶ�����10��
					if(PCMgr.cells[cellno-1].sum_grade <= 0)//���ֲ�������0�֣����Ǵ�ӡ��Ԫ����
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
				OSSemPost(cellp->printDoneSem);	// ���Ͷ�����ӡ����ź�
			}
		}else {	// ��ӡ��Ԫ��æµ��״̬���ָ�����Խ�������߳�
			if(cellp->status == PRINT_CELL_STATUS_ERR) {
				if(status == NORMAL_STATE) {	// ��ӡ�����쳣�ָ�
//					DEBUG_PRINT("\nPrint Cell %u Restore from exception.\n", cellp->no);
					PutPrintCell(cellno, PRINT_CELL_STATUS_IDLE);
					exception_printer_set_normal_flag++;//
					printf("exception_printer_set_normal_flag = %d\r\n",exception_printer_set_normal_flag);
					printf("exception_PRINT_CELL_STATUS_IDLE\r\n");
				}
			}else if(cellp->status == PRINT_CELL_STATUS_IDLE) {	
				if(status != NORMAL_STATE){		// ��ӡ�����������쳣
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
 *	@brief ���ʹ�ӡ��״̬��ѯ4�ֽ�ָ����ڵ�4�ֽڻ�ȡ�����DealwithOrder
 *	@param	no ��ӡ��Ԫ���
 *	@ret	1����ȡǰ�����ֽڷ���
					0����ȡ����4���ֽ�
					-1�������������
 */
static s8_t CheckPrintStatue(PrintCellNum cellno)
{
	static u8_t tmp[4];
	static u8_t cnt = 0;
	
	tmp[cnt++] = PRINTER_GET(cellno);

	// �ڴ����ж��н�������ֻ�ܰ��ж�һ�ν���һByte�����У�����ÿ���յ�һ���ֽڣ��ͻᷢ����һ״ָ̬�
	// ��������һ�ν�����жϴ�����ʱ��״̬�������ݻᱻ���뻺������
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