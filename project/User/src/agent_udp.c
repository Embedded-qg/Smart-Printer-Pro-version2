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



/**************************************************************
*	Struct Define Section
**************************************************************/


/**************************************************************
*	Prototype Declare Section
**************************************************************/

/**************************************************************
*	Global Variable Declare Section
**************************************************************/
struct netconn *agent_udp_client_netconn;		//本地客户端，用于向peer发送数据
struct netconn *agent_udp_server_netconn;		//本地服务器，用于监听peer连接
const static u8_t UDPASKDATA_FREE[] = "Who is free?";
const static u8_t UDPASKDATA_WISF[] = "WISF";

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
	
 
}
 
/**
 *  @name	    broadcast_to_localLAN
 *	@description   UDP广播, 寻找符合要求的主机IP，相当于UDP客户端
 *	@param			none
 *	@return		  none
 *  @notice
 */
void  broadcast_to_localLAN()
{
/*你发送信息的创建*/
//	struct netbuf *buf;
//	
//	pbuf = pbuf_alloc(PBUF_RAW, sizeof(UDPASKDATA_WISF), PBUF_RAM);
//	buf->p = UDPASKDATA_WISF;
//	
	if((agent_udp_client_netconn = netconn_new(NETCONN_UDP)) != NULL){
		DEBUG_PRINT("UDP netconn build.\n");
	}else{
		DEBUG_PRINT("Fail to build netconn.\n");
	}
	netconn_set_recvtimeout(agent_udp_client_netconn,10000);//设置接收延时时间 		 
	
	netconn_bind(agent_udp_client_netconn, IP_ADDR_BROADCAST, 8001);		//绑定到广播端口
//拟广播内容
//	netconn_send(agent_udp_client_netconn, buf);			//进行广播

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

