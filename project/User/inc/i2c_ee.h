/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __I2C_EE_H
#define __I2C_EE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* The M24C08W contains 4 blocks (128byte each) with the adresses below: E2 = 0 */
/* EEPROM Addresses defines */
#define EEPROM_Block0_ADDRESS 0xA0   /* E2 = 0 */
//#define EEPROM_Block1_ADDRESS 0xA2 /* E2 = 0 */
//#define EEPROM_Block2_ADDRESS 0xA4 /* E2 = 0 */
//#define EEPROM_Block3_ADDRESS 0xA6 /* E2 = 0 */
#define  EEPTx_BUFSIZE      sizeof(EEP_Tx_Buffer) / sizeof(EEP_Tx_Buffer[0])
#define  EEP_Firstpage      0x00
#define  EEP_Randompage	    0x06

//TestStatus TransferStatus;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void GPIO_Configuration(void);
void I2C_Configuration(void);

void I2C_EE_Init(void);
void I2C_EE_ByteWrite(u8* pBuffer, u8 WriteAddr);
void I2C_EE_PageWrite(u8* pBuffer, u8 WriteAddr, u8 NumByteToWrite);
void I2C_EE_BufferWrite(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite);
void I2C_EE_BufferRead(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead);
void I2C_EE_WaitEepromStandbyState(void);
void I2C_Test(void);

#endif /* __I2C_EE_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/


