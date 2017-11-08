#include "netconf.h"

#define ORDER_NUM_MAX 10

struct netif DM9161_netif;
struct netconn *order_netconn;	//ȫ��TCP����

extern OS_EVENT *Print_Sem;
extern batch_info batch_info_table[];	//���α�
extern OS_EVENT *Batch_Rec_Sem;			//���һ�����ζ�ȡ�Ķ�ֵ�ź���

struct ip_addr localhost_ip;
struct ip_addr localhost_netmask;
struct ip_addr localhost_gw;

//#define TCPSEVER
#define TCPCLIENT
#define REMOTE  1



/****************************************************************************************
*@Name............: put_in_buf
*@Description.....: �����ݴ洢�����ػ�����
*@Parameters......: data		:�洢���ݵ�ָ��
*					len			:���ݳ���
*					urg			:�Ӽ���־
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
		if(BUF_OK != (buf_err = Write_Buf(buf, data, len))){  //���ƶ������ݵ�������
			DEBUG_PRINT("buf %d write error, err is %d, len is %d.\n", urg, buf_err, len);
			OSSemPost(Print_Queue_Sem);//�ͷ�
			OSTimeDlyHMSM(0, 0, 1, 0);
		}else{
			return;
		}
	}
}

static u16_t net_head_current_len = 0;//��ǰ��ȡ���ĺ�ͬ�ĳ���
static u16_t contract_total_len = 0;//�����ܹ�����
static u16_t contract_number = 0;	//���κ�
static u16_t last_contract_number = 0;
static char contract[33] = {0};
static u16_t len = 0;	//�����绺������ȡ�������ݵĳ���
static u8_t flag = 0;	//1��ʾ�Ѿ���ȡ������ͷ������û��
static u16_t count = 0;
static u16_t leave_len = 0;

/**
 * @brief	�����µ�����ͷ�ĳ�ʼ��
 */
static void begin_new_contract(void)
{
	contract[0] = '\0';
	net_head_current_len = 0;//�µĺ�ͬ
	contract_total_len = 0;
	flag = 0;
	count = 0;
	last_contract_number = contract_number;
}

/**
*@brief ���ձ��Ĳ�����
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
			netbuf_data(order_netbuf,(void **)&data,&len);//�����绺������
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
//						netbuf_data(order_netbuf,(void **)&data,&len);//�����绺������
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
		if(ERR_IS_FATAL(err))//�������󣬱�ʾû������
			break;
		
		//������д�����ʱ����Ҫ�ȴ�һ��ʱ������д������ݰ��������޷�������������
		OSTimeDlyHMSM(0,0,++i,0);
		DEBUG_PRINT("\n\n\nNETCONN WRITE ERR_T IS %d\n\n\n", err);
	}	

}
/****************************************************************************************
*@Name............: write_connection
*@Description.....: �������ݱ�
*@Parameters......: conn		:����
*					type		:��������
*					symbol		:��־λ
*					preservation:�����ֶΣ������У���16λΪ������ţ�������ʱ�ҵ�16λΪ���������
*@Return values...: void
*****************************************************************************************/
void write_connection(struct netconn *conn, req_type type, u8_t symbol, u32_t preservation)
{
	char sent_data[SEND_DATA_SIZE] = {0};	//״̬���ĺ������Ķ��ǹ̶�20�ֽ�
	err_t err;
	int i = 0;
#ifdef REMOTE
	//�ȴ���������Ǳ���
	if(type == order_req){
		Pack_Req_Or_Status_Message(sent_data, ORDER_REQ, symbol, Get_Printer_ID(), 
									Get_Current_Unix_Time(), preservation);
	}
	else if(type == batch_status){//��ʱ��preservation��16λΪ���κ�
		Pack_Req_Or_Status_Message(sent_data, BATCH_STATUS, symbol, Get_Printer_ID(), 
									Get_Batch_Unix_Time((u16_t)preservation), preservation << 16);
	}
	else if(type == printer_status){//��ʱ��preservation�����ذ��ӡ��Ԫ��Ż�Ϊ0
		Pack_Req_Or_Status_Message(sent_data, PRINTER_STATUS, symbol, Get_Printer_ID(), 
									Get_Current_Unix_Time(), preservation);
	}
	else if(type == order_status){//��ʱ��preservation�ĸ�16λΪ���κţ���16λΪ���������
		Pack_Req_Or_Status_Message(sent_data, ORDER_STATUS, symbol, Get_Printer_ID(), 
									Get_Batch_Unix_Time((u16_t)(preservation >> 16)), preservation);
	}
	else if(type == first_req){//�����һ�ν������ӣ��������ذ�id
		Pack_Req_Or_Status_Message(sent_data, FIRST_REQ, symbol, Get_Printer_ID(), 
									Get_Current_Unix_Time(), preservation);	
	}

#endif
	
	while(0 != (err = netconn_write(conn, sent_data, SEND_DATA_SIZE, NETCONN_COPY))){
		if(ERR_IS_FATAL(err))//�������󣬱�ʾû������
			break;
		
		//������д�����ʱ����Ҫ�ȴ�һ��ʱ������д������ݰ��������޷�������������
		OSTimeDlyHMSM(0,0,++i,0);
		DEBUG_PRINT("\n\n\nNETCONN WRITE ERR_T IS %d\n\n\n", err);
		
		if(type != order_req){//����������������ͣ�������������޷��´ﶩ��
			if(i > 3) break;
		}else if(i > 10) break;//���ȴ���κ����������
	}
		
}

