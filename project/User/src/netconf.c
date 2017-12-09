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
			NET_DEBUG_PRINT("buf %d write error, err is %d, len is %d.\r\n", urg, buf_err, len);
			OSSemPost(Print_Queue_Sem);//�ͷ�
			OSTimeDlyHMSM(0, 0, 1, 0);
		}else{
			return;
		}
	}
}

contract_info contract_information;
static u16_t contract_total_len = 0;//�����ܹ�����
static u16_t contract_number = 0;	//��ͬ��
static u16_t batch_number = 0; //���κ�
static u16_t last_bacth_number = 0;
static u32_t contract_partner = 0;
static char contract[29] = {0};
static char batch[21] = {0};
static u16_t len = 0;	//�����绺������ȡ�������ݵĳ���
u8_t netbuf_type = 0;
u8_t batch_flag = 0;//���α��
u8_t batch_table_hash = 0;

/****************************************************************************************
*@Name............: read_message_from_netbuf
*@Description.....: �����绺�����ж�ȡ������������
*@Parameters......: 
										netbuf----------�������ݻ�����
										data------------�������绺����
										netbuf----------���ݴ�Ż�����
*										len���������绺��������
*@Return 					��1 ----�ɹ���ȡ
										0 ----�����ȡ
*****************************************************************************************/
u8_t read_message_from_netbuf(char **netbuf,char ** data,u8_t netbuf_type,u16_t *len)
{
	u16_t net_head_current_len = 0,sub_len = *len;//��Ҫ��ȡ�ı��ĵ��ܳ��ȣ���ǰ��ȡ���ı��ĵĳ���;
	static u16_t max_length = 0;
	char *sub_data = *data;
	u8_t read_symbol = 0;//�Ƿ�ɹ���ȡ������Ϣ
	NET_DEBUG_PRINT("��ȡ��������\r\n");
	if(netbuf_type == NETORDER_TYPE_CONTRACT) 	//��ȡ��ͬ������
	{
		NET_DEBUG_PRINT("��ȡ��ͬ������\r\n");
		if(max_length == 0) max_length = MAX_CONTRACT_HEAD_LENGTH;
		while(sub_len > 0 && net_head_current_len < max_length)
		{
			contract[net_head_current_len++] = *sub_data++;
			sub_len--;		
		}		
		if(net_head_current_len == max_length)     //�жϺ�ͬ����β���
			if(*(contract + CONTRACT_TAIL_OFFSET) == '\xfb' && *(contract + CONTRACT_TAIL_OFFSET + 1) == '\xbf') 
				read_symbol = 1;
		*netbuf = contract;
		max_length = 0;
	}
	else if(netbuf_type == NETORDER_TYPE_BATCH)//��ȡ���α���
	{
		NET_DEBUG_PRINT("��ȡ���α���\r\n");
		if(max_length == 0) max_length = MAX_BATCH_HEAD_LENGTH;
		while(sub_len > 0 && net_head_current_len < max_length)
		{
			batch[net_head_current_len++] = *sub_data++;
			sub_len--;		
		}
		if(net_head_current_len == max_length)
				if(*(batch + BATCH_TAIL_OFFSET) == '\x55' && *(batch + BATCH_TAIL_OFFSET + 1) == '\xaa') read_symbol = 1;
		max_length = 0;
	}
	else if(netbuf_type == NETORDER_TYPE_ORDER)//����Ƕ������ģ������Ѿ��ɹ����������α��ģ����ȡ��������
	{
		if(batch_flag)
		{
			NET_DEBUG_PRINT("��ȡ�������ݱ���\r\n");
			if(max_length == 0) max_length = batch_info_table[batch_table_hash].batch_length - MAX_BATCH_HEAD_LENGTH;//��ȡ�������ݳ���
			NET_DEBUG_PRINT("��Ҫ��ȡ�������ݳ���Ϊ%d\r\n",max_length);
			if(max_length >= sub_len) //���������ݲ���ʱ�������������ݶ���
			{
				put_in_buf((u8_t *)sub_data,sub_len,batch_info_table[batch_table_hash].preservation);
				max_length -= sub_len;
				sub_data += sub_len;
				sub_len = 0;
			}
			if(max_length > 0 && sub_len > 0)//�����������㹻ʱֱ��һ���Խ��������ݶ���
			{
				put_in_buf((u8_t *)sub_data,max_length,batch_info_table[batch_table_hash].preservation);
				sub_len -= max_length;
				sub_data += max_length;
				max_length = 0;
			}
			if(!max_length) read_symbol = 1;
		}
		else{
			sub_data += sub_len;
			sub_len = 0;
		}
	}
	*data = sub_data;
	*len = sub_len;
	if(read_symbol)
	{
		batch_flag = 0;
		NET_DEBUG_PRINT("��ȡ�������ݳɹ�!!!\r\n");
	}
	else NET_DEBUG_PRINT("��ȡ��������ʧ��!!!\r\n");
	return read_symbol;
}

