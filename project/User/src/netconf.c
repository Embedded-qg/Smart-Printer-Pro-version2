#include "netconf.h"

#define ORDER_NUM_MAX 10

struct netif DM9161_netif;
struct netconn *order_netconn;	//全局TCP链接

extern OS_EVENT *Print_Sem;
extern batch_info batch_info_table[];	//批次表
extern OS_EVENT *Batch_Rec_Sem;			//完成一次批次读取的二值信号量
extern OS_EVENT *Print_Queue_Sem;		//数据存入打印队列的信号量

struct ip_addr localhost_ip;
struct ip_addr localhost_netmask;
struct ip_addr localhost_gw;

//#define TCPSEVER
#define TCPCLIENT
#define REMOTE  1



/****************************************************************************************
*@Name............: put_in_buf
*@Description.....: 将数据存储至本地缓冲区
*@Parameters......: data		:存储数据的指针
*					len			:数据长度
*					urg			:加急标志
*@Return values...: void
*****************************************************************************************/
void put_in_buf(u8_t *data, u16_t len, u16_t urg)
{
	SqQueue *buf;
	u8_t err;
	s8_t buf_err;
	extern OS_EVENT *Print_Queue_Sem;
	
	if(urg)
		buf = &urgent_buf;
	else
		buf = &queue_buf;
	
	while(1){
		if(BUF_OK != (buf_err = Write_Buf(buf, data, len))){  //复制订单数据到缓冲区
			printf("buf %d write error, err is %d, len is %d.\n", urg, buf_err, len);
			OSSemPost(Print_Queue_Sem);//释放
			OSTimeDlyHMSM(0, 0, 1, 0);
		}else{
			return;
		}
	}
}


static u16_t contract_total_len = 0;//批次总共长度
static u16_t contract_number = 0;	//合同号
static u16_t batch_number = 0; //批次号
static u16_t last_bacth_number = 0;
static u32_t contract_partner = 0;
static char contract[33] = {0};
static char batch[21] = {0};
static u16_t len = 0;	//从网络缓冲区读取到的数据的长度
static u16_t count = 0;
static u16_t leave_len = 0; 
u8_t batch_flag = 0;//批次标记
u8_t batch_table_hash = 0;

/****************************************************************************************
*@Name............: read_message_from_netbuf
*@Description.....: 从网络缓冲区中读取整个报文数据
*@Parameters......: order_netbuf----一级网络缓冲区数据
										netbuf----------报文数据缓冲区
										data------------二级网络缓冲区
										netbuf----------数据存放缓冲区
*										len：二级网络缓冲区长度
*@Return 					：1 ----成功读取
										0 ----错误读取
*****************************************************************************************/
u8_t read_message_from_netbuf(struct netbuf* order_netbuf,char **netbuf,char ** data,u8_t netbuf_type,u16_t *len)
{
	u16_t max_length = 0,net_head_current_len = 0,sub_len = *len;//需要读取的报文的总长度，当前读取到的报文的长度;
	char *sub_data = *data;
	u8_t read_symbol = 0;//是否成功读取报文信息
	if(netbuf_type == NETORDER_CONTRACT) 	//读取合同网报文
	{
		max_length = MAX_CONTRACT_HEAD_LENGTH;
		while(sub_len > 0 && net_head_current_len < max_length)
		{
			contract[net_head_current_len++] = *sub_data++;
			sub_len--;		
		}		
		if(net_head_current_len == max_length)
			if(*(contract + CONTRACT_TAIL_OFFSET) == '\xfb' && *(contract + CONTRACT_TAIL_OFFSET + 1) == '\xbf') read_symbol = 1;		
		*netbuf = contract;
	}
	else if(netbuf_type == NETOREDER_BATCH)//读取批次报文
	{
		max_length = MAX_BATCH_HEAD_LENGTH;
		while(sub_len > 0 && net_head_current_len < max_length)
		{
			batch[net_head_current_len++] = *sub_data++;
			sub_len--;		
		}
		if(net_head_current_len == max_length)
				if(*(batch + BATCH_TAIL_OFFSET) == '\x55' && *(batch + BATCH_TAIL_OFFSET + 1) == '\xee') read_symbol = 1;
	}
	else if(netbuf_type == NETOREDER_ORDER)//读取订单数据
	{
		max_length = batch_info_table[batch_table_hash].batch_length - MAX_BATCH_HEAD_LENGTH;
		while(max_length >= sub_len)
		{
			put_in_buf((u8_t *)sub_data,sub_len,batch_info_table[batch_table_hash].preservation);
			max_length -= sub_len;
			if(netbuf_next(order_netbuf) > 0) netbuf_data(order_netbuf, (void **)&sub_data, &sub_len);//从网络缓冲区读取
		}
		put_in_buf((u8_t *)sub_data,max_length,batch_info_table[batch_table_hash].preservation);
		sub_len -= max_length;
		sub_data += max_length;
		read_symbol = 1;
	}
	*data = sub_data;
	*len = sub_len;
	return read_symbol;
}

