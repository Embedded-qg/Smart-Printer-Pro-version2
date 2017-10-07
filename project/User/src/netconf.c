#include "netconf.h"

#define ORDER_NUM_MAX 10

struct netif DM9161_netif;
struct netconn *order_netconn;	//全局TCP链接

extern OS_EVENT *Print_Sem;
extern batch_info batch_info_table[];	//批次表
extern OS_EVENT *Batch_Rec_Sem;			//完成一次批次读取的二值信号量

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
			DEBUG_PRINT("buf %d write error, err is %d, len is %d.\n", urg, buf_err, len);
			OSSemPost(Print_Queue_Sem);//释放
			OSTimeDlyHMSM(0, 0, 1, 0);
		}else{
			return;
		}
	}
}

static u16_t batch_head_current_len = 0;//当前读取到的批次的长度
static u16_t batch_total_len = 0;//批次总共长度
static u16_t batch_number = 0;	//批次号
static u16_t last_batch_number = 0;
static char batch[21] = {0};
static u16_t len = 0;	//从网络缓冲区读取到的数据的长度
static u8_t flag = 0;	//1表示已经读取了批次头，否则没有
static u16_t count = 0;
static u16_t leave_len = 0;

/**
 * @brief	开启新的批次头的初始化
 */
static void begin_new_batch(void)
{
	batch[0] = '\0';
	batch_head_current_len = 0;//新的批次
	batch_total_len = 0;
	flag = 0;
	count = 0;
	last_batch_number = batch_number;
}

/**
 * @brief	接收报文并解析
 */