//�����ͬ������
void deal_with_contract_order(char *contract_buf)
{
		u16_t contract_type;
		extern struct netconn *order_netconn;
		contract_type = contract_buf[CONTRACT_TYPE_OFFSET];		
		if(contract_type == CONTRACT_DOCUMENT)//����
		{
			if(contract_partner)			NET_DEBUG_PRINT("��ǩ����ͬ��������ǩԼ������\r\n");
			else 
			{
				Analyze_Contract_Info_Table(contract_buf);
				contract_response(order_netconn,contract_type+1);
				NET_DEBUG_PRINT("���ذ�״̬�Ϸ��ɹ�!!!\r\n");
			}
		}
		else if(contract_type == CONTRACT_SIGN)//ǩԼ
		{
			if(contract_partner)			NET_DEBUG_PRINT("��ǩ����ͬ��������ǩԼ������\r\n");
			else{
				Analyze_Contract_Info_Table(contract_buf);
				contract_partner = 1;
				contract_response(order_netconn,contract_type+1);
				NET_DEBUG_PRINT("ǩԼ�����Ϸ��ɹ�!!!\r\n");
			}
		}
		else if(contract_type == CONTRACT_ESCAPE)//��Լ
		{
			contract_partner = contract_information.contract_number = 0;
			contract_response(order_netconn,contract_type+1);
			NET_DEBUG_PRINT("�ѽ�Լ\r\n");
		}
}

//�������α���
void deal_with_batch_order(char *batch_buf)
{
	u16_t preservation;

//	ANALYZE_DATA_2B((batch + BATCH_PRESERVATION_OFFSET), preservation);//��ȡ����״̬

//	if(contract_partner == 0 && preservation == 0)
//	{
//		NET_DEBUG_PRINT("�����մ˱��ģ�����\r\n");
//		return ;
//	}
	
	last_bacth_number = batch_number;
	ANALYZE_DATA_2B((batch + BATCH_NUMBER_OFFSET), batch_number);//��ȡ���κ�
	
	NET_DEBUG_PRINT("batch read success ,batch_number is %x %x\r\n", *(batch + BATCH_NUMBER_OFFSET), *(batch + BATCH_NUMBER_OFFSET + 1));
	NET_DEBUG_PRINT("batch read success ,batch_length is %x %x\r\n", *(batch + BATCH_TOTAL_LENGTH_OFFSET), *(batch + BATCH_TOTAL_LENGTH_OFFSET + 1));		
	
	batch_table_hash = get_batch_hash(batch_number);
	Analyze_Batch_Info_Table(batch, batch_number);//���ν��
	
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
		NET_DEBUG_PRINT("�������ݳ���\r\n");
		OSMutexPend(tempBuf->mutex,0,&err);			//������ͨ������
		tempBuf->write = oldWritePtr;
		OSMutexPost(tempBuf->mutex);			//������ͨ������
	}										
	else{//����������ȷ						
		NET_DEBUG_PRINT("���ն����ɹ�!!\r\n");														
		OSSemPost(Print_Queue_Sem);
		OSSemPost(Batch_Rec_Sem); 
		NON_BASE_SEND_STATUS(batch_status, BATCH_ENTER_BUF, batch_number);//��������״̬�����뻺����
		batch_flag  = 0;
	}
}

