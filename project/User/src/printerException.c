#include "printerException.h"
#include "i2c_ee.h"
#include "compress.h"
#include "format.h"

USART_TypeDef * portSelect[4] = {0,USART1,UART4,USART3};

/* static function declaration */
static unsigned char stateCheck(u8_t option, u8_t deviceNum);


/*********************************************************************************************************
*                                             mySendData
*
* Description: This function is used to command corrsponding port to Send out data
*
* Arguments  : dataBuff - the first address of the dataBuff 
*							 size - the size of the dataBuff
*							 deviceNum - portSelect[deviceNum] (the set of usart you gonna use to ouuput data)
*
* Returns    : None
*********************************************************************************************************/
void mySendData(u8_t *dataBuff, u16_t size, u8_t deviceNum)
{
	u16_t counter;	//keep sending the byte to printer
	for (counter = 1; counter <= size; counter++, dataBuff++)
		PRINTER_PUT(*dataBuff, deviceNum);
}


/*********************************************************************************************************
*                                             printQRCode
*
* Description: This function is used to printQRCode
*
* Arguments  : data - the first address of the dataBuff(usually dataBuff must be an address of an URL)
*			   			 size - the size of the data (u16_t)
*
* Returns    : None
*********************************************************************************************************/
void printQRCode(u8_t *data, u16_t size, u8_t deviceNum)
{
	u8_t nL = (u8_t)(size & 0x00ff);
	u8_t nH = (u8_t)((size >> 8) & 0x00ff);
	u8_t cmd1[4] = {0x1d, 0x01, 0x03, 0x08}; //dot-size configuration :8
	u8_t cmd2[4] = {0x1d, 0x01, 0x04, 0x33}; //error-correcting level configuration :H level
	u8_t cmd3[3] = {0x1d, 0x01, 0x01};			 //send length 
	u8_t cmd4[3] = {0x1d, 0x01, 0x02};			 //start printing QRCode
	
	u8_t cmd5[3] = {0x1b, 0x61, 0x01};
	u8_t cmd6[3] = {0x1b, 0x61, 0x03};
	
	mySendData(cmd5, sizeof(cmd5), deviceNum); //set
	
	mySendData(cmd1, sizeof(cmd1), deviceNum);
	mySendData(cmd2, sizeof(cmd2), deviceNum);
	mySendData(cmd3, sizeof(cmd3), deviceNum);
	PRINTER_PUT(nL,deviceNum);						 
	PRINTER_PUT(nH,deviceNum);						 
	mySendData(data, size, deviceNum);
	mySendData(cmd4, sizeof(cmd4), deviceNum);	//start printing QRCode

	mySendData(cmd6, sizeof(cmd6), deviceNum); //reset
}

/*********************************************************************************************************
*                                             printImages
*
* Description: This function is used to printImages
*
* Arguments  : data - the first address of the dataBuff 
*			   			 size - the size of the data
*
* Returns    : None
*********************************************************************************************************/
void printImages(u8_t *data, u16_t size, u8_t deviceNum)
{
	DEBUG_PRINT("printImages\n");
	uncompress_print(data, data + size + 1, deviceNum);
}	

/*********************************************************************************************************
*                                          printImages_Alternate
*
* Description: This function is used to printImages
*
* Arguments  : data - the first address of the dataBuff 
*			   			 size - the size of the data
*							 length - numbers of dots from horizontal direction
*							 width - numbers of dots from vertical direction							 
*
* Returns    : None
*********************************************************************************************************/
void printImages_Alternate(u8_t *data, u16_t size, u16_t length, u16_t width, u8_t deviceNum)
{
	u8_t XL, XH, YL, YH;
	u8_t scale = 0x00;   //Option byte: scale of the image  : NORMAL 
//	u8_t scale = 0x03;   //Option byte: scale of the image  : FOURTIMES 
	u8_t cmd[3] = {0x1d, 0x76, 0x30};
	XL = (u8_t)(length/8 & 0x00ff);
	XH = (u8_t)((length/8 >> 8) & 0x00ff);
	YL = (u8_t)(width & 0x00ff);
	YH = (u8_t)((width >> 8) & 0x00ff);
	
	mySendData(cmd, sizeof(cmd), deviceNum); //send cmd
	PRINTER_PUT(scale, deviceNum); //send scale
	
	PRINTER_PUT(XL,deviceNum);
	PRINTER_PUT(XH,deviceNum);
	PRINTER_PUT(YL,deviceNum);
	PRINTER_PUT(YH,deviceNum);// send XL,XH,YL,YH

	mySendData(data, size, deviceNum); //send the image-bytes
}

/********************************************************************************************************
*                                             outputData
*
* Description: This function is used to Output the data(perhaps an order) from dataBuff one byte by one byte
*
* Arguments  : data - the first address of the data 
*			   			 size - the size of the data
*							 deviceNum - portSelect[deviceNum] (the set of usart you gonna use to ouuput data)
*
* Returns    : None
*********************************************************************************************************/
unsigned char outputData(u8_t* data, u16_t size, u8_t deviceNum)
{
    u16_t i;

	for (i = 0; i < size; i++)
		PRINTER_PUT(data[i], deviceNum);
	return ORDER_CREATED_OK;  	//the order is printed successfully
}

