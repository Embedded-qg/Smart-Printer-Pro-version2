#include "pack_data.h"

/**************************************************************
*	Function Define Section
**************************************************************/
/****************************************************************************************
*@Name............: Pack_Req_Or_Status_Message
*@Description.....: 封装数据报
*@Parameters......: message		:用于存放即将打包的消息
*					type		:报文类型
*					symbol		:标志位
*					id			:可以是主控板id(32位),也可以批次序号(高16位)
*					UNIX_time	:服务器或主控板发送时间；在本地状态应答时，为区分打印机或订单应答
*					preservation:保留字段，在订单中，高16位为批次序号，低16位为批次内序号；在本地状态应答时为应答序号
*@Return values...: void
*****************************************************************************************/
void Pack_Req_Or_Status_Message(char *message, req_type type, u8_t symbol, u32_t id, u32_t UNIX_time, u32_t preservation)
{
	u16_t check_sum;
	
	/*起始符*/
	message[0] = '\xCF';
	message[1] = '\xFC';
	
	/*设置类型和标志*/
	message[2] = type;
	message[3] = symbol;
	
	SET_DATA_4B(&message[4], id);//设置id

	SET_DATA_4B(&message[8], UNIX_time);//设置Unix时间戳
	
	SET_DATA_4B(&message[12], preservation);//填充段
	
	/*终止符*/
	message[18] = '\xFC';
	message[19] = '\xCF';
	
	/*获取校验和*/
	check_sum = Check_Sum((u16_t*)message, SEND_DATA_SIZE);
	SET_DATA_2B(&message[16], ((check_sum << 8) + (check_sum >> 8)));
}

void Pack_Contract_Message(char *message,contract_type type,u32_t mcu_id,u32_t UNIX_time,u16_t mcu_speed,u16_t mcu_health,u32_t preservation)
{
	u16_t check_sum;

//起始符
	message[CONTRACT_START_SYMBOL_OFFSET] = '\xbf';
	message[CONTRACT_START_SYMBOL_OFFSET+1] = '\xfb';
	
//设置类型
	message[CONTRACT_TYPE_OFFSET] = type;
	
//设置时间,主控板id，主控板打印速度，主控板健康状态
	SET_DATA_4B(&message[CONTRACT_SERVER_SEND_TIME_OFFSET],UNIX_time);
	SET_DATA_4B(&message[CONTRACT_MCU_ID_OFFSET],mcu_id);
	SET_DATA_2B(&message[CONTRACT_MCU_SPEED_OFFSET],mcu_speed);
	SET_DATA_2B(&message[CONTRACT_MCU_HEALTH],mcu_health);
	
//终止符
	message[CONTRACT_TAIL_OFFSET] = '\xfb';
	message[CONTRACT_TAIL_OFFSET+1] = '\xbf';
	
//校验和	
	check_sum = Check_Sum((u16_t *)message,MAX_CONTRACT_HEAD_LENGTH);

}