//处理合同网报文
void deal_with_contract_order(char *contract_buf)
{
		u16_t contract_type;
		extern struct netconn *order_netconn;
		contract_type = contract_buf[CONTRACT_TYPE_OFFSET];
		printf("contract_type = %d\r\n",contract_type);				
		if(contract_type == CONTRACT_DOCUMENT)
		{
			if(!contract_partner)			contract_response(order_netconn,contract_type+1,0);
		}
		else if(contract_type == CONTRACT_SIGN)
		{
			Analyze_Contract_Info_Table(contract_buf);
			contract_response(order_netconn,contract_type+1,0);
			contract_partner = 1;
		}
		else if(contract_type == CONTRACT_ESCAPE)
		{
			contract_response(order_netconn,contract_type+1,0);
			contract_partner = 0;
		}
}

//处理批次报文
void deal_with_batch_order(char *batch_buf)
{
	ANALYZE_DATA_2B((batch + BATCH_NUMBER_OFFSET), batch_number);//获取批次号

	printf("batch read success ,batch_number is %x %x\n", *(batch + BATCH_NUMBER_OFFSET), *(batch + BATCH_NUMBER_OFFSET + 1));
	printf("batch read success ,batch_length is %x %x\n", *(batch + BATCH_TOTAL_LENGTH_OFFSET), *(batch + BATCH_TOTAL_LENGTH_OFFSET + 1));	
	Analyze_Batch_Info_Table(batch, batch_number);//批次解包
	
	batch_table_hash = get_batch_hash(batch_number);
	
	if(batch_number != last_bacth_number) batch_flag = 1;
}

void deal_with_order_order(void)
{
	SqQueue * tempBuf = 0;
	u32_t oldWritePtr = 0;
	tempBuf = (batch_info_table[batch_table_hash].preservation)?(&urgent_buf):(&queue_buf);
	oldWritePtr = ((tempBuf->write + tempBuf->MAX - (batch_info_table[batch_table_hash].batch_length - MAX_BATCH_HEAD_LENGTH) )%tempBuf->MAX );
	if(checkBufData(tempBuf,oldWritePtr) == 1){	//订单数据错误				
		INT8U err = 0;	
		printf("订单数据出错\r\n");
		OSMutexPend(tempBuf->mutex,0,&err);			//申请普通缓冲锁
		tempBuf->write = oldWritePtr;
		OSMutexPost(tempBuf->mutex);			//申请普通缓冲锁
	}										
	else{//订单数据正确						
		printf("接收订单成功!!\r\n");														
		OSSemPost(Print_Queue_Sem);
		OSSemPost(Batch_Rec_Sem); 
		NON_BASE_SEND_STATUS(batch_status, BATCH_ENTER_BUF, batch_number);//发送批次状态，进入缓冲区
		batch_flag  = 0;
	}
}

//处理报文
void deal_with_order(char *netbuf,u16_t netbuf_type)
{
	if(netbuf_type == NETORDER_CONTRACT)
	{
		deal_with_contract_order(netbuf);
	}
	if(netbuf_type == NETOREDER_BATCH)
	{
		deal_with_batch_order(netbuf);
	}
	if(netbuf_type == NETOREDER_ORDER)
	{
		deal_with_order_order();
	}
}
/**
*@brief 接收报文并解析
*/
void receive_connection(struct netconn *conn)
{
	struct netbuf* order_netbuf = NULL;
	err_t err;
	u8_t netbuf_type = 0,read_message_symbol;
	u16 i;
	char *data,*netbuf;
	while((err = netconn_recv(conn,&order_netbuf)) == ERR_OK)
	{
		netbuf_data(order_netbuf,(void **)&data,&len);//从网络缓冲区读
//		for(i = 0;i < len;i++)
//		{
//			printf("%x ",data[i]);
//		}
start:
		netbuf_type = find_order_head(&data,&len); //从网络缓冲区中读取第一个报文种类，分析报文种类
		printf("netbuf_type = %d\r\n",netbuf_type);	
		read_message_symbol = read_message_from_netbuf(order_netbuf,&netbuf,&data,netbuf_type,&len);//读取整个报文数据长度
		printf("read_message_symbol = %d\r\n",read_message_symbol);		
		printf("网络缓冲区剩余长度：len = %d\r\n",len);	
		deal_with_order(netbuf,netbuf_type);//对数据报文进行处理
		if(len > 0) goto start; //如有多余数据，重新进行报文判断
		if(!(netbuf_next(order_netbuf) > 0)) netbuf_delete(order_netbuf);
	}
	OSTimeDlyHMSM(0, 0, 0, 50);
}
	
