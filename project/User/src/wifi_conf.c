#include "wifi_conf.h"

#define ORDER_LENGTH_LAST_OFFSET	4//订单头的订单长度的最后字节位置
#define ORDER_HEAD_LAST_OFFSET		2//订单头的标志的最后字节位置
#define ORDER_NUM_HEAD_OFFSET		12//订单序号头偏移
#define URGENT						1//加急为1
#define READ_MAX_SIZE				(1024 * 10)
#define BUF_IS_NOT_EMPTY			0//缓冲区不为空
#define BUF_IS_EMPTY				1//缓冲区为空
#define ORDER_NUM_LENGTH			4//订单序号长度
#define ORDER_CREATE_TIME			4//订单生成时间
#define ORDER_REQ_LENGTH			12//请求订单长度
#define IPADD_OFFSET				8//ip地址偏移
#define ORDER_HEAD_LENGTH			28//订单中除数据外的字节长度
#define ACK_LENGTH					20//应答长度

/**************************************************************
*	Global Variable Section
**************************************************************/
extern SqQueue wifi_buf;				//串口缓冲区
static u32_t last_order_number = 0;		//上一次接收的订单号
static u32_t current_order_number = 0;	//本次接收的订单号
extern OS_EVENT *Print_Queue_Sem;
/**************************************************************
*	Function Define Section
**************************************************************/

void UART6_Hook(u8_t ch)//将从串口6读取到的数据放入缓冲区
{
	extern OS_EVENT *Wifi_Rec_Data_Sem;
	
	if((wifi_buf.write + 1) % wifi_buf.MAX == wifi_buf.read)
		return;
	
	wifi_buf.base[wifi_buf.write++] = ch;
	wifi_buf.write = wifi_buf.write % wifi_buf.MAX;
	if(wifi_buf.buf_empty == BUF_IS_EMPTY)
		OSSemPost(Wifi_Rec_Data_Sem);
	
	wifi_buf.buf_empty = BUF_IS_NOT_EMPTY;    //缓冲不为空
}

//获取缓冲区长度
#define GetBufLen(buf) \
	(((buf.write) + (buf.MAX)- (buf.read)) % buf.MAX)

void WifiReadBytes(u8_t *data, u16_t *len)//读取串口缓冲区，有多少读多少，但不移动缓冲区指针
{
	u16_t i;
	u16_t buf_len = 0;

	if(!wifi_buf.buf_empty){//缓冲区不为空
	
		buf_len = GetBufLen(wifi_buf);//缓冲区长度
		
		if(buf_len > READ_MAX_SIZE)
			buf_len = READ_MAX_SIZE;//最多只读缓存量

		for(i = 0; i < buf_len; ++i)
			*data++ = wifi_buf.base[(wifi_buf.read + i) % wifi_buf.MAX];
	}
	
	*len = buf_len;
}

s8_t WifiReadAssignBytes(u8_t *data, u16_t len)//读取串口缓冲区，指定长度为len
{
	u16_t i;
	u16_t buf_len;
	s8_t buf_err = BUF_EMPTY;
	u16_t timeCount = 0;
	
	do{
		if(!wifi_buf.buf_empty){//缓冲区不为空
		
			buf_len = GetBufLen(wifi_buf);//缓冲区长度
			
			if(buf_len < len){
				buf_err = BUF_ARG_ERR;
				if(++timeCount >= 5)
					return buf_err;
				DEBUG_PRINT("Wait for data from wifi\n");
				OSTimeDlyHMSM(0, 0, 0, 100);//等待有数据
				DEBUG_PRINT("Continue to read data from wifi\n");
			}else{
				for(i = 0; i < len; ++i)
					*data++ = wifi_buf.base[(wifi_buf.read + i) % wifi_buf.MAX];
				DEBUG_PRINT("Assign read data from wifi ok\n");
				buf_err = BUF_OK;
			}
		}else{
			DEBUG_PRINT("Wait for data from wifi\n");
			OSTimeDlyHMSM(0, 0, 0, 100);//等待有数据
			DEBUG_PRINT("Continue to read data from wifi\n");
		}
		
	}while(buf_err != BUF_OK);
	
	return BUF_OK;
}