/********************************************************************************************************
*                                             cutPaper
*
* Description: This function is used to Send paper-cutting command to the printer through USART1
*
* Arguments  : deviceNum
*
* Returns    : none
********************************************************************************************************/
void cutPaper(u8_t deviceNum)
{
	u8_t cmd1[] = {0x1b, 0x69};
	u8_t cmd2[] = {0x1b, 0x4a, 0xff}; //打印并向前走纸n点行
	u8_t cmd3[] = {0x1b, 0x40};
	
	PCMgr.cells[deviceNum-1].cutCnt++;	// 切刀次数加一
	
	outputData(cmd2, sizeof(cmd2), deviceNum);

	outputData(cmd1, sizeof(cmd1), deviceNum);

	outputData(cmd3, sizeof(cmd3), deviceNum);
}

void initialPrinter(u8_t deviceNum)
{
	u8_t cmd3[] = {0x1b, 0x40};
	
	outputData(cmd3, sizeof(cmd3), deviceNum);
}

/********************************************************************************************************
*                                             getPrinterState
*
* Description: This function is used to examine the Real-Time @ General state @ of the printer
*
* Arguments  : deviceNum number of device 
*
* Returns    : the corresponding state of the printer
********************************************************************************************************/
unsigned char getPrinterState(u8_t deviceNum)
{
	unsigned char tmp[4] = {0};	
	unsigned i;
	unsigned char state = 0;
	
	for (i = 0; i <= 3; i++)
		tmp[i] = stateCheck(i + 1, deviceNum); //option = i + 1
	

	//find out the matched situation	
	if (tmp[0] == 0x08 && tmp[1] == 0x40 && tmp[2] == 0x28 && tmp[3] == 0x0c)//KNIFE_ERROR_STATE 
		state = KNIFE_ERROR_STATE;
	
	else if ((tmp[0] == 0x08 && tmp[1] == 0x44 && tmp[2] == 0x20 && tmp[3] == 0x0c) || 
			(tmp[0] == 0x08 && tmp[1] == 0x44 && tmp[2] == 0x20 && tmp[3] == 0x0c))//UNEXPECTED_OPENED_STATE  
		state = UNEXPECTED_OPENED_STATE;
	
	else if ((tmp[0] == 0x08 && tmp[1] == 0x20 && tmp[2] == 0x00 && tmp[3] == 0x6c) ||
				(tmp[0] == 0x08 && tmp[1] == 0x64 && tmp[2] == 0x20 && tmp[3] == 0x6c))//PAPER_INSUFFICIENT_STATE  
		state = PAPER_INSUFFICIENT_STATE;

	else if (tmp[0] == 0x08 && tmp[1] == 0x48 && tmp[2] == 0x28 && tmp[3] == 0x0c)//FILL_IN_PAPER_STATE 
		state = FILL_IN_PAPER_STATE;

	else if (tmp[0] == 0x00 && tmp[1] == 0x00 && tmp[2] == 0x00 && tmp[3] == 0x0c)//SELF_CHECK_STATE 
		state = NORMAL_STATE;
	
	else 
	{
		state = NORMAL_STATE;
//		if ((state == NORMAL_STATE) && (tmp[3] == 0x6c))
//			state = PAPER_WILL_BE_IN_INSUFFICIENT_STATE;
	}

	return state;
}

/********************************************************************************************************
*                                             stateCheck
*
* Description: This function is used to collect the @ Branch state @ of the printer
*
* Arguments  : 	option - the corresponding part of the printer to  be checked
*				deviceNum - the number of device 
* Returns    : the corresponding state of the certain part of the printer
********************************************************************************************************/
static unsigned char stateCheck(u8_t option, u8_t deviceNum)
{
	unsigned char command[2] = {0x10, 0x04};

	outputData(command, sizeof(command), deviceNum);
	PRINTER_PUT(option, deviceNum);
	 
	return PRINTER_GET(deviceNum);
}


/**	
 * @fn		SendStatusCheckCmd
 * @brief	向打印单元发送检查状态指令
 *
 * @notes	反馈将在串口接收中断中进行处理
 */
void SendStatusCheckCmd(u8_t deviceNum)
{
	int i;
	u8_t cmd[] = {
		0x10, 0x04, 0x01, 
		0x10, 0x04, 0x02, 
		0x10, 0x04, 0x03,
		0x10, 0x04, 0x04,
	};
	
	for(i = 0; i < 12; i++) 
		PRINTER_PUT(cmd[i], deviceNum);
}


/**	
 * @fn		GetAndResolveStatus
 * @brief	解析打印单元的状态
 *
 * @notes	此函数应该在串口中断中被调用
 */
