#include "local_conf.h"

#define ORDER_LENGTH_LAST_OFFSET	4//订单头的订单长度的最后字节位置
#define ORDER_HEAD_LAST_OFFSET		2//订单头的标志的最后字节位置
#define ORDER_NUM_HEAD_OFFSET		12//订单序号头偏移
#define URGENT						1//加急为1
#define READ_MAX_SIZE				(1024 * 10)
#define BUF_IS_NOT_EMPTY			0//缓冲区不为空
#define BUF_IS_EMPTY				1//缓冲区为空
#define ORDER_NUM_LENGTH			4//订单序号长度
#define ORDER_HEAD_LENGTH			28//订单中除数据外的字节长度

/**************************************************************
*	Global Variable Section
**************************************************************/
extern SqQueue usart_buf;				//串口缓冲区
//static char readFromBuf[READ_MAX_SIZE];	//从缓冲区读取的数据
extern OS_EVENT *Ack_Rec_Order_Sem;		//本地接收对订单状态应答的信号量
extern OS_EVENT *Ack_Rec_Printer_Sem;	//本地接收对打印机状态应答的信号量
static u32_t last_order_number = 0;		//上一次接收的订单号
static u32_t current_order_number = 0;	//本次接收的订单号
extern OS_EVENT *Print_Queue_Sem;
/**************************************************************
*	Function Define Section
**************************************************************/

void USART3_Hook(u8_t ch)//将从串口3读取到的数据放入缓冲区
{
	extern OS_EVENT *Local_Rec_Data_Sem;
	if((usart_buf.write + 1) % usart_buf.MAX == usart_buf.read)
		return;
	
	usart_buf.base[usart_buf.write++] = ch;
	usart_buf.write = usart_buf.write % usart_buf.MAX;
	if(usart_buf.buf_empty)
		OSSemPost(Local_Rec_Data_Sem);
	usart_buf.buf_empty = BUF_IS_NOT_EMPTY;    //缓冲不为空
}

//获取缓冲区长度
#define GetBufLen(buf) \
	((buf.write) - (buf.read))
	
void ReadBytes(char *data, u16_t *len)//读取串口缓冲区，有多少读多少，但不移动缓冲区指针
{
	u16_t i;
	u16_t buf_len = 0;

	if(!usart_buf.buf_empty){//缓冲区不为空
	
		buf_len = GetBufLen(usart_buf);//缓冲区长度
		
		if(buf_len > READ_MAX_SIZE)
			buf_len = READ_MAX_SIZE;//最多只读缓存量

		for(i = 0; i < buf_len; ++i)
			*data++ = usart_buf.base[usart_buf.read + i];
	}
	
	*len = buf_len;
}

s8_t ReadAssignBytes(char *data, u16_t len)//读取串口缓冲区，指定长度为len
{
	u16_t i;
	u16_t buf_len;
	s8_t buf_err = BUF_EMPTY;
	
	do{
		if(!usart_buf.buf_empty){//缓冲区不为空
		
			buf_len = GetBufLen(usart_buf);//缓冲区长度
			
			if(buf_len < len){
				buf_err = BUF_ARG_ERR;
				OSTimeDlyHMSM(0, 0, 0, 20);//等待有数据
			}else{
				for(i = 0; i < len; ++i)
					*data++ = usart_buf.base[usart_buf.read + i];
				
				buf_err = BUF_OK;
			}
		}else{
			OSTimeDlyHMSM(0, 0, 0, 20);//等待有数据
		}
		
	}while(buf_err != BUF_OK);
	
	return BUF_OK;
}

s8_t DeleteBuf(u16_t len)//删除长度为len的缓冲区
{
	s8_t buf_err;
	u16_t buf_len;
	
	if(usart_buf.buf_empty)
		buf_err = BUF_ARG_ERR;
	
	buf_len = GetBufLen(usart_buf);//缓冲区长度	
	if(buf_len < len)
		buf_err = BUF_ARG_ERR;
	
	if(buf_err == BUF_ARG_ERR){
		return buf_err;
	}
	
	usart_buf.read = usart_buf.read + len;//更新读指针
	if(usart_buf.read == usart_buf.write){
		usart_buf.read = usart_buf.write = 0;
		usart_buf.buf_empty = BUF_IS_EMPTY;
	}
	
	DEBUG_PRINT("usart_buf.write is %lu, usart_buf.read is %lu\n", usart_buf.write, usart_buf.read);
	
	return BUF_OK;
}

static void ClearBuf(void) //清空缓冲区
{
	usart_buf.read = usart_buf.write = 0;
	usart_buf.buf_empty = BUF_IS_EMPTY;
}