static void ClearBuf(void)
{
	wifi_buf.read = wifi_buf.write = 0;
	wifi_buf.buf_empty = BUF_IS_EMPTY;
}

s8_t WifiDeleteBuf(u16_t len)//删除长度为len的缓冲区
{
	s8_t buf_err;
	u16_t buf_len;
	
	if(wifi_buf.buf_empty)
		buf_err = BUF_ARG_ERR;
	
	buf_len = GetBufLen(wifi_buf);//缓冲区长度	
	if(buf_len < len)
		buf_err = BUF_ARG_ERR;
	
	if(buf_err == BUF_ARG_ERR){
		ClearBuf();
		return buf_err;
	}
	
	wifi_buf.read = (wifi_buf.read + len) % wifi_buf.MAX;
	if(wifi_buf.read == wifi_buf.write){
		wifi_buf.buf_empty = BUF_IS_EMPTY;
	}
	DEBUG_PRINT("wifi_buf.write is %lu, wifi_buf.read is %lu\n", wifi_buf.write, wifi_buf.read);
	return BUF_OK;
}

void WifiSendData(u8_t *data, u16_t len)//从串口发送数据
{
	int i;
	INT8U err;
	
	extern OS_EVENT *Wifi_Send_Data_Mutex;		//wifi发送状态锁
	
	OSMutexPend(Wifi_Send_Data_Mutex, 0, &err);	//申请发送状态锁
	for(i = 0; i < len; ++i)
		USART_SendData(USART6, *(data + i));	//暂定从串口6输出
	
	OSMutexPost(Wifi_Send_Data_Mutex);			//释放发送状态锁
}

void SendStatusToWifi(req_type type, u8_t symbol, u32_t IPadd, u32_t preservation)
{
	u8_t sent_data[SEND_DATA_SIZE] = {0};	//状态报文和请求报文都是固定20字节
	
	if(type == printer_status){
		Pack_Req_Or_Status_Message((char *)sent_data, PRINTER_STATUS, symbol, Get_Printer_ID(), 0, preservation);//此时的preservation是主控板打印单元序号或为0
	}else if(type == order_status){
		Pack_Req_Or_Status_Message((char *)sent_data, ORDER_STATUS, symbol, Get_Printer_ID(), IPadd, preservation);//此时的preservation为订单号
	}else if(type == wifi_order_req){
		Pack_Req_Or_Status_Message((char *)sent_data, WIFI_REQ_ACK, symbol, Get_Printer_ID(), IPadd, preservation);//此时的preservation为保留
	}

	WifiSendData(sent_data, SEND_DATA_SIZE);//从串口发送报文
		
}

void AckToWifi(u32_t orderNum, u32_t ipAdd)//打印机发送应答给本地
{
	u8_t sent_data[SEND_DATA_SIZE] = {0};	//状态报文和请求报文都是固定20字节
	
	Pack_Req_Or_Status_Message((char *)sent_data, ACK_STATUS, 0, Get_Printer_ID(), ipAdd, orderNum);
	
	WifiSendData(sent_data, SEND_DATA_SIZE);//从串口发送报文
}

void AnalyzeData(char *data, u8_t offset, u8_t srcLen, u16_t *dst)
{
	if((wifi_buf.read + offset) <= wifi_buf.MAX){
		if((wifi_buf.read + offset + srcLen) <= wifi_buf.MAX){ //待解析数据未区域的情况
			if(srcLen == 4)
				ANALYZE_DATA_4B(data + offset, (*dst));
			else if(srcLen == 2)
				ANALYZE_DATA_2B(data + offset, (*dst));
		}else{//待解析数据跨区域的情况
			u8_t FirstLen = wifi_buf.MAX - (wifi_buf.read + offset);
			u8_t SecondLen = srcLen - FirstLen;
			Analyze_Data_With_Diff_Part((u8_t*)data, FirstLen, wifi_buf.base, SecondLen, (u32_t*)dst);
		}
	}else if((wifi_buf.read + offset + srcLen) > wifi_buf.MAX){ //待解析数据在另一端的情况
		offset = offset - (wifi_buf.MAX - wifi_buf.read);
		
		if(srcLen == 4)
			ANALYZE_DATA_4B(data + offset, (*dst));
		else if(srcLen == 2)
			ANALYZE_DATA_2B(data + offset, (*dst));
	}
}