/**
 * @brief	������Զ�̷�����
 */
void con_to_server(void)
{
	struct ip_addr server_ip;
	extern struct netconn *order_netconn;	//ȫ��TCP����
	extern OS_EVENT *Recon_To_Server_Sem;
	if((order_netconn = netconn_new(NETCONN_TCP)) != NULL){
		DEBUG_PRINT("Connection build.\n");
	}else{
		DEBUG_PRINT("Fail to build connection.\n");
		OSSemPost(Recon_To_Server_Sem);	
	}
	
	netconn_set_recvtimeout(order_netconn,10000);//���ý�����ʱʱ�� 
	
#ifdef REMOTE//���÷�����ip��ַ
	IP4_ADDR(&server_ip,10,21,48,11);//�Ʒ�����ѧУ
//		IP4_ADDR(&server_ip,123,207,228,117);		//�Ʒ�����_����
//	IP4_ADDR(&server_ip,192,168,1,116);		//JockJo���Ի�
//	IP4_ADDR(&server_ip, 192,168,1,119);	//�����Ҳ��Ի�
//	IP4_ADDR(&server_ip,192,168,1,110);  //�÷ʳ���Ե�IP
	netconn_connect(order_netconn,&server_ip,8086);
#else	
	IP4_ADDR(&server_ip,192,168,1,116);
	netconn_connect(order_netconn,&server_ip,8086);
#endif	
	
	write_connection(order_netconn, first_req, REQ_LINK_OK, 0);//�����������������ذ�ID
	OSTimeDlyHMSM(0,0,1,0);
//	write_connection(order_netconn, order_req, ORDER_REQUEST, 0);//���󶩵�			�˴������޸�Ϊ��Ҫ���������
//	DEBUG_PRINT("Order req.\n");
	
	//����β���
#ifdef APP_DEBUG
	//write_connection(order_netconn, first_req, REQ_LINK_OK, 0);//����������
	//write_connection(order_netconn, first_req, REQ_LINK_OK, 0);//����������
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


#if LWIP_DHCP//��̬ip����
  localhost_ip.addr = 0;
  localhost_netmask.addr = 0;
  localhost_gw.addr = 0;
	DEBUG_PRINT("\nDHCP can be choosed !!!");
#else
//  IP4_ADDR(&localhost_ip, 192,168,1,135);
//  IP4_ADDR(&localhost_netmask, 255, 255, 255, 0);
//  IP4_ADDR(&localhost_gw, 192, 168, 1, 1);
//JockJo���в���
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
				//�˴�Ӧ���в��ɹ��Ĵ�����Ϊ��
		}
		while(DM9161_netif.ip_addr.addr == 0)
		{
			//�ȴ�һ�ᣬ��IP��ַ��Ž�DM9161
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
//				//�˴�Ӧ���в��ɹ��Ĵ�����Ϊ��
//	}
//#endif	//LWIP_IGMP
	
/*  When the netif is fully configured this function must be called.*/
  netif_set_up(&DM9161_netif);
	OSSemPost(LWIP_Init_Sem);			//�ͷų��ź�
}


	