void receive_connection(struct netconn *conn)
{
	struct netbuf *order_netbuf = NULL;
	extern OS_EVENT *Print_Queue_Sem;
	u8_t hash;//批次哈希值
	err_t err;

	extern OS_EVENT *Recon_To_Server_Sem;
	
	while((err = netconn_recv(conn, &order_netbuf)) == ERR_OK)
	{		
		char *data;
start:
		while(batch_head_current_len < MAX_BATCH_HEAD_LENGTH)
		{
			netbuf_data(order_netbuf, (void **)&data, &len);//从网络缓冲区读取
			/*考虑上一个缓冲区读取完一个批次后还剩下一些数据，故需要重新读取这个缓冲区，并加上被读取过后的偏移量，目的是让data指针指向未读的数据*/
			data += leave_len;
			len -= leave_len;
			DEBUG_PRINT("get data ,len is %d\n", len);
			
			while(len > 0 && batch_head_current_len < MAX_BATCH_HEAD_LENGTH)
			{
				DEBUG_PRINT("len haa is %d\n", len);
				if(batch[0] != '\xaa')//一直找寻不到批次头，所以一直寻找
				{
					find_substr_head(&data, "\xaa\x55", &len, 2);
					DEBUG_PRINT("len xaa is %d\n", len);
					if(len == 0)
					{
						DEBUG_PRINT("There is no data\n");
					}
				}
				
				if(len > 0)//len大于0代表检测到批次头并且data指针还指向的缓冲区还有剩余容量
				{
					batch[batch_head_current_len++] = *data++;
					count++;
					DEBUG_PRINT("batch[0] is %x, *data - 1 is %x, *data is %x, *data + 1 is %x, len is %d, batch_head_current_len is %d\n", batch[0], *(data - 1), *data, *(data + 1), len, batch_head_current_len);
					
					if(batch_head_current_len == MAX_BATCH_HEAD_LENGTH)
					{
						if(*(batch + BATCH_TAIL_OFFSET) == '\x55' && *(batch + BATCH_TAIL_OFFSET + 1) == '\xaa')//表示是批次尾
						{
							flag = 1;
							ANALYZE_DATA_2B((batch + BATCH_NUMBER_OFFSET), batch_number);//获取批次号
							
							DEBUG_PRINT("batch read success ,batch_number is %x %x\n", *(batch + BATCH_NUMBER_OFFSET), *(batch + BATCH_NUMBER_OFFSET + 1));
							DEBUG_PRINT("batch read success ,batch_length is %x %x\n", *(batch + BATCH_TOTAL_LENGTH_OFFSET), *(batch + BATCH_TOTAL_LENGTH_OFFSET + 1));
//							printf("batch read success ,batch_number is %d, ascii is %x %x \n", batch_number, *(batch + BATCH_NUMBER_OFFSET), *(batch + BATCH_NUMBER_OFFSET + 1));
							
							Analyze_Batch_Info_Table(batch, batch_number);//批次解包
							
							hash = get_batch_hash(batch_number);
//							printf("batch read success ,batch_length is %d ascii is %x %x\n", batch_info_table[hash].batch_length, *(batch + BATCH_TOTAL_LENGTH_OFFSET),  *(batch + BATCH_TOTAL_LENGTH_OFFSET + 1));
							len -= count;
//							DEBUG_PRINT("read from netconf1\n");

							batch_total_len = MAX_BATCH_HEAD_LENGTH + len;
							if (batch_total_len >= batch_info_table[hash].batch_length)
							{
								u16_t sub_len;
								sub_len =  len - (batch_total_len - batch_info_table[hash].batch_length);//从网络缓冲区读取到多于一个批次的数据，则只保留前面的数据
								
								if(batch_number != last_batch_number){
									SqQueue * tempBuf = 0;
									u32_t oldWritePtr = 0;
									
									if(batch_info_table[hash].preservation){
										tempBuf = &urgent_buf;										
									}
									else{
										tempBuf = &queue_buf;
									}	
									if(sub_len)
										put_in_buf(data, sub_len, batch_info_table[hash].preservation);//保存订单数据至缓冲区
									
									oldWritePtr = ((tempBuf->write + tempBuf->MAX - (batch_info_table[hash].batch_length - 20) )%tempBuf->MAX );
	
									if(checkBufData(tempBuf,oldWritePtr) == 1){	//订单数据错误											
										INT8U err = 0;
										OSMutexPend(tempBuf->mutex,0,&err);			//申请普通缓冲锁
										tempBuf->write = oldWritePtr;
										OSMutexPost(tempBuf->mutex);			//申请普通缓冲锁
									}										
									else{//订单数据正确																			
										OSSemPost(Print_Queue_Sem);
										OSSemPost(Batch_Rec_Sem); 
										NON_BASE_SEND_STATUS(batch_status, BATCH_ENTER_BUF, batch_number);//发送批次状态，进入缓冲区
									}
								}
								
								begin_new_batch();
								//DEBUG_PRINT("batch read enough1!\n");
//								printf("batch read success ,batch_number is %d\n", batch_number);
								//DEBUG_PRINT("PRINT_QUEUE_SEM\n");
								leave_len = len - sub_len;					
								if(leave_len > 0)
									goto start;
								else
									break;
								
							}
							else
							{
								DEBUG_PRINT("Continue put in buf 1\n");
								if(batch_number != last_batch_number)
									put_in_buf(data, len, batch_info_table[hash].preservation);//保存订单数据至缓冲区
								DEBUG_PRINT("Continue put in buf 2\n");
							}
							DEBUG_PRINT("batch read success ,len is %d,count is %d, btl is %d\n", len, count, batch_total_len);
						}
						else
						{
							DEBUG_PRINT("Detect error\n");
							begin_new_batch();
						}
					}
					if(count == len)//数据读取完，结束操作
					{
						count = 0;
						len = 0;
						break;
					}
				}
			}
			if(!(netbuf_next(order_netbuf) > 0))
			{
				goto delete;
			}
		}
		
		DEBUG_PRINT("batch_head_current_len is %d\n", batch_head_current_len);
		do
		{
			if(flag == 0)
				goto start;
			
			DEBUG_PRINT("batch head read finished, bhcl is %d, btl is %d, batch_number is %d, batlen is %d\n", batch_head_current_len, batch_total_len, batch_number, get_batch_length(batch_number));
			netbuf_data(order_netbuf, (void **)&data, &len);
			DEBUG_PRINT("we are in new orderbuf, btl is %d, len is %d\n", batch_total_len, len);

			//DEBUG_PRINT("%s, %d\n", data, batch_total_len);
			
			batch_total_len += len;
			
			if (batch_total_len >= batch_info_table[hash].batch_length)
			{
				u16_t sub_len;
				sub_len =  len - (batch_total_len - batch_info_table[hash].batch_length);//从网络缓冲区读取到多于一个批次的数据，则只保留前面的数据
				
				if(batch_number != last_batch_number){					
					SqQueue * tempBuf = 0;
					u32_t oldWritePtr = 0;
					if(batch_info_table[hash].preservation){
						tempBuf = &urgent_buf;										
					}
					else{
						tempBuf = &queue_buf;
					}		
					if(sub_len)
						put_in_buf(data, sub_len, batch_info_table[hash].preservation);//保存订单数据至缓冲区
					oldWritePtr = ((tempBuf->write + tempBuf->MAX - (batch_info_table[hash].batch_length - 20) )%tempBuf->MAX );
			
					if(checkBufData(tempBuf , oldWritePtr) == 1){	//订单数据错误											
						INT8U err = 0;
						OSMutexPend(tempBuf->mutex,0,&err);			//申请普通缓冲锁
						tempBuf->write = oldWritePtr;
						OSMutexPost(tempBuf->mutex);			//申请普通缓冲锁
					}										
					else{//订单数据正确																			
						OSSemPost(Print_Queue_Sem);
						OSSemPost(Batch_Rec_Sem);						
						NON_BASE_SEND_STATUS(batch_status, BATCH_ENTER_BUF, batch_number);//发送批次状态，进入缓冲区
					}		

				}
				
				begin_new_batch();
//				printf("batch read success ,batch_number is %d\n", batch_number);
				//DEBUG_PRINT("PRINT_QUEUE_SEM\n");
				leave_len = len - sub_len;															
				if(leave_len > 0)//若有剩余数据则继续返回检测是否是一个新的批次
					goto start;
				
			}
			else
			{
				if(batch_number != last_batch_number)
					put_in_buf(data, len, batch_info_table[hash].preservation);//保存普通订单数据至缓冲区
			}
		}while(netbuf_next(order_netbuf) > 0);
delete:
		netbuf_delete(order_netbuf);
		DEBUG_PRINT("go to delete\n");
	}
	OSTimeDlyHMSM(0, 0, 0, 50);
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
		Pack_Req_Or_Status_Message(sent_data, ORDER_REQ, symbol, Get_Printer_ID(), 
									Get_Current_Unix_Time(), preservation);
	}
	else if(type == batch_status){//此时的preservation高16位为批次号
		Pack_Req_Or_Status_Message(sent_data, BATCH_STATUS, symbol, Get_Printer_ID(), 
									Get_Batch_Unix_Time((u16_t)preservation), preservation << 16);
	}
	else if(type == printer_status){//此时的preservation是主控板打印单元序号或为0
		Pack_Req_Or_Status_Message(sent_data, PRINTER_STATUS, symbol, Get_Printer_ID(), 
									Get_Current_Unix_Time(), preservation);
	}
	else if(type == order_status){//此时的preservation的高16位为批次号，低16位为批次内序号
		Pack_Req_Or_Status_Message(sent_data, ORDER_STATUS, symbol, Get_Printer_ID(), 
									Get_Batch_Unix_Time((u16_t)(preservation >> 16)), preservation);
	}
	else if(type == first_req){//请求第一次建立链接，发送主控板id
		Pack_Req_Or_Status_Message(sent_data, FIRST_REQ, symbol, Get_Printer_ID(), 
									Get_Current_Unix_Time(), preservation);	
	}

