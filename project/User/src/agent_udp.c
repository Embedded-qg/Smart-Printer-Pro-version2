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
#define UDP_CLIENT_PORT 8002
#define UDP_SERVER_PORT 8001
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
struct netbuf *buf_send = (void*)0;
struct pbuf *pbuf_send = (void*)0;
struct ip_addr multicast_ip;	
struct ip_addr localhost_ip;
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
 	/*�˴���Ҫ���*/
	netconn_sendto(agent_udp_client_netconn, buf_send, &multicast_ip, UDP_SERVER_PORT);			//���жಥ
}

/**
 *  @name	    init_UDP_bufs
 *	@description   ��ʼ��UDPʹ�õ���bufs
 *	@param			none
 *	@return		  none
 *  @notice
 */
void init_UDP_bufs(void)
{
	create_buf_pbuf(buf_send, pbuf_send, sizeof(UDPASKDATA_WISF));
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

/**
 *  @name	    add_sned_data
 *	@description   ��buf�����Ӵ�������Ϣ
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
 *	@description   ����IP��ַ������ಥ��ַ��������ַ
 *	@param			none
 *	@return		  none
 *  @notice
 */
static void set_ipaddr()
{
	#if GLOBAL	
	IP4_ADDR(&multicast_ip, 238,255,255,255);
	#elif LOCAL
	IP4_ADDR(&multicast_ip, 224,0,0,0);
	#else
	IP4_ADDR(&multicast_ip, 239,255,255,255);		//private
	#endif
	
	IP4_ADDR(&localhost_ip, 192,168,1,134);
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
	if( ((agent_udp_client_netconn = netconn_new(NETCONN_UDP)) != NULL) &&
			((agent_udp_server_netconn = netconn_new(NETCONN_UDP)) != NULL)  )
	{
		DEBUG_PRINT("UDP netconn build.\n");
	}
	else
	{
		DEBUG_PRINT("Fail to build netconn.\n");
		return ;
	}
	netconn_set_recvtimeout(agent_udp_client_netconn,10000);//���ý�����ʱʱ�� 		 
	netconn_set_recvtimeout(agent_udp_server_netconn,10000);//���ý�����ʱʱ�� 		 	
	
	netconn_bind(agent_udp_client_netconn, &multicast_ip, UDP_CLIENT_PORT);		//�󶨵���ͻ��˶˿ڣ��ಥ��ַ
	netconn_join_leave_group(agent_udp_client_netconn, &multicast_ip, &localhost_ip, NETCONN_JOIN);
	
	netconn_bind(agent_udp_server_netconn, &multicast_ip, UDP_SERVER_PORT);		//�󶨵��������˿ڣ��ಥ��ַ
	netconn_join_leave_group(agent_udp_server_netconn, &multicast_ip, &localhost_ip, NETCONN_JOIN);		//����ಥ��
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
