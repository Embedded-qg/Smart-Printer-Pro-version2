#ifndef PRINTEREXCEPTION_H
#define PRINTEREXCEPTION_H

#include "stm32f2xx.h"
#include "data_form.h"
#include "print_cells.h"

/*********************************************************Gloabal Area**************************************************/
extern USART_TypeDef * portSelect[4];

#define PRINTER_PUT(data, deviceNum) USART_SendData(portSelect[deviceNum], data)
#define PRINTER_GET(deviceNum) USART_ReceiveData(portSelect[deviceNum])

#define SEND_STATUS_CMD_ONE(deviceNum) 		\
	do {									\
		PRINTER_PUT(0x10, deviceNum);		\
		PRINTER_PUT(0x04, deviceNum);		\
		PRINTER_PUT(0x01, deviceNum);		\
	}while(0)
		
#define SEND_STATUS_CMD_TWO(deviceNum) 		\
	do {									\
		PRINTER_PUT(0x10, deviceNum);		\
		PRINTER_PUT(0x04, deviceNum);		\
		PRINTER_PUT(0x02, deviceNum);		\
	}while(0)

#define SEND_STATUS_CMD_THREE(deviceNum) 	\
	do {									\
		PRINTER_PUT(0x10, deviceNum);		\
		PRINTER_PUT(0x04, deviceNum);		\
		PRINTER_PUT(0x03, deviceNum);		\
	}while(0)	
	
#define SEND_STATUS_CMD_FOUR(deviceNum) 	\
	do {									\
		PRINTER_PUT(0x10, deviceNum);		\
		PRINTER_PUT(0x04, deviceNum);		\
		PRINTER_PUT(0x04, deviceNum);		\
	}while(0)

//API package

/*********************************************************************************************************
*                                             printQRCode
*
* Description: This function is used to printQRCode
*
* Arguments  : data - the first address of the dataBuff(usually dataBuff must be an address of an URL)
*			   			 size - the size of the data
*
* Returns    : None
*********************************************************************************************************/
void printQRCode(u8_t *data, u16_t size, u8_t deviceNum);	
	
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
void mySendData(u8_t *data, u16_t size, u8_t deviceNum);


/********************************************************************************************************
*                                             printImages
*
* Description: This function is used to print images
*
* Arguments  : dataBuff - the first address of the dataBuff 
*			   				size - the size of the dataBuff
*
* Returns    : none
*********************************************************************************************************/
void printImages(u8_t* data, u16_t size, u8_t deviceNum);

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
void printImages_Alternate(u8_t *data, u16_t size, u16_t length, u16_t width, u8_t deviceNum);


/********************************************************************************************************
*                                             outputData
*
* Description: This function is used to Output the data from dataBuff one byte by one byte
*
* Arguments  : dataBuff - the first address of the dataBuff 
*			   size - the size of the dataBuff
*
* Returns    : None
*********************************************************************************************************/
unsigned char outputData(u8_t* data, u16_t size, u8_t deviceNum);


/********************************************************************************************************
*                                             cutPaper
*
* Description: This function is used to Send paper-cutting command to the printer through USART1
*
* Arguments  : deviceNum
*
* Returns    : none
********************************************************************************************************/
void cutPaper(u8_t deviceNum);

/********************************************************************************************************
*                                             getPrinterState
*
* Description: This function is used to examine the Real-Time @ General state @ of the printer
*
* Arguments  : deviceNum
*
* Returns    : the corresponding state of the printer
********************************************************************************************************/
unsigned char getPrinterState(u8_t deviceNum);

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
void WriteEEPROM(u8_t *data, u8_t WriteAddr, u8_t NumByteToWrite);

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
void ReadEEPROM(u8_t *data, u8_t ReadAddr, u8_t NumByteToWrite);

/*******************************************************************************
* Function Name  : ClearEEPROM
* Description    : Clear the first Page of the EEPROM
* Input          : None 
* Output         : None
* Return         : None
*******************************************************************************/
void ClearEEPROM(void);

/*******************************************************************************
* Function Name  : specialAdditon
* Description    : To do Addtion operation for a 3-bytes-long data area
* Input          : - mem : pointer to the buffer  containing the data to be 
*                    operated addition operation
*                  - numsToAdd : numbers gonna gonna be added
* Output         : None
* Return         : None
*	Note					 : the API has passed  the test in Code::Blocks Platform
*******************************************************************************/
static void specialAdditon(unsigned char* mem, unsigned char numsToAdd);


void initialPrinter(u8_t deviceNum);
	
/** 
 * @fn		ReadyPrintImage
 * @brief	使指定deviceNum的打印机为打印size大小的图片作准备
 */
void ReadyPrintImage(u16_t size, u8_t deviceNum);


/**	
 * @fn		SendStatusCheckCmd
 * @brief	向打印单元发送检查状态指令
 *
 * @notes	反馈将在串口接收中断中进行处理
 */
void SendStatusCheckCmd(u8_t deviceNum);


/**	
 * @fn		GetAndResolveStatus
 * @brief	解析打印单元的状态
 *
 * @notes	此函数应该在串口中断中被调用
 */
u8_t ResolveStatus(u8_t tmp[4]);


#endif 