Status CheckRec(u8_t *data, u16_t len)//检测接收报文是否正确
{
	if(0 == Check_Sum((u16_t*)data, len))
		return True;
	else
		return Fail;
}

void LocalSendData(char *data, u16_t len)//从串口发送数据
{
	int i;
	INT8U err;
	
	extern OS_EVENT *Local_Send_Data_Mutex;		//本地发送状态锁
	
	OSMutexPend(Local_Send_Data_Mutex, 0, &err);	//申请发送状态锁
	for(i = 0; i < len; ++i)
		USART_SendData(USART3, *(data + i));
	
	OSMutexPost(Local_Send_Data_Mutex);			//释放发送状态锁
}

/****************************************************************************************
*@Name............: SendStatusToLocal
*@Description.....: 发送数据报给本地
*@Parameters......: type		:报文类型
*					symbol		:标志位
*					preservation:保留字段，打印机状态时为打印单元序号，订单状态时则为订单序号
*@Return values...: void
*****************************************************************************************/
void SendStatusToLocal(req_type type, u8_t symbol, u32_t preservation)
{
	char sent_data[SEND_DATA_SIZE] = {0};	//状态报文和请求报文都是固定20字节
	int delayTimes;
	OS_EVENT *tmpSem;
	u8_t accFlag = 0; //接收应答标志
	u8_t timeCounter = 0;
	u8_t reSendTimes = 0;//重传次数
	
	
	if(type == printer_status){
		Pack_Req_Or_Status_Message(sent_data, PRINTER_STATUS, symbol, Get_Printer_ID(), 0, preservation);//此时的preservation是主控板打印单元序号或为0
		tmpSem = Ack_Rec_Printer_Sem;
		delayTimes = 4;//暂时设置为4
	}else if(type == order_status){
		Pack_Req_Or_Status_Message(sent_data, ORDER_STATUS, symbol, Get_Printer_ID(), Get_Order_Unix_Time(preservation), preservation);//此时的preservation为订单号
		tmpSem = Ack_Rec_Order_Sem;
		delayTimes = 20;//暂时设置为20
	}
	
	/*超时重传状态报文*/
	do{
		LocalSendData(sent_data, SEND_DATA_SIZE);//从串口发送报文
		
		do{
			timeCounter++;
			if(timeCounter == delayTimes){//计数delayTimes，如20次，延时时间为20*40=800ms，具体次数需要实际测试才能得出
				timeCounter = 0;
				reSendTimes++;
				break;
			}
			
			OSTimeDlyHMSM(0, 0, 0, 40);//每次延时为40ms，波特率为9600，每秒传输1200B，则传输一个20字节的状态报文需要(1 / 60)s，来回接收大概是40ms
			if(1 > OSSemAccept(tmpSem))
				accFlag = 1;
		}while(accFlag == 0);
		
		if(reSendTimes > 3)//重传3次后则不重传了
			break;
	}while(accFlag == 0);
}

void AckToLocal(u32_t orderNum)//打印机发送应答给本地
{
	char sent_data[SEND_DATA_SIZE] = {0};	//状态报文和请求报文都是固定20字节
	
	Pack_Req_Or_Status_Message(sent_data, ACK_STATUS, 0, Get_Printer_ID(), ORDER_ACK_LOCAL, orderNum);
	
	LocalSendData(sent_data, SEND_DATA_SIZE);//从串口发送报文
}

u8_t GetTypeOfAck(char *data)//获取应答类型
{
#define ACK_TYPE_OFFSET 11
	return (u8_t)(*(data + ACK_TYPE_OFFSET));
}

u32_t GetAckNumber(char *data)//获取应答序号
{
#define ACK_NUMBER_OFFSET 12
		return ((u32_t)(*(data + ACK_NUMBER_OFFSET)) << 24) + ((u32_t)(*(data + ACK_NUMBER_OFFSET + 1)) << 16)
			  + ((u32_t)(*(data + ACK_NUMBER_OFFSET + 2)) << 8) + (u32_t)(*(data + ACK_NUMBER_OFFSET + 3));
}

void DealAckFromLocal(char *ack)//解决来自本地的应答
{
	static u32_t printerAckNum = 0;
	static u32_t orderAckNum = 0;
	u32_t tmpAckNum;//临时应答号
	
	tmpAckNum = GetAckNumber(ack);
	if(GetTypeOfAck(ack) == PRINTER_ACK_LOCAL){
		if(printerAckNum != tmpAckNum){//判断应答号是否重复，即迟到
			printerAckNum = tmpAckNum;
			OSSemPost(Ack_Rec_Printer_Sem);
		}//迟到应答直接丢弃
	}else{
		if(orderAckNum != tmpAckNum){//判断应答号是否重复，即迟到
			orderAckNum = tmpAckNum;
			OSSemPost(Ack_Rec_Order_Sem);
		}//迟到应答直接丢弃
	}
}

