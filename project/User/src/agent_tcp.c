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
 *
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


/**************************************************************
*	File Static Variable Define Section
**************************************************************/


/**************************************************************
*	Function Define Section
**************************************************************/



/**
 *  @name	    con_to_agent
 *	@description   ��Եȵ�agent����TCP����
 *	@param			none
 *	@return		  none
 *  @notice
 */
void con_to_agent(void)
{
	err_t  con_err;
	static u8_t con_time = 0;
	struct ip_addr peer_ip;
	
	if((agent_tcp_client_netconn = netconn_new(NETCONN_TCP)) != NULL){
		DEBUG_PRINT("TCP netconn build.\n");
	}else{
		DEBUG_PRINT("Fail to build netconn.\n");
	}
	
	netconn_set_recvtimeout(order_netconn,10000);//���ý�����ʱʱ�� 		
	/*������Ҫ��ȡpeer��IP��ַ*/
	IP4_ADDR(&peer_ip, 192,168,1,116);//IP of peer
	if( (con_err = netconn_connect(agent_tcp_client_netconn,&peer_ip, 8000)) == ERR_OK )	
	{
		con_time = 0;
		write_connection(agent_tcp_client_netconn, first_req, REQ_LINK_OK, 0);//�����������������ذ�ID
	}
	//ʧ���������ӣ�����10�η�������
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
 *	@description   agent_tcp_server
 *	@param			none
 *	@return		  none
 *  @notice
 */
void agent_tcp_server(void)
{
	
}
/**
 * @brief ��Եȵ�agent����TCP����
 */
void send_to_agent(void)
{


}


/**
 * @brief �ӶԵȵ�agent����TCP����
 */
void rec_from_agent(void)
{
	extern struct netconn *order_netconn;	//ȫ��TCP����

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

