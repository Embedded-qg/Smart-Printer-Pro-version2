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

static u16_t net_head_current_len = 0;//当前读取到的合同的长度
static u16_t contract_total_len = 0;//批次总共长度
static u16_t contract_number = 0;	//批次号
static u16_t last_contract_number = 0;
static char contract[33] = {0};
static u16_t len = 0;	//从网络缓冲区读取到的数据的长度
static u8_t flag = 0;	//1表示已经读取了批次头，否则没有
static u16_t count = 0;
static u16_t leave_len = 0;

/**
 * @brief	开启新的批次头的初始化
 */
static void begin_new_contract(void)
{
	contract[0] = '\0';
	net_head_current_len = 0;//新的合同
	contract_total_len = 0;
	flag = 0;
	count = 0;
	last_contract_number = contract_number;
}

/**
*@brief 接收报文并解析
*/
void receive_connection(struct netconn *conn)
{
	struct netbuf* order_netbuf = NULL;
	u8_t hash;
	err_t err;
	u8_t netbuf_type,contract_type;
	u16_t buflength = 0;	
	u16_t max_length,i;
	char *data,*netbuf;
	while((err = netconn_recv(conn,&order_netbuf)) == ERR_OK)
	{
			netbuf_data(order_netbuf,(void **)&data,&len);//从网络缓冲区读
//			find_order_head(&netbuf_type,&data,&len);
//			if(netbuf_type == NETORDER_CONTRACT) 
//			{
//				netbuf = contract;
//				max_length = SEND_CONTRACT_SIZE;
//			}
//			else max_length = SEND_CONTRACT_SIZE;
//			while(net_head_current_len < max_length)
//			{
//				if(len == 0)
//				{
//						while((err = netconn_recv(conn,&order_netbuf)) == ERR_OK);
//						netbuf_data(order_netbuf,(void **)&data,&len);//从网络缓冲区读
//				}				
//				netbuf[net_head_current_len++] = *data++;
//				len--;
//			}
//			netbuf_type = NETORDER_CONTRACT;
//			if(netbuf_type == NETORDER_CONTRACT)
//			{
//				contract_type = contract[CONTRACT_TYPE_OFFSET];
//				DEBUG_PRINT("contract_type = %d\r\n",contract_type);
//				switch(contract_type)
//				{
//					case CONTRACT_DOCUMENT:contract_response(conn,contract_type+1,0);	break;
//					case CONTRACT_SIGN:  break;
//					case CONTRACT_ESCAPE:  break;
//					default: ;
//				}
//			}
//			begin_new_contract();
		for(i = 0;i < len ;i++)		DEBUG_PRINT("%x ",data[i]);
	}
}
	

void contract_response(struct netconn *conn,contract_type type,u32_t preservation)
{
	char sent_data[SEND_CONTRACT_SIZE] = {0};
	err_t err;
	u16 i = 0;
#ifdef REMOTE	
	Pack_Contract_Message(sent_data,type,Get_Printer_ID(),Get_Current_Unix_Time(),Get_Printer_Speed(),Get_Printer_Status(),preservation);
	DEBUG_PRINT("type = %d,mcu_id = %d,time = %d,speed = %d,status = %d\r\n",type,(u16_t)Get_Printer_ID(),(u16_t)Get_Current_Unix_Time(),Get_Printer_Speed(),Get_Printer_Status());
#endif
	while(0 != (err = netconn_write(conn, sent_data, SEND_CONTRACT_SIZE, NETCONN_COPY))){
		if(ERR_IS_FATAL(err))//致命错误，表示没有连接
			break;
		
		//当网络写入错误时，需要等待一段时间后继续写入该数据包，否则无法反馈给服务器
		OSTimeDlyHMSM(0,0,++i,0);
		DEBUG_PRINT("\n\n\nNETCONN WRITE ERR_T IS %d\n\n\n", err);
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
//	DEBUG_PRINT("Order req.\n");
	
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
	DEBUG_PRINT("\nDHCP can be choosed !!!");
#else
//  IP4_ADDR(&localhost_ip, 192,168,1,135);
//  IP4_ADDR(&localhost_netmask, 255, 255, 255, 0);
//  IP4_ADDR(&localhost_gw, 192, 168, 1, 1);
//JockJo家中测试
  IP4_ADDR(&localhost_ip, 192,168,0,135);
  IP4_ADDR(&localhost_netmask, 255, 255, 255, 0);
  IP4_ADDR(&localhost_gw, 192, 168, 0, 1);	
	DEBUG_PRINT("\nDHCP is not  be choosed !!!\n");	
#endif

	netif_add(&DM9161_netif, &localhost_ip, &localhost_netmask, &localhost_gw, NULL, &ethernetif_init, &tcpip_input);
  netif_set_default(&DM9161_netif);
	
#if LWIP_DHCP
		while(  (err_start = dhcp_start(&DM9161_netif)) != ERR_OK)
		{
				DEBUG_PRINT("\nDHCP start failed!");
				//此处应该有不成功的处理函数为佳
		}
		while(DM9161_netif.ip_addr.addr == 0)
		{
			//等待一会，让IP地址存放近DM9161
				OSTimeDlyHMSM(0, 0, 1, 0);
				DEBUG_PRINT("\nDM9161 netif's  is  null!");
		}
		localhost_ip.addr = DM9161_netif.ip_addr.addr;	
		localhost_netmask.addr = DM9161_netif.netmask.addr;
		localhost_gw.addr = DM9161_netif.gw.addr;
		str_ipaddr = ipaddr_ntoa(&localhost_ip);
#endif
		
	str_ipaddr = ipaddr_ntoa(&localhost_ip);
	if(str_ipaddr != (void*)0)
	{
		DEBUG_PRINT("\nstr_ipaddr is:");		
		DEBUG_PRINT(str_ipaddr);
	}
	else
	{
		DEBUG_PRINT("\nstr_ipaddr is null");		
	}
	
//#ifdef LWIP_IGMP
//	while(  (err_start = igmp_start(&DM9161_netif)) != ERR_OK)
//	{
//			DEBUG_PRINT("igmp start failed!");
//				//此处应该有不成功的处理函数为佳
//	}
//#endif	//LWIP_IGMP
	
/*  When the netif is fully configured this function must be called.*/
  netif_set_up(&DM9161_netif);
	OSSemPost(LWIP_Init_Sem);			//释放成信号
}


	