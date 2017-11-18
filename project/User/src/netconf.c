#include "netconf.h"

#define ORDER_NUM_MAX 10

struct netif DM9161_netif;
struct netconn *order_netconn;	//ȫ��TCP����

extern OS_EVENT *Print_Sem;
extern batch_info batch_info_table[];	//���α�
extern OS_EVENT *Batch_Rec_Sem;			//���һ�����ζ�ȡ�Ķ�ֵ�ź���
extern OS_EVENT *Print_Queue_Sem;		//���ݴ����ӡ���е��ź���

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
			printf("buf %d write error, err is %d, len is %d.\n", urg, buf_err, len);
			OSSemPost(Print_Queue_Sem);//�ͷ�
			OSTimeDlyHMSM(0, 0, 1, 0);
		}else{
			return;
		}
	}
}


static u16_t contract_total_len = 0;//�����ܹ�����
static u16_t contract_number = 0;	//��ͬ��
static u16_t batch_number = 0; //���κ�
static u16_t last_bacth_number = 0;
static u32_t contract_partner = 0;
static char contract[33] = {0};
static char batch[21] = {0};
static u16_t len = 0;	//�����绺������ȡ�������ݵĳ���
static u16_t count = 0;
static u16_t leave_len = 0; 
u8_t batch_flag = 0;//���α��
u8_t batch_table_hash = 0;

/****************************************************************************************
*@Name............: read_message_from_netbuf
*@Description.....: �����绺�����ж�ȡ������������
*@Parameters......: order_netbuf----һ�����绺��������
										netbuf----------�������ݻ�����
										data------------�������绺����
										netbuf----------���ݴ�Ż�����
*										len���������绺��������
*@Return 					��1 ----�ɹ���ȡ
										0 ----�����ȡ
*****************************************************************************************/
u8_t read_message_from_netbuf(struct netbuf* order_netbuf,char **netbuf,char ** data,u8_t netbuf_type,u16_t *len)
{
	u16_t max_length = 0,net_head_current_len = 0,sub_len = *len;//��Ҫ��ȡ�ı��ĵ��ܳ��ȣ���ǰ��ȡ���ı��ĵĳ���;
	char *sub_data = *data;
	u8_t read_symbol = 0;//�Ƿ�ɹ���ȡ������Ϣ
	if(netbuf_type == NETORDER_CONTRACT) 	//��ȡ��ͬ������
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
	else if(netbuf_type == NETOREDER_BATCH)//��ȡ���α���
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
	else if(netbuf_type == NETOREDER_ORDER)//��ȡ��������
	{
		max_length = batch_info_table[batch_table_hash].batch_length - MAX_BATCH_HEAD_LENGTH;
		while(max_length >= sub_len)
		{
			put_in_buf((u8_t *)sub_data,sub_len,batch_info_table[batch_table_hash].preservation);
			max_length -= sub_len;
			if(netbuf_next(order_netbuf) > 0) netbuf_data(order_netbuf, (void **)&sub_data, &sub_len);//�����绺������ȡ
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

//�����ͬ������
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

//�������α���
void deal_with_batch_order(char *batch_buf)
{
	ANALYZE_DATA_2B((batch + BATCH_NUMBER_OFFSET), batch_number);//��ȡ���κ�

	printf("batch read success ,batch_number is %x %x\n", *(batch + BATCH_NUMBER_OFFSET), *(batch + BATCH_NUMBER_OFFSET + 1));
	printf("batch read success ,batch_length is %x %x\n", *(batch + BATCH_TOTAL_LENGTH_OFFSET), *(batch + BATCH_TOTAL_LENGTH_OFFSET + 1));	
	Analyze_Batch_Info_Table(batch, batch_number);//���ν��
	
	batch_table_hash = get_batch_hash(batch_number);
	
	if(batch_number != last_bacth_number) batch_flag = 1;
}

void deal_with_order_order(void)
{
	SqQueue * tempBuf = 0;
	u32_t oldWritePtr = 0;
	tempBuf = (batch_info_table[batch_table_hash].preservation)?(&urgent_buf):(&queue_buf);
	oldWritePtr = ((tempBuf->write + tempBuf->MAX - (batch_info_table[batch_table_hash].batch_length - MAX_BATCH_HEAD_LENGTH) )%tempBuf->MAX );
	if(checkBufData(tempBuf,oldWritePtr) == 1){	//�������ݴ���				
		INT8U err = 0;	
		printf("�������ݳ���\r\n");
		OSMutexPend(tempBuf->mutex,0,&err);			//������ͨ������
		tempBuf->write = oldWritePtr;
		OSMutexPost(tempBuf->mutex);			//������ͨ������
	}										
	else{//����������ȷ						
		printf("���ն����ɹ�!!\r\n");														
		OSSemPost(Print_Queue_Sem);
		OSSemPost(Batch_Rec_Sem); 
		NON_BASE_SEND_STATUS(batch_status, BATCH_ENTER_BUF, batch_number);//��������״̬�����뻺����
		batch_flag  = 0;
	}
}

//������
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
*@brief ���ձ��Ĳ�����
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
		netbuf_data(order_netbuf,(void **)&data,&len);//�����绺������
//		for(i = 0;i < len;i++)
//		{
//			printf("%x ",data[i]);
//		}
start:
		netbuf_type = find_order_head(&data,&len); //�����绺�����ж�ȡ��һ���������࣬������������
		printf("netbuf_type = %d\r\n",netbuf_type);	
		read_message_symbol = read_message_from_netbuf(order_netbuf,&netbuf,&data,netbuf_type,&len);//��ȡ�����������ݳ���
		printf("read_message_symbol = %d\r\n",read_message_symbol);		
		printf("���绺����ʣ�೤�ȣ�len = %d\r\n",len);	
		deal_with_order(netbuf,netbuf_type);//�����ݱ��Ľ��д���
		if(len > 0) goto start; //���ж������ݣ����½��б����ж�
		if(!(netbuf_next(order_netbuf) > 0)) netbuf_delete(order_netbuf);
	}
	OSTimeDlyHMSM(0, 0, 0, 50);
}
	
//��ͬ��Э���ظ�
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
		if(ERR_IS_FATAL(err))//�������󣬱�ʾû������
			break;
		
		//������д�����ʱ����Ҫ�ȴ�һ��ʱ������д������ݰ��������޷�������������
		OSTimeDlyHMSM(0,0,++i,0);
		printf("\n\n\nNETCONN WRITE ERR_T IS %d\n\n\n", err);
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
		Pack_Req_Or_Status_Message(sent_data, ORDER_REQ, symbol, Get_MCU_ID(), 
									Get_Current_Unix_Time(), preservation);
	}
	else if(type == batch_status){//��ʱ��preservation��16λΪ���κ�
		Pack_Req_Or_Status_Message(sent_data, BATCH_STATUS, symbol, Get_MCU_ID(), 
									Get_Batch_Unix_Time((u16_t)preservation), preservation << 16);
	}
	else if(type == printer_status){//��ʱ��preservation�����ذ��ӡ��Ԫ��Ż�Ϊ0
		Pack_Req_Or_Status_Message(sent_data, PRINTER_STATUS, symbol, Get_MCU_ID(), 
									Get_Current_Unix_Time(), preservation);
	}
	else if(type == order_status){//��ʱ��preservation�ĸ�16λΪ���κţ���16λΪ���������
		Pack_Req_Or_Status_Message(sent_data, ORDER_STATUS, symbol, Get_MCU_ID(), 
									Get_Batch_Unix_Time((u16_t)(preservation >> 16)), preservation);
	}
	else if(type == first_req){//�����һ�ν������ӣ��������ذ�id
		Pack_Req_Or_Status_Message(sent_data, FIRST_REQ, symbol, Get_MCU_ID(), 
									Get_Current_Unix_Time(), preservation);	
	}

