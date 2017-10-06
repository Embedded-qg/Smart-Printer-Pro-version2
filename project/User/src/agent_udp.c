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
struct netconn *agent_udp_client_netconn = (void*)0;		//本地客户端，用于向peer发送数据
struct netconn *agent_udp_server_netconn = (void*)0;		//本地服务器，用于监听peer连接
const static u8_t UDPASKDATA_FREE[] = "Who is free?";
const static u8_t UDPASKDATA_WISF[] = "WISF";
const static u8_t UDPREPLYDATA_IAMF[] = "IAMF";
struct netbuf *buf_send = (void*)0;
struct pbuf *pbuf_send = (void*)0;
struct ip_addr multicast_ip;	
extern struct ip_addr localhost_ip;
/**************************************************************
*	File Static Variable Define Section
**************************************************************/


/**************************************************************
*	Function Define Section
**************************************************************/



/**
 *  @name	    reply_to_peer
 *	@description   UDP应答，发送本主机IP，相当于UDP服务器
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
	/*此处需要一个解析程序，得出目标地址*/
	add_sned_data(buf_send, (u8_t *)UDPREPLYDATA_IAMF);
	netconn_sendto(agent_udp_server_netconn, buf_send, &peer_ip, UDP_SERVER_PORT);			//进行单播
}
 
/**
 *  @name	    broadcast_to_localLAN
 *	@description   UDP多播, 寻找符合要求的主机IP，相当于UDP客户端
 *	@param			none
 *	@return		  none
 *  @notice
 */
void  multicast_to_localLAN(void)
{
	netconn_sendto(agent_udp_server_netconn, buf_send, &multicast_ip, UDP_SERVER_PORT);			//进行多播
//		udp_sendto(agent_udp_server_netconn->pcb.udp, pbuf_send, &multicast_ip, UDP_SERVER_PORT);
}

/**
 *  @name	    init_UDP
 *	@description   初始化UDP使用到的bufs
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
 *	@description   创建buf和pbuf
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
 *	@description   向buf中添加待发送信息
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
 *	@description   设置多播地址
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
}

/**
 *  @name	    init_client_server
 *	@description   初始化UDP客户端和服务器，绑定端口和地址
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
	netconn_set_recvtimeout(agent_udp_client_netconn,10000);//设置接收延时时间 		 
	netconn_set_recvtimeout(agent_udp_server_netconn,10000);//设置接收延时时间 		 	
	
	netconn_bind(agent_udp_client_netconn, IP_ADDR_ANY, UDP_CLIENT_PORT);		//绑定到到本地所有地址，客户端端口
	netconn_bind(agent_udp_server_netconn, IP_ADDR_ANY, UDP_SERVER_PORT);		//绑定到本地所有地址，服务器端口

#ifdef LWIP_IGMP
	netconn_join_leave_group(agent_udp_server_netconn, &multicast_ip, &localhost_ip, NETCONN_JOIN);		//服务器端加入多播组,接受信息
#endif 	//LWIP_IGMP
	
	//此处缺少一个server接受的处理函数
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