//合同网协作回复
void contract_response(struct netconn *conn,contract_type type,u32_t preservation)
{
	char sent_data[MAX_CONTRACT_HEAD_LENGTH] = {0};
	err_t err;
	u16 i = 0;
#ifdef REMOTE	
	Pack_Contract_Message(sent_data,type,Get_MCU_ID(),Get_Current_Unix_Time(),Get_MCU_Speed(),Get_MCU_Status(),preservation);
	printf("type = %d,mcu_id = %d,time = %d,speed = %d,status = %d\r\n",type,(u16_t)Get_MCU_ID(),(u16_t)Get_Current_Unix_Time(),Get_MCU_Speed(),Get_MCU_Status());
#endif
	while(0 != (err = netconn_write(conn, sent_data, MAX_CONTRACT_HEAD_LENGTH, NETCONN_COPY))){
		if(ERR_IS_FATAL(err))//致命错误，表示没有连接
			break;
		
		//当网络写入错误时，需要等待一段时间后继续写入该数据包，否则无法反馈给服务器
		OSTimeDlyHMSM(0,0,++i,0);
		printf("\n\n\nNETCONN WRITE ERR_T IS %d\n\n\n", err);
	}	

}

/****************************************************************************************
*@Name............: write_connection
*@Description.....: 发送数据报
*@Parameters......: conn		:链接
*					type		:报文类型
*					symbol		:标志位
*					preservation:保留字段，批次中，高16位为批次序号，订单中时且低16位为批次内序号
*@Return values...: void
*****************************************************************************************/
void write_connection(struct netconn *conn, req_type type, u8_t symbol, u32_t preservation)
{
	char sent_data[SEND_DATA_SIZE] = {0};	//状态报文和请求报文都是固定20字节
	err_t err;
	int i = 0;
#ifdef REMOTE
	//先处理网络而非本地
	if(type == order_req){
		Pack_Req_Or_Status_Message(sent_data, ORDER_REQ, symbol, Get_MCU_ID(), 
									Get_Current_Unix_Time(), preservation);
	}
	else if(type == batch_status){//此时的preservation高16位为批次号
		Pack_Req_Or_Status_Message(sent_data, BATCH_STATUS, symbol, Get_MCU_ID(), 
									Get_Batch_Unix_Time((u16_t)preservation), preservation << 16);
	}
	else if(type == printer_status){//此时的preservation是主控板打印单元序号或为0
		Pack_Req_Or_Status_Message(sent_data, PRINTER_STATUS, symbol, Get_MCU_ID(), 
									Get_Current_Unix_Time(), preservation);
	}
	else if(type == order_status){//此时的preservation的高16位为批次号，低16位为批次内序号
		Pack_Req_Or_Status_Message(sent_data, ORDER_STATUS, symbol, Get_MCU_ID(), 
									Get_Batch_Unix_Time((u16_t)(preservation >> 16)), preservation);
	}
	else if(type == first_req){//请求第一次建立链接，发送主控板id
		Pack_Req_Or_Status_Message(sent_data, FIRST_REQ, symbol, Get_MCU_ID(), 
									Get_Current_Unix_Time(), preservation);	
	}

#endif
	
	while(0 != (err = netconn_write(conn, sent_data, SEND_DATA_SIZE, NETCONN_COPY))){
		if(ERR_IS_FATAL(err))//致命错误，表示没有连接
			break;
		
		//当网络写入错误时，需要等待一段时间后继续写入该数据包，否则无法反馈给服务器
		OSTimeDlyHMSM(0,0,++i,0);
		printf("\n\n\nNETCONN WRITE ERR_T IS %d\n\n\n", err);
		
		if(type != order_req){//订单请求需持续发送，否则服务器将无法下达订单
			if(i > 3) break;
		}else if(i > 10) break;//但等待多次后是无意义的
	}
		
}

/**
 * @brief	连接至远程服务器
 */