#endif
	
	while(0 != (err = netconn_write(conn, sent_data, SEND_DATA_SIZE, NETCONN_COPY))){
		if(ERR_IS_FATAL(err))//�������󣬱�ʾû������
			break;
		
		//������д�����ʱ����Ҫ�ȴ�һ��ʱ������д������ݰ��������޷�������������
		OSTimeDlyHMSM(0,0,++i,0);
		printf("\n\n\nNETCONN WRITE ERR_T IS %d\n\n\n", err);
		
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
		printf("Connection build.\n");
	}else{
		printf("Fail to build connection.\n");
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
//	printf("Order req.\n");
	
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
	printf("\nDHCP can be choosed !!!");
#else
//  IP4_ADDR(&localhost_ip, 192,168,1,135);
//  IP4_ADDR(&localhost_netmask, 255, 255, 255, 0);
//  IP4_ADDR(&localhost_gw, 192, 168, 1, 1);
//JockJo���в���
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
				//�˴�Ӧ���в��ɹ��Ĵ�����Ϊ��
		}
		while(DM9161_netif.ip_addr.addr == 0)
		{
			//�ȴ�һ�ᣬ��IP��ַ��Ž�DM9161
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
//				//�˴�Ӧ���в��ɹ��Ĵ�����Ϊ��
//	}
//#endif	//LWIP_IGMP
	
/*  When the netif is fully configured this function must be called.*/
  netif_set_up(&DM9161_netif);
	OSSemPost(LWIP_Init_Sem);			//�ͷų��ź�
}


	