void recOrder(char *data, u16_t len)//接收订单
{
	u16_t order_len;
	u16_t order_total_len;			//订单总共长度
	
	u16_t timeCount = 0;
	while(len < 20){
		len = GetBufLen(usart_buf);
		OSTimeDlyHMSM(0, 0, 0, 100);
	}
	//OSTimeDlyHMSM(0, 0, 4, 500);
	ANALYZE_DATA_2B((data + ORDER_HEAD_LAST_OFFSET), order_total_len);//获取订单长度
	order_len = order_total_len + ORDER_HEAD_LENGTH;
	//ReadAssignBytes(readFromBuf, order_len);//根据订单长度读取一定块的数据
	//DeleteBuf(order_len);//删除缓冲区，倘若接收应答时没有删除的缓冲区，也从这里被删除
	
	while(order_len > GetBufLen(usart_buf) && timeCount < 40){//超时接收订单20s，若在20s内没有接收完订单，则证明订单接收有错，清除缓冲区并应答失败
		OSTimeDlyHMSM(0, 0, 0, 500);
		timeCount++;
	}
	
	if(timeCount < 40 && True == CheckRec((u8_t *)data, order_len)){//检测订单是否正确
		ANALYZE_DATA_4B((data + ORDER_NUM_HEAD_OFFSET), current_order_number);//获取订单号
		
		if(current_order_number != last_order_number){//判断订单号是否重复
			put_in_buf((u8_t*)data, order_len, URGENT);//写入缓冲区
			ClearBuf();
			AckToLocal(current_order_number);//无论订单是否重复，都要应答给本地，接收到了订单
			last_order_number = current_order_number;
			OSSemPost(Print_Queue_Sem);
		}
		else{
			ClearBuf();
			AckToLocal(current_order_number);//无论订单是否重复，都要应答给本地，接收到了订单
		}
				
	}else{//检测失败
		DEBUG_PRINT("batch's checksum error\n");
		ClearBuf();
		SendStatusToLocal(order_status, ORDER_DATA_ERR, 0);//由于无法得知订单号，故填充订单号为0		
	}
}

Status recAck(u16_t offset)//接收应答
{
	u8_t *data;
	u16_t len = REC_DATA_SIZE + offset;
	//ReadAssignBytes(readFromBuf, len);//根据应答长度读取一定块的数据
	
	data = (u8_t*)(usart_buf.base + offset);//偏移表示无效的数据
	if(True == CheckRec((u8_t*)data, REC_DATA_SIZE)){//检测应答是否正确
		if(ACK_STATUS == (u8_t)(*(data + STATUS_TYPE_OFFSET)))
			DealAckFromLocal((char*)data);//判断应答类型
		else
			return Fail;
		
		//DeleteBuf(len);//删除缓冲区
		ClearBuf();
		return True;
	}else{
		return Fail;
	}
}

void local_receive(void)//接收本地的订单及应答
{
	char *data;
	char readFromBuf[20];
	u16_t len;//从缓冲区读取到的数据的长度
	u8_t os_err = 0;
	extern OS_EVENT *Local_Rec_Data_Sem;
	
	while(1){//串口接收缓冲区不为空
		OSSemPend(Local_Rec_Data_Sem, 0, &os_err);
		DEBUG_PRINT("receive data from local\n");
		while((len = GetBufLen(usart_buf)) < 2){
			OSTimeDlyHMSM(0, 0, 0, 50);//等待50ms以等待缓冲有数据
		}while(len < 2);
		

		ReadAssignBytes(readFromBuf, 20);//从串口缓冲区获取数据
		data = readFromBuf;
		len = GetBufLen(usart_buf);//缓冲区长度
		find_substr_head(&data, "\xCF\xFC", &len, 2);//寻找应答头
		if(len > 0) {//检测到是应答头了
			if(True == recAck(0)){
				continue;//接收应答成功
			}
		}
		
		/*
		**否则检测是否为订单头
		*/
		data = (char *)usart_buf.base;
		len = GetBufLen(usart_buf);//缓冲区长度
		find_substr_head(&data, "\x3e\x11", &len, 2);//寻找订单头
		if(len > 0) {//检测到是订单头了
			recOrder(data, len);//接收订单
		}else{//如果找不到订单或应答头，就清除缓冲区，并继续下一步
			ClearBuf();
			break;
		}
	}
}