Status WifiCheckRec(u8_t *data, u16_t len)//检测接收报文是否正确
{
	u16_t sum;
	u16_t subLen = wifi_buf.MAX - wifi_buf.read;
	if(subLen > len){
		sum = Check_Sum((u16_t*)data, len);
	}else{
		sum = Check_Sum_With_Diff_Part((u16_t*)data, subLen, (u16_t*)wifi_buf.base, len - subLen);
	}
	
	if(0 == sum)
		return True;
	else
		return Fail;
}

/* 将wifi缓冲区订单写入加急缓冲区 */
void putbuf(u8_t *data, u16_t len)
{
	u16_t subLen = wifi_buf.MAX - wifi_buf.read;
	if(subLen > len){
		DEBUG_PRINT("put in buf in one part\n");
		put_in_buf(data, len, URGENT);
	}else{
		/* 鉴于优先级最高，故两次填充不会被中断，但还是存在风险 */
		DEBUG_PRINT("put in buf in two part\n");
		put_in_buf(data, subLen, URGENT);
		put_in_buf(wifi_buf.base, len - subLen, URGENT);
	} 
}

#define KB (1024)
void ChangeCpa(Heap *heap, u16_t len)
{
	INT8U err;
	extern OS_EVENT *Capacity_Change_Sem;	//wifi缓冲区容量改变信号
	
	OSMutexPend(heap->mutex, 0, &err);	//申请堆控制锁
	{
		heap->capcity += len;
		DEBUG_PRINT("-------------Delete an order, heap capcity is %lu---------\n", heap->capcity);
		OSSemPost(Capacity_Change_Sem);
	}
	OSMutexPost(heap->mutex);	//释放堆控制锁
}

void WifiRecOrder(u8_t *data)//接收订单
{
	u16_t order_len;
	u16_t order_total_len;			//订单总共长度
	u32_t ipAdd;
	u16_t timeCount = 0;
	u8_t readFromBuf[ORDER_HEAD_LENGTH];	//订单头缓冲区
	extern Heap heap;
	
	WifiReadAssignBytes(readFromBuf, ORDER_HEAD_LENGTH);
	
	ANALYZE_DATA_2B((readFromBuf + ORDER_HEAD_LAST_OFFSET), order_total_len);//获取订单长度
	order_len = order_total_len + ORDER_HEAD_LENGTH;
//	WifiReadAssignBytes(readFromBuf, order_len);//根据订单长度读取一定块的数据
//	WifiDeleteBuf(order_len);//删除缓冲区，倘若接收应答时没有删除的缓冲区，也从这里被删除
	
	while(order_len > GetBufLen(wifi_buf) && timeCount < 20){//超时接收订单10s，若在10s内没有接收完订单，则证明订单接收有错，清除缓冲区并应答失败
		timeCount++;
		DEBUG_PRINT("Wait for data from wifi, timeCount is %u\n", timeCount);
		OSTimeDlyHMSM(0, 0, 0, 500);//等待有数据
		DEBUG_PRINT("Continue to read data from wifi\n");
	}
		
	ANALYZE_DATA_4B((readFromBuf + IPADD_OFFSET), ipAdd);//获取ip地址
	if(timeCount < 20 && True == WifiCheckRec((u8_t *)data, order_len)){//检测订单是否正确
		ANALYZE_DATA_4B((readFromBuf + ORDER_NUM_HEAD_OFFSET), current_order_number);//获取订单号
		
//		if(current_order_number != last_order_number){//判断订单号是否重复
			putbuf(data, order_len);//写入缓冲区
			AckToWifi(current_order_number, ipAdd);//无论订单是否重复，都要应答给本地，接收到了订单
			DEBUG_PRINT("receive a new order\n");
			last_order_number = current_order_number;
			OSSemPost(Print_Queue_Sem);
//		}else{
//			AckToWifi(current_order_number, ipAdd);//无论订单是否重复，都要应答给本地，接收到了订单
//		}
	}else{//检测失败
		DEBUG_PRINT("order's checksum error\n");
		SendStatusToWifi(order_status, ORDER_DATA_ERR, ipAdd, 0);//由于无法得知订单号，故填充订单号为0
	}
	
	WifiDeleteBuf(order_len);
	ChangeCpa(&heap, order_len);//通知缓冲区容量改变
}