//������
void deal_with_order(char *netbuf)
{
	if(netbuf_type == NETORDER_TYPE_CONTRACT)
	{
		NET_DEBUG_PRINT("�����ͬ������\r\n");
		deal_with_contract_order(netbuf);//�����ͬ������
	}
	if(netbuf_type == NETORDER_TYPE_BATCH)
	{
		NET_DEBUG_PRINT("�������α���\r\n");
		deal_with_batch_order(netbuf);//�������α���
	}
	if(netbuf_type == NETORDER_TYPE_ORDER)
	{
		NET_DEBUG_PRINT("���������ݱ���\r\n");
		deal_with_order_order();//����������
	}
	netbuf_type = NETORDER_TYPE_NULL; //�����籨��������Ϊ��
}
/**
*@brief ���ձ��Ĳ�����
*/
void receive_connection(struct netconn *conn)
{
	err_t err;
	char *data,*netbuf;
	struct netbuf* order_netbuf = NULL;	
	u8_t read_message_symbol = 0;//���ĳɹ���ȡ���
	while((err = netconn_recv(conn,&order_netbuf)) == ERR_OK)
	{	
		NET_DEBUG_PRINT("����̫����������!!\r\n");
		netbuf_data(order_netbuf,(void **)&data,&len);//�����绺������ȡ����
start:
		NET_DEBUG_PRINT("���ݽ��ճɹ�,����Ϊ%d,netbuf_type = %d\r\n",len,netbuf_type);
		if(netbuf_type == NETORDER_TYPE_NULL) netbuf_type = find_order_head(&data,&len); //�����绺�����ж�ȡ��һ���������࣬������������
		if(netbuf_type != NETORDER_TYPE_NULL) read_message_symbol = read_message_from_netbuf(&netbuf,&data,netbuf_type,&len);//��ȡ�����������ݳ���	
		NET_DEBUG_PRINT("���ݶ�ȡ������绺����ʣ�೤�ȣ�len = %d\r\n",len);
		if(read_message_symbol) deal_with_order(netbuf);//�����ݱ��Ľ��д���
		if(len) goto start; //���ж������ݣ����½��б����ж�
		if(!(netbuf_next(order_netbuf) > 0)) netbuf_delete(order_netbuf);
	}
	OSTimeDlyHMSM(0, 0, 0, 50);
}
	
//��ͬ��Э���ظ�
void contract_response(struct netconn *conn,contract_type type)
{
	char sent_data[MAX_CONTRACT_HEAD_LENGTH] = {0};
	err_t err;
	u16_t i = 0;
#ifdef REMOTE	
	Pack_Contract_Message(sent_data,type,contract_information.contract_number,Get_MCU_ID(),Get_Current_Unix_Time(),Get_MCU_Speed(),Get_MCU_Status());
#endif
//	NET_DEBUG_PRINT("\r\n");
//	for(i = 0;i < MAX_CONTRACT_HEAD_LENGTH;i++)
//		NET_DEBUG_PRINT("%x ",sent_data[i]);
//	NET_DEBUG_PRINT("\r\n");
	while(0 != (err = netconn_write(conn, sent_data, MAX_CONTRACT_HEAD_LENGTH, NETCONN_COPY))){
		if(ERR_IS_FATAL(err))//�������󣬱�ʾû������
			break;		
		//������д�����ʱ����Ҫ�ȴ�һ��ʱ������д������ݰ��������޷�������������
		OSTimeDlyHMSM(0,0,++i,0);
//		NET_DEBUG_PRINT("NETCONN WRITE ERR_T IS %d\r\n", err);
	}	

}