void con_to_server(void)
{
	struct ip_addr server_ip;
	extern struct netconn *order_netconn;	//全局TCP链接
	extern OS_EVENT *Recon_To_Server_Sem;
	if((order_netconn = netconn_new(NETCONN_TCP)) != NULL){
		printf("Connection build.\n");
	}else{
		printf("Fail to build connection.\n");
		OSSemPost(Recon_To_Server_Sem);	
	}
	
	netconn_set_recvtimeout(order_netconn,10000);//设置接收延时时间 
	
#ifdef REMOTE//设置服务器ip地址
	IP4_ADDR(&server_ip,10,21,48,11);//云服务器学校
//		IP4_ADDR(&server_ip,123,207,228,117);		//云服务器_胖子
//	IP4_ADDR(&server_ip,192,168,1,116);		//JockJo测试机
//	IP4_ADDR(&server_ip, 192,168,1,119);	//工作室测试机
//	IP4_ADDR(&server_ip,192,168,1,110);  //用肥虫电脑的IP
	netconn_connect(order_netconn,&server_ip,8086);
#else	
	IP4_ADDR(&server_ip,192,168,1,116);
	netconn_connect(order_netconn,&server_ip,8086);
#endif	
	
	write_connection(order_netconn, first_req, REQ_LINK_OK, 0);//初次请求建立发送主控板ID
	OSTimeDlyHMSM(0,0,1,0);
//	write_connection(order_netconn, order_req, ORDER_REQUEST, 0);//请求订单			此处可以修改为需要请求的内容
//	printf("Order req.\n");
	
	//发多次测试
#ifdef APP_DEBUG
	//write_connection(order_netconn, first_req, REQ_LINK_OK, 0);//初次请求建立
	//write_connection(order_netconn, first_req, REQ_LINK_OK, 0);//初次请求建立
#endif
}


/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void LwIP_Init(void)
{
	extern OS_EVENT *LWIP_Init_Sem;
	err_t err_start = ERR_OK;
	static char *str_ipaddr = (void*)0;
	tcpip_init(NULL,NULL);


#if LWIP_DHCP//动态ip分配
  localhost_ip.addr = 0;
  localhost_netmask.addr = 0;
  localhost_gw.addr = 0;
	printf("\nDHCP can be choosed !!!");
#else
//  IP4_ADDR(&localhost_ip, 192,168,1,135);
//  IP4_ADDR(&localhost_netmask, 255, 255, 255, 0);
//  IP4_ADDR(&localhost_gw, 192, 168, 1, 1);
//JockJo家中测试
  IP4_ADDR(&localhost_ip, 192,168,0,135);
  IP4_ADDR(&localhost_netmask, 255, 255, 255, 0);
  IP4_ADDR(&localhost_gw, 192, 168, 0, 1);	
	printf("\nDHCP is not  be choosed !!!\n");	
#endif

	netif_add(&DM9161_netif, &localhost_ip, &localhost_netmask, &localhost_gw, NULL, &ethernetif_init, &tcpip_input);
  netif_set_default(&DM9161_netif);
	
#if LWIP_DHCP
		while(  (err_start = dhcp_start(&DM9161_netif)) != ERR_OK)
		{
				printf("\nDHCP start failed!");
				//此处应该有不成功的处理函数为佳
		}
		while(DM9161_netif.ip_addr.addr == 0)
		{
			//等待一会，让IP地址存放近DM9161
				OSTimeDlyHMSM(0, 0, 1, 0);
				printf("\nDM9161 netif's  is  null!");
		}
		localhost_ip.addr = DM9161_netif.ip_addr.addr;	
		localhost_netmask.addr = DM9161_netif.netmask.addr;
		localhost_gw.addr = DM9161_netif.gw.addr;
		str_ipaddr = ipaddr_ntoa(&localhost_ip);
#endif
		
	str_ipaddr = ipaddr_ntoa(&localhost_ip);
	if(str_ipaddr != (void*)0)
	{
		printf("\nstr_ipaddr is:");		
		printf(str_ipaddr);
	}
	else
	{
		printf("\nstr_ipaddr is null");		
	}
	
//#ifdef LWIP_IGMP
//	while(  (err_start = igmp_start(&DM9161_netif)) != ERR_OK)
//	{
//			printf("igmp start failed!");
//				//此处应该有不成功的处理函数为佳
//	}
//#endif	//LWIP_IGMP
	
/*  When the netif is fully configured this function must be called.*/
  netif_set_up(&DM9161_netif);
	OSSemPost(LWIP_Init_Sem);			//释放成信号
}


	