Status recReq(u8_t *data)//接收应答
{
	if(True == CheckRec(data, ACK_LENGTH)){
		if(WIFI_ORDER_REQ == (u8_t)(*(data + STATUS_TYPE_OFFSET))){
			INT8U err;
			extern Heap heap;
			extern OS_EVENT *Rec_Wifi_Req_Sem;		//获取到wifi订单请求信号
			Request req;
			
			/* 解析请求订单报文 */
			ANALYZE_DATA_4B((data + ORDER_CREATE_TIME), req.recTime);//获取订单生成时间
			ANALYZE_DATA_4B((data + IPADD_OFFSET), req.ipAdd);//获取ip地址
			ANALYZE_DATA_4B((data + ORDER_REQ_LENGTH), req.orderLen);//获取订单长度
			
			if(req.orderLen > 10 * KB){
				DEBUG_PRINT("Wifi Req Order is more than 10 KB.\n");
				return Fail;
			}
			
			DEBUG_PRINT("-------wifi pend heap mutex!-------\n");
			OSMutexPend(heap.mutex, 0, &err);	//申请堆控制锁
			InsertHeap(&heap, &req);	//插入堆
			DEBUG_PRINT("-------receive a wifi order request!------\n");
			OSMutexPost(heap.mutex);	//释放堆控制锁
			DEBUG_PRINT("-------wifi post heap mutex!-------\n");
			OSSemPost(Rec_Wifi_Req_Sem);	//通知已经收到订单请求
			WifiDeleteBuf(ACK_LENGTH);
		}else{
			DEBUG_PRINT("-------fail to receive a wifi order request!-------\n");
			return Fail;
		}
	}
}

void wifi_receive(void)//接收本地的订单及应答
{
	u8_t *data;
	u16_t len;//从缓冲区读取到的数据的长度
	u16_t subLen;
	u8_t os_err = 0;
	extern OS_EVENT *Wifi_Rec_Data_Sem;
	u8_t readFromBuf[ORDER_HEAD_LENGTH];
	
	while(1){//串口接收缓冲区不为空
		OSSemPend(Wifi_Rec_Data_Sem, 0, &os_err);
		DEBUG_PRINT("receive data from wifi\n");
		while(!wifi_buf.buf_empty){
			while((len = GetBufLen(wifi_buf)) < 20){
				OSTimeDlyHMSM(0, 0, 0, 50);//等待50ms以等待缓冲有数据
				DEBUG_PRINT("Wait for data for 50ms from wifi\n");
			}while(len < 2);
			
			WifiReadAssignBytes(readFromBuf, ACK_LENGTH);//从串口缓冲区获取数据
			data = readFromBuf;
			len = ACK_LENGTH;
			find_substr_head((char **)(&data), "\xCF\xFC", &len, 2);//寻找订单请求头
			
			if(len > 0) {//检测到是订单请求头
				DEBUG_PRINT("detect req from wifi\n");
				if(len < ACK_LENGTH) {
					WifiDeleteBuf(ACK_LENGTH - len);
					WifiReadAssignBytes(readFromBuf, ACK_LENGTH);//从串口缓冲区获取数据
				}
				if(True == recReq(readFromBuf)){
					continue;//接收订单请求成功
				}
			}
			
			data = wifi_buf.base + wifi_buf.read;
			len = GetBufLen(wifi_buf);
			DEBUG_PRINT("Before detect orderhead ,len is %u\n", len);
			subLen = len = (len > ORDER_HEAD_LENGTH) ? ORDER_HEAD_LENGTH : len;
			find_substr_head((char **)(&data), "\x3e\x11", &len, 2);//寻找订单头
			DEBUG_PRINT("After detect orderhead ,sublen is %u,len is %u\n",subLen, len);
			if(len > 0) {//检测到是订单头了
				DEBUG_PRINT("Detect order from wifi\n");
				WifiDeleteBuf(subLen - len);
				WifiRecOrder(wifi_buf.base + wifi_buf.read);//接收订单
			}else{//如果找不到订单或应答头，就清除缓冲区，并继续下一步
				WifiDeleteBuf(subLen);
			}
		}
	}
}