/****************************************************************************************
*@Name............: write_connection
*@Description.....: �������ݱ�
*@Parameters......: conn		:����
*					type		:��������
*					symbol		:��־λ
*					preservation:�����ֶΣ������У���16λΪ������ţ�������ʱ�ҵ�16λΪ���� �����
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
		NET_DEBUG_PRINT("NETCONN WRITE ERR_T IS %d\r\n", err);
		
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
		NET_DEBUG_PRINT("Connection build.\r\n");
	}else{
		NET_DEBUG_PRINT("Fail to build connection.\r\n");
		OSSemPost(Recon_To_Server_Sem);	
	}
	
	netconn_set_recvtimeout(order_netconn,10000);//���ý�����ʱʱ�� 
	
#ifdef REMOTE//���÷�����ip��ַ
	IP4_ADDR(&server_ip,10,21,48,11);//�Ʒ�����ѧУ
//	IP4_ADDR(&server_ip,123,207,228,117);		//�Ʒ�����_����
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
//	NET_DEBUG_PRINT("Order req.\n");
	
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
	NET_DEBUG_PRINT("DHCP can be choosed !!!\r\n");
#else
//  IP4_ADDR(&localhost_ip, 192,168,1,135);
//  IP4_ADDR(&localhost_netmask, 255, 255, 255, 0);
//  IP4_ADDR(&localhost_gw, 192, 168, 1, 1);
//JockJo���в���
  IP4_ADDR(&localhost_ip, 192,168,0,135);
  IP4_ADDR(&localhost_netmask, 255, 255, 255, 0);
  IP4_ADDR(&localhost_gw, 192, 168, 0, 1);	
	NET_DEBUG_PRINT("DHCP is not  be choosed !!!\r\n");	
#endif

	netif_add(&DM9161_netif, &localhost_ip, &localhost_netmask, &localhost_gw, NULL, &ethernetif_init, &tcpip_input);
  netif_set_default(&DM9161_netif);
	
#if LWIP_DHCP
		while(  (err_start = dhcp_start(&DM9161_netif)) != ERR_OK)
		{
				NET_DEBUG_PRINT("DHCP start failed!\r\n");
				//�˴�Ӧ���в��ɹ��Ĵ�����Ϊ��
		}
		while(DM9161_netif.ip_addr.addr == 0)
		{
			//�ȴ�һ�ᣬ��IP��ַ��Ž�DM9161
				OSTimeDlyHMSM(0, 0, 1, 0);
				NET_DEBUG_PRINT("DM9161 netif's  is  null!\r\n");
		}
		localhost_ip.addr = DM9161_netif.ip_addr.addr;	
		localhost_netmask.addr = DM9161_netif.netmask.addr;
		localhost_gw.addr = DM9161_netif.gw.addr;
		str_ipaddr = ipaddr_ntoa(&localhost_ip);
#endif
		
	str_ipaddr = ipaddr_ntoa(&localhost_ip);
	if(str_ipaddr != (void*)0)
	{
		NET_DEBUG_PRINT("str_ipaddr is:");		
		NET_DEBUG_PRINT(str_ipaddr);
		NET_DEBUG_PRINT("\r\n");
	}
	else
	{
		NET_DEBUG_PRINT("str_ipaddr is null\r\n");		
	}
	
//#ifdef LWIP_IGMP
//	while(  (err_start = igmp_start(&DM9161_netif)) != ERR_OK)
//	{
//			NET_DEBUG_PRINT("igmp start failed!");
//				//�˴�Ӧ���в��ɹ��Ĵ�����Ϊ��
//	}
//#endif	//LWIP_IGMP
	
/*  When the netif is fully configured this function must be called.*/
  netif_set_up(&DM9161_netif);
	OSSemPost(LWIP_Init_Sem);			//�ͷų��ź�
}


	