#include "pack_data.h"

/**************************************************************
*	Function Define Section
**************************************************************/
/****************************************************************************************
*@Name............: Pack_Req_Or_Status_Message
*@Description.....: ��װ���ݱ�
*@Parameters......: message		:���ڴ�ż����������Ϣ
*					type		:��������
*					symbol		:��־λ
*					id			:���������ذ�id(32λ),Ҳ�����������(��16λ)
*					UNIX_time	:�����������ذ巢��ʱ�䣻�ڱ���״̬Ӧ��ʱ��Ϊ���ִ�ӡ���򶩵�Ӧ��
*					preservation:�����ֶΣ��ڶ����У���16λΪ������ţ���16λΪ��������ţ��ڱ���״̬Ӧ��ʱΪӦ�����
*@Return values...: void
*****************************************************************************************/
void Pack_Req_Or_Status_Message(char *message, req_type type, u8_t symbol, u32_t id, u32_t UNIX_time, u32_t preservation)
{
	u16_t check_sum;
	
	/*��ʼ��*/
	message[0] = '\xCF';
	message[1] = '\xFC';
	
	/*�������ͺͱ�־*/
	message[2] = type;
	message[3] = symbol;
	
	SET_DATA_4B(&message[4], id);//����id

	SET_DATA_4B(&message[8], UNIX_time);//����Unixʱ���
	
	SET_DATA_4B(&message[12], preservation);//����
	
	/*��ֹ��*/
	message[18] = '\xFC';
	message[19] = '\xCF';
	
	/*��ȡУ���*/
	check_sum = Check_Sum((u16_t*)message, SEND_DATA_SIZE);
	SET_DATA_2B(&message[16], ((check_sum << 8) + (check_sum >> 8)));
}

void Pack_Contract_Message(char *message,contract_type type,u32_t mcu_id,u32_t UNIX_time,u16_t mcu_speed,u16_t mcu_health,u32_t preservation)
{
	u16_t check_sum;

//��ʼ��
	message[CONTRACT_START_SYMBOL_OFFSET] = '\xbf';
	message[CONTRACT_START_SYMBOL_OFFSET+1] = '\xfb';
	
//��������
	message[CONTRACT_TYPE_OFFSET] = type;
	
//����ʱ��,���ذ�id�����ذ��ӡ�ٶȣ����ذ彡��״̬
	SET_DATA_4B(&message[CONTRACT_SERVER_SEND_TIME_OFFSET],UNIX_time);
	SET_DATA_4B(&message[CONTRACT_MCU_ID_OFFSET],mcu_id);
	SET_DATA_2B(&message[CONTRACT_MCU_SPEED_OFFSET],mcu_speed);
	SET_DATA_2B(&message[CONTRACT_MCU_HEALTH],mcu_health);
	
//��ֹ��
	message[CONTRACT_TAIL_OFFSET] = '\xfb';
	message[CONTRACT_TAIL_OFFSET+1] = '\xbf';
	
//У���	
	check_sum = Check_Sum((u16_t *)message,MAX_CONTRACT_HEAD_LENGTH);

}
