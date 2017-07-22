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