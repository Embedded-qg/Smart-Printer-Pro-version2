/***************************************************************************************
 *	FileName					: agent_tcp.c
 *	CopyRight					: JockJo
 *	ModuleName				: agent_tcp	
 *
 *	CPU							:	
 *	RTOS						:
 *
 *	Create Data					: 2017-8-19	
 *	Author/Corportation			:	JockJo
 *
 *	Abstract Description		:	agent_tcp
 *
 *--------------------------------Revision History--------------------------------------
 *	No	version		Data			Revised By			Item			Description
 *	1		1.0     2017-8-19    JockJo
 *	2		1.0.1		2017-9-15		 JockJo										finish the tcp 
 ***************************************************************************************/


/**************************************************************
*	Debug switch Section
**************************************************************/


/**************************************************************
*	Include File Section
**************************************************************/
#include "agent_tcp.h"

/**************************************************************
*	Macro Define Section
**************************************************************/



/**************************************************************
*	Struct Define Section
**************************************************************/


/**************************************************************
*	Prototype Declare Section
**************************************************************/


/**************************************************************
*	Global Variable Declare Section
**************************************************************/
struct netconn *agent_tcp_client_netconn;		//本地客户端，用于向peer发送数据
struct netconn *agent_tcp_server_netconn;		//本地服务器，用于监听peer连接
u8_t agent_tcp_backlog = 10;			//最大监听数

/**************************************************************
*	File Static Variable Define Section
**************************************************************/


/**************************************************************
*	Function Define Section
**************************************************************/



/**
 *  @name	    con_to_agent
 *	@description   与对等的agent建立TCP连接
 *	@param			none
 *	@return		  none
 *  @notice
 */
void con_to_agent(void)
{
	err_t  con_err;
	static u8_t con_time = 0;
	struct netconn *agent_tcp_client_netconn;	
	struct ip_addr peer_ip;
	
	if((agent_tcp_client_netconn = netconn_new(NETCONN_TCP)) != NULL){
		DEBUG_PRINT("TCP netconn build.\n");
	}else{
		DEBUG_PRINT("Fail to build netconn.\n");
	}
	netconn_set_recvtimeout(agent_tcp_client_netconn,10000);//设置接收延时时间 		
	
	/*这里需要调用UDP广播获取peer的IP地址*/
	IP4_ADDR(&peer_ip, 192,168,1,135);//IP of peer
	if( (con_err = netconn_connect(agent_tcp_client_netconn, &peer_ip, 8000)) == ERR_OK )	
	{
		con_time = 0;
		write_connection(agent_tcp_client_netconn, first_req, REQ_LINK_OK, 0);//初次请求建立发送主控板ID
	}
	//失败重新连接，超过10次放弃连接
	else
	{
		con_time += 1;
		if(con_time > 9)
		{
			DEBUG_PRINT("Fail to connect to peer.\n");
			return ;
		}
		OSTimeDlyHMSM(0,0,1,0);
		con_to_agent();
	}
} 


/**
 *  @name	    agent_tcp_server
 *	@description   服务器端，负责监听其他主机的连接，最多监听十个
 *	@param			none
 *	@return		  none
 *  @notice
 */
void agent_tcp_server(void)
{
	struct ip_addr localhost_ip;
	struct netconn *peer_newconn;	
	err_t err_tcp;
	
	if((agent_tcp_server_netconn = netconn_new(NETCONN_TCP)) != NULL){
		DEBUG_PRINT("TCP netconn build.\n");
	}else{
		DEBUG_PRINT("Fail to build netconn.\n");
	}	
	netconn_set_recvtimeout(agent_tcp_server_netconn,10000);//设置接收延时时间 		

	netconn_bind(agent_tcp_server_netconn, IP_ADDR_ANY, 8000);		//绑定到本地所有IP，全部进行监听
	netconn_listen_with_backlog(agent_tcp_server_netconn, agent_tcp_backlog);	//开启TCP监听，最多监听10个
	
	while( (err_tcp = netconn_accept(agent_tcp_server_netconn, &peer_newconn)) == ERR_OK ) 		//接收新的连接请求
	{
			DEBUG_PRINT("receive from peer.\n");
			rec_from_peer(peer_newconn);
			/*开启新的线程，接收数据*/
			/*接收数据完成，关闭该连接*/
	}
}

/**
 *  @name	    send_to_peer
 *	@description   向对等的ageng发送信息
 *	@param			none
 *	@return		  none
 *  @notice
 */
void send_to_peer(struct netconn *peer_newconn)
{
	/*此处为需要发送的打印信息内容*/
	write_connection(peer_newconn, order_req, ORDER_REQUEST, 0);//请求订单			此处可以修改为需要请求的内容
	DEBUG_PRINT("send to peer.\n");
}


/**
 *  @name	    rec_from_peer
 *	@description   从对等的agent接收数据
 *	@param			none
 *	@return		  none
 *  @notice
 */
void rec_from_peer(struct netconn *peer_newconn)
{
	receive_connection(peer_newconn);		//沿用Smart-Printer-Pro-version2的数据协议
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