#endif
	
	while(0 != (err = netconn_write(conn, sent_data, SEND_DATA_SIZE, NETCONN_COPY))){
		if(ERR_IS_FATAL(err))//致命错误，表示没有连接
			break;
		
		//当网络写入错误时，需要等待一段时间后继续写入该数据包，否则无法反馈给服务器
		OSTimeDlyHMSM(0,0,++i,0);
		DEBUG_PRINT("\n\n\nNETCONN WRITE ERR_T IS %d\n\n\n", err);
		
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
		DEBUG_PRINT("Connection build.\n");
	}else{
		DEBUG_PRINT("Fail to build connection.\n");
		OSSemPost(Recon_To_Server_Sem);	
	}
	
	netconn_set_recvtimeout(order_netconn,10000);//设置接收延时时间 
	
#ifdef REMOTE//设置服务器ip地址
//	IP4_ADDR(&server_ip,10,21,48,11);//云服务器学校
//		IP4_ADDR(&server_ip,123,207,228,117);		//云服务器_胖子
//	IP4_ADDR(&server_ip,192,168,1,116);		//JockJo测试机
	IP4_ADDR(&server_ip, 192,168,1,119);	//工作室测试机
//	IP4_ADDR(&server_ip,192,168,1,110);  //用肥虫电脑的IP
	netconn_connect(order_netconn,&server_ip,8086);
