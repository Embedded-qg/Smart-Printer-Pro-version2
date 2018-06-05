/***************************************************************************************
 *	FileName					: transfer_task.c
 *	CopyRight					: JockJo
 *	ModuleName				: transfer_task	
 *
 *	CPU							:	
 *	RTOS						:
 *
 *	Create Data					: 2017-12-09	
 *	Author/Corportation			:	JockJo
 *
 *	Abstract Description		:	transfer_task
 *
 *--------------------------------Revision History--------------------------------------
 *	No	version		Data			Revised By			Item			Description
 *	1		1.0     2017-12-09    JockJo
 ***************************************************************************************/


/**************************************************************
*	Debug switch Section
**************************************************************/


/**************************************************************
*	Include File Section
**************************************************************/
#include "transfer_task.h"

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
 *  @name	    transf_task
 *	@description   进行任务转移
 *	@param			none
 *	@return		  none
 *  @notice
 */
void transf_task(struct netconn *conn, req_type type, u8_t symbol, u32_t target_id, u32_t preservation)
{
		char send_data[TRANSF_MESS_SIZE] = {0};	//任务转移报文是固定24字节
		err_t err;
		int i = 0;
			printf("\nbbbbbbbbb\n");
		
		if(type == order_status){
			Pack_TransfTask_Message(send_data, ORDER_STATUS, symbol, Get_MCU_ID(), target_id, 
															Get_Current_Unix_Time(), preservation);	//订单状态中preservation为订单批次号和批次内序号
		}
			printf("\nssssssssss\n");
		
		while(0 != (err = netconn_write(conn, send_data, TRANSF_MESS_SIZE, NETCONN_COPY))){
			if(ERR_IS_FATAL(err))//致命错误，表示没有连接
				break;
		
			//当网络写入错误时，需要等待一段时间后继续写入该数据包，否则无法反馈给服务器
			OSTimeDlyHMSM(0,0,++i,0);
			printf("\n\n\nTransf_task:NETCONN WRITE ERR_T IS %d\n\n\n", err);
			
			if(type != order_req){//订单请求需持续发送，否则服务器将无法下达订单
				if(i > 3) break;
			}else if(i > 10) break;//但等待多次后是无意义的
		}
		
			printf("\n-------------------------\n");
			for(i=0;i<TRANSF_MESS_SIZE;i++)
				printf("\n%d = %x\n", i,send_data[i]);
			printf("\n-------------------------\n");
		
}

u32_t Get_TARGET_ID()
{
		return 0;
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

