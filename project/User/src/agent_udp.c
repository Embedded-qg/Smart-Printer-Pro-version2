/***************************************************************************************
 *	FileName					: agent_udp.c
 *	CopyRight					: JockJo
 *	ModuleName				: agent_udp	
 *
 *	CPU							:	
 *	RTOS						:
 *
 *	Create Data					: 2017-8-19	
 *	Author/Corportation			:	JockJo
 *
 *	Abstract Description		:	agent_udp
 *
 *--------------------------------Revision History--------------------------------------
 *	No	version		Data			Revised By			Item			Description
 *	1		1.0     2017-9-5    JockJo 
 ***************************************************************************************/


/**************************************************************
*	Debug switch Section
**************************************************************/


/**************************************************************
*	Include File Section
**************************************************************/
#include "agent_udp.h"

/**************************************************************
*	Macro Define Section
**************************************************************/
#ifndef UDP_PORT
#define UDP_PORT
#define UDP_CLIENT_PORT 8123
#define UDP_SERVER_PORT 65000
#endif	//UDP_PORT

#ifndef MULTICASTIP
#define MULTICASTIP
#define GLOBAL 1
#define LOCAL 0
#endif //MULTICASTIP

/**************************************************************
*	Struct Define Section
**************************************************************/


/**************************************************************
*	Prototype Declare Section
**************************************************************/

/**************************************************************
*	Global Variable Declare Section
**************************************************************/
struct netconn *agent_udp_client_netconn = (void*)0;		//���ؿͻ��ˣ�������peer��������
struct netconn *agent_udp_server_netconn = (void*)0;		//���ط����������ڼ���peer����
const static u8_t UDPASKDATA_FREE[] = "Who is free?";
const static u8_t UDPASKDATA_WISF[] = "WISF";
const static u8_t UDPREPLYDATA_IAMF[] = "IAMF";
struct netbuf *buf_send = (void*)0;
struct pbuf *pbuf_send = (void*)0;
struct netbuf *buf_receive = (void*)0;
struct ip_addr multicast_ip;	
extern struct ip_addr localhost_ip;
struct ip_addr peer_ip;
/**************************************************************
*	File Static Variable Define Section
**************************************************************/


/**************************************************************
*	Function Define Section
**************************************************************/



/**
 *  @name	    reply_to_peer
 *	@description   UDPӦ�𣬷��ͱ�����IP���൱��UDP������
 *	@param			none
 *	@return		  none
 *  @notice
 */
void reply_to_peer()
{
 	struct ip_addr peer_ip;
	struct netbuf *receive_buf = (void*)0;
	receive_buf = netbuf_new();
	netconn_recv(agent_udp_server_netconn, &receive_buf);
	/*�˴���Ҫһ���������򣬵ó�Ŀ���ַ*/
	add_sned_data(buf_send, (u8_t *)UDPREPLYDATA_IAMF);
	netconn_sendto(agent_udp_server_netconn, buf_send, &peer_ip, UDP_SERVER_PORT);			//���е���
}
 
/**
 *  @name	    broadcast_to_localLAN
 *	@description   UDP�ಥ, Ѱ�ҷ���Ҫ�������IP���൱��UDP�ͻ���
 *	@param			none
 *	@return		  none
 *  @notice
 */
void  multicast_to_localLAN(void)
{
#ifdef DEBUG_PRINT_ON
		static char *str_ipaddr = (void*)0;
		str_ipaddr = ipaddr_ntoa(&multicast_ip);
		if(str_ipaddr != (void*)0)
		{
			DEBUG_PRINT("\nstr_ipaddr is:");		
			DEBUG_PRINT(str_ipaddr);
			str_ipaddr = ipaddr_ntoa(&localhost_ip);
			DEBUG_PRINT("\nstr_ipaddr is:");		
			DEBUG_PRINT(str_ipaddr);
		}
#endif //	DEBUG_PRINT_ON
		netconn_sendto(agent_udp_server_netconn, buf_send, &multicast_ip, UDP_SERVER_PORT);			//���жಥ
		
//		udp_sendto(agent_udp_server_netconn->pcb.udp, pbuf_send, &multicast_ip, UDP_SERVER_PORT);
}

/**
 *  @name	    init_UDP
 *	@description   ��ʼ��UDPʹ�õ���bufs
 *	@param			none
 *	@return		  none
 *  @notice
 */
void init_UDP(void)
{
	create_buf_pbuf(buf_send, pbuf_send, sizeof(UDPASKDATA_WISF)+20);
  add_sned_data(buf_send, (u8_t *)UDPASKDATA_WISF);
	set_ipaddr();
	init_client_server();
}

/**
 *  @name	    create_buf_pbuf
 *	@description   ����buf��pbuf
 *	@param			struct netbuf *newbuf, struct pbuf *newpbuf
 *	@return		  none
 *  @notice
 */
void create_buf_pbuf(struct netbuf *newbuf, struct pbuf *newpbuf, int sizeOfNewbuf)
{	
	newbuf = netbuf_new();
	newpbuf = pbuf_alloc(PBUF_RAW, sizeOfNewbuf, PBUF_RAM);
	newbuf->p = newpbuf;
}

void add_localhostIP_to_sendData()
{
 	static char *str_ipaddr = (void*)0;
	static struct pbuf *str_ipaddr_pbuf = (void*)0;		
	str_ipaddr_pbuf = pbuf_alloc(PBUF_RAW, 20, PBUF_RAM);
	str_ipaddr = ipaddr_ntoa(&localhost_ip);
	str_ipaddr_pbuf->payload = (void*)str_ipaddr;;
	pbuf_chain(pbuf_send, str_ipaddr_pbuf);	
}
/**
 *  @name	    add_sned_data
 *	@description   ��buf����Ӵ�������Ϣ
 *	@param			struct netbuf *buf, u8_t *data
 *	@return		  none
 *  @notice
 */