#else	
	IP4_ADDR(&server_ip,192,168,1,116);
	netconn_connect(order_netconn,&server_ip,8086);
#endif	
	
	write_connection(order_netconn, first_req, REQ_LINK_OK, 0);//初次请求建立发送主控板ID
	OSTimeDlyHMSM(0,0,1,0);
	write_connection(order_netconn, order_req, ORDER_REQUEST, 0);//请求订单			此处可以修改为需要请求的内容
	DEBUG_PRINT("Order req.\n");
	
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


#if LWIP_DHCP
  localhost_ip.addr = 0;
  localhost_netmask.addr = 0;
  localhost_gw.addr = 0;
	DEBUG_PRINT("DHCP can be choosed !!!\n");
#else
//  IP4_ADDR(&localhost_ip, 192,168,1,135);
//  IP4_ADDR(&localhost_netmask, 255, 255, 255, 0);
//  IP4_ADDR(&localhost_gw, 192, 168, 1, 1);
//JockJo家中测试
  IP4_ADDR(&localhost_ip, 192,168,0,135);
  IP4_ADDR(&localhost_netmask, 255, 255, 255, 0);
  IP4_ADDR(&localhost_gw, 192, 168, 0, 1);	
	DEBUG_PRINT("DHCP is not  be choosed !!!\n");	
#endif

	netif_add(&DM9161_netif, &localhost_ip, &localhost_netmask, &localhost_gw, NULL, &ethernetif_init, &tcpip_input);
  netif_set_default(&DM9161_netif);
	
#if LWIP_DHCP
		while(  (err_start = dhcp_start(&DM9161_netif)) != ERR_OK)
		{
				DEBUG_PRINT("DHCP start failed!");
				//此处应该有不成功的处理函数为佳
		}
		while(DM9161_netif.ip_addr.addr == 0)
		{
			//等待一会，让IP地址存放近DM9161
				OSTimeDlyHMSM(0, 0, 1, 0);
				DEBUG_PRINT("DM9161 netif's  is  null!");
		}
		localhost_ip.addr = DM9161_netif.ip_addr.addr;	
		localhost_netmask.addr = DM9161_netif.netmask.addr;
		localhost_gw.addr = DM9161_netif.gw.addr;
		str_ipaddr = ipaddr_ntoa(&localhost_ip);
#endif
		
	str_ipaddr = ipaddr_ntoa(&localhost_ip);
	if(str_ipaddr != (void*)0)
	{
		DEBUG_PRINT("str_ipaddr is:\n");		
		DEBUG_PRINT(str_ipaddr);
	}
	else
	{
		DEBUG_PRINT("str_ipaddr is null");		
	}
	
#ifdef LWIP_IGMP
	while(  (err_start = igmp_start(&DM9161_netif)) != ERR_OK)
	{
			DEBUG_PRINT("igmp start failed!");
				//此处应该有不成功的处理函数为佳
	}
#endif	//LWIP_IGMP
	
/*  When the netif is fully configured this function must be called.*/
  netif_set_up(&DM9161_netif);
	OSSemPost(LWIP_Init_Sem);			//释放成信号
}


	