u8_t ResolveStatus(u8_t tmp[4])
{
	u8_t state;
	
	//find out the matched situation	
	if (tmp[0] == 0x08 && tmp[1] == 0x40 && tmp[2] == 0x28 && tmp[3] == 0x0c)//KNIFE_ERROR_STATE 
		state = KNIFE_ERROR_STATE;
	
	else if ((tmp[0] == 0x08 && tmp[1] == 0x44 && tmp[2] == 0x20 && tmp[3] == 0x0c) || 
			(tmp[0] == 0x08 && tmp[1] == 0x44 && tmp[2] == 0x20 && tmp[3] == 0x0c))//UNEXPECTED_OPENED_STATE  
		state = UNEXPECTED_OPENED_STATE;
	
	else if ((tmp[0] == 0x08 && tmp[1] == 0x20 && tmp[2] == 0x00 && tmp[3] == 0x6c) ||
				(tmp[0] == 0x08 && tmp[1] == 0x64 && tmp[2] == 0x20 && tmp[3] == 0x6c))//PAPER_INSUFFICIENT_STATE  
		state = PAPER_INSUFFICIENT_STATE;

	else if (tmp[0] == 0x08 && tmp[1] == 0x48 && tmp[2] == 0x28 && tmp[3] == 0x0c)//FILL_IN_PAPER_STATE 
		state = FILL_IN_PAPER_STATE;

	else if (tmp[0] == 0x00 && tmp[1] == 0x00 && tmp[2] == 0x00 && tmp[3] == 0x0c)//SELF_CHECK_STATE 
		state = NORMAL_STATE;
	
	else 
		state = NORMAL_STATE;	
	
	return state;
}

/** 
 * @fn		ReadyPrintImage
 * @brief	使指定deviceNum的打印机为打印size大小的图片作准备
 */
void ReadyPrintImage(u16_t size, u8_t deviceNum)
{
	unsigned char n1 = (unsigned char)(size & 0x000000ff);
	unsigned char n2 = (unsigned char)((size >> 8) & 0x00000007);
	unsigned char command[2] = {0x1b , 0x2a};//1b 2a
		
	outputData(command, sizeof(command), deviceNum);
	PRINTER_PUT(n1, deviceNum); //n1
	PRINTER_PUT(n2, deviceNum); //n2	
}

/*****************************************************************************************EEPROM SECTION********************************************************************************************/
/*******************************************************************************
* Function Name  : specialAdditon
* Description    : To do Addtion operation for a 4-bytes-long data area
* Input          : - mem : pointer to the buffer  containing the data to be 
*                    operated addition operation
*                  - numsToAdd : numbers gonna gonna be added
* Output         : None
* Return         : None
*	Note					 : the API has passed  the test in Code::Blocks Platform
*******************************************************************************/
static void specialAdditon(unsigned char* mem, unsigned char numsToAdd)
{
    unsigned char tmp = numsToAdd;
    unsigned char tmp1 = 0x00;
    unsigned char highB = 1, mediumB = 2, lowB = 3;

    if ((unsigned short)(numsToAdd + mem[lowB]) > 255)
    {
        tmp1 = (unsigned short)(numsToAdd + mem[lowB]) % 255;
        //Carry
        mem[lowB] = 0x00;
        mem[lowB] += tmp1 - 1;

        if ((unsigned short)(mem[mediumB] + 1) > 255)
        {
            mem[mediumB] = 0x00;
            //Carry
            mem[highB] += mem[highB] + 1; //Normally the
        }
        else
            mem[mediumB] += 1;
    }
    else
        mem[lowB] += numsToAdd;
}

/*******************************************************************************
* Function Name  : ClearEEPROM
* Description    : Clear the first Page of the EEPROM
* Input          : None 
* Output         : None
* Return         : None
*******************************************************************************/
void ClearEEPROM(void)
{
    u16 i;
    u8 I2c_Buf_Write[256];
    u8 I2c_Buf_Read[256];

    for(i=0;i<=255;i++)
			I2c_Buf_Write[i]=0;

		WriteEEPROM(I2c_Buf_Write, 0x00, 0xff);
}

/*******************************************************************************
* Function Name  : WriteEEPROM
* Description    : Writes buffer of data to the I2C EEPROM.
* Input          : - data : pointer to the buffer  containing the data to be 
*                    written to the EEPROM.
*                  - WriteAddr : EEPROM's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the EEPROM.
* Output         : None
* Return         : None
*******************************************************************************/
void WriteEEPROM(u8_t *data, u8_t WriteAddr, u8_t NumByteToWrite)
{
	 //将data中顺序递增的数据写入EERPOM中 
   I2C_EE_BufferWrite(data, WriteAddr, NumByteToWrite);
}

/*******************************************************************************
* Function Name  : ReadEEPROM
* Description    : Reads a block of data from the EEPROM.
* Input          : - data : pointer to the buffer that receives the data read 
*                    from the EEPROM.
*                  - ReadAddr : EEPROM's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the EEPROM.
* Output         : None
* Return         : None
*******************************************************************************/
void ReadEEPROM(u8_t *data, u8_t ReadAddr, u8_t NumByteToRead)
{
	//data中顺序递增的数据写入EERPOM中 
   I2C_EE_BufferRead(data, ReadAddr, NumByteToRead);
}