void add_sned_data(struct netbuf *buf, u8_t *data)
{
	buf->p->payload = (void *)data;
}

/**
 *  @name	    set_ipaddr
 *	@description   ���öಥ��ַ
 *	@param			none
 *	@return		  none
 *  @notice
 */
static void set_ipaddr()
{
	#if GLOBAL	

//	IP4_ADDR(&multicast_ip, 230,0,0,11);
	IP4_ADDR(&multicast_ip, 255,255,255,255);	
	#elif LOCAL
	IP4_ADDR(&multicast_ip, 224,0,0,0);
	#else
	IP4_ADDR(&multicast_ip, 239,255,255,255);		//private
	#endif
}

void UDP_Receive(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	struct ip_addr destAddr = *addr;
	struct netbuf *buf_receive = (void*)0;
	struct pbuf *p_temp = pbuf_alloc(PBUF_TRANSPORT, 20, PBUF_RAM);

	buf_receive = netbuf_new();
	memcpy(p_temp->payload, "Test", 5);
	buf_receive->p->payload = p_temp;
	DEBUG_PRINT("got udp multicast");
	if(p_temp != NULL)
	{
//		netconn_sendto(agent_udp_server_netconn, buf_receive, &destAddr, port);
//		netconn_sendto(agent_udp_server_netconn, buf_receive, &multicast_ip, UDP_SERVER_PORT);		
//		udp_sendto(upcb, p_temp, &destAddr, port);
		udp_sendto(upcb, p_temp, &multicast_ip, UDP_SERVER_PORT);
		pbuf_free(p_temp);
		netbuf_delete(buf_receive);
	}
	

}
/**
 *  @name	    init_client_server
 *	@description   ��ʼ��UDP�ͻ��˺ͷ��������󶨶˿ں͵�ַ
 *	@param			none
 *	@return		  none
 *  @notice
 */
void init_client_server(void)
{
	static char *str_ipaddr = (void*)0;

	if( ((agent_udp_client_netconn = netconn_new(NETCONN_UDP)) != NULL) &&
			((agent_udp_server_netconn = netconn_new(NETCONN_UDP)) != NULL)  )
	{
		DEBUG_PRINT("\nUDP netconn build.");
	}
	else
	{
		DEBUG_PRINT("\nFail to build netconn.");
		return ;
	}

//	netconn_set_recvtimeout(agent_udp_client_netconn,10000);//���ý�����ʱʱ�� 		 
//	netconn_set_recvtimeout(agent_udp_server_netconn,10000);//���ý�����ʱʱ�� 		 	

#if 0
	udp_bind(agent_udp_client_netconn->pcb.udp, &localhost_ip, UDP_CLIENT_PORT);
	udp_bind(agent_udp_server_netconn->pcb.udp, &localhost_ip, UDP_SERVER_PORT);	
#else
	netconn_bind(agent_udp_client_netconn, &localhost_ip, UDP_CLIENT_PORT);		//�󶨵����������е�ַ���ͻ��˶˿�
	netconn_bind(agent_udp_server_netconn, &localhost_ip, UDP_SERVER_PORT);		//�󶨵��������е�ַ���������˿�
#endif //

//#ifdef LWIP_IGMP
////	if( netconn_connect(agent_udp_server_netconn, &multicast_ip, UDP_SERVER_PORT) != ERR_OK)
////	{
////			DEBUG_PRINT("\nconnect multicast failed.");
////	}
////	else
////	{
////			DEBUG_PRINT("\nconnect multicast successfully.");
////	}

//	//�������˼���ಥ��,������Ϣ
//#if 0
//	netconn_join_leave_group(agent_udp_server_netconn, &multicast_ip, &localhost_ip, NETCONN_JOIN);
//#else
//	igmp_joingroup(&localhost_ip, &multicast_ip);
//#endif	//

//#endif 	//LWIP_IGMP

#if 1
  udp_recv(agent_udp_server_netconn->pcb.udp, UDP_Receive, NULL);
#else
while(1)
	{
		netconn_recv(agent_udp_server_netconn, &buf_receive);
		if(buf_receive != NULL)
		{
			DEBUG_PRINT("\nreceive data!");
			str_ipaddr = ipaddr_ntoa(&multicast_ip);
			IP4_ADDR(&peer_ip, 192,168,1,116);		//private

			if(str_ipaddr != (void*)0)
			{
				DEBUG_PRINT("\nmulticast_ipaddr is:");		
				DEBUG_PRINT(str_ipaddr);
				str_ipaddr = ipaddr_ntoa(&peer_ip);
				DEBUG_PRINT("\npeer_ipaddr is:");		
				DEBUG_PRINT(str_ipaddr);			
			}
			
			netconn_sendto(agent_udp_server_netconn, buf_receive, &peer_ip, UDP_CLIENT_PORT);
			if(netconn_sendto(agent_udp_server_netconn, buf_receive, &multicast_ip, UDP_SERVER_PORT) !=  ERR_OK)
			{
				DEBUG_PRINT("\nMULTICAST FAIL.");
			}				
			else
			{
				DEBUG_PRINT("\nMULTICAST SUCCESS.");
			}
			
			netbuf_delete(buf_receive);
		}
	}
#endif	//
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
void assert_failed(u8* file, u32 line)
{
          /* User can add his own implementation to report the file name and line number,
             ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
        
          while (1)
          {}
}
#endif

