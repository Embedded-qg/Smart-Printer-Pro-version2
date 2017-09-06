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
struct netconn *agent_udp_client_netconn;		//���ؿͻ��ˣ�������peer��������
struct netconn *agent_udp_server_netconn;		//���ط����������ڼ���peer����
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
 *	@description   UDPӦ�𣬷��ͱ�����IP���൱��UDP������
 *	@param			none
 *	@return		  none
 *  @notice
 */
void reply_to_peer()
{
	
 
}
 
/**
 *  @name	    broadcast_to_localLAN
 *	@description   UDP�㲥, Ѱ�ҷ���Ҫ�������IP���൱��UDP�ͻ���
 *	@param			none
 *	@return		  none
 *  @notice
 */
void  broadcast_to_localLAN()
{
/*�㷢����Ϣ�Ĵ���*/
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
	netconn_set_recvtimeout(agent_udp_client_netconn,10000);//���ý�����ʱʱ�� 		 
	
	netconn_bind(agent_udp_client_netconn, IP_ADDR_BROADCAST, 8001);		//�󶨵��㲥�˿�
//��㲥����
//	netconn_send(agent_udp_client_netconn, buf);			//���й㲥

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

