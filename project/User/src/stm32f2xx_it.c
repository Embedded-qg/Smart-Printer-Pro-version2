/**
  ******************************************************************************
  * @file    stm32f2xx_it.c
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    06-June-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_exti.h"
#include "stm32f2x7_eth.h"
#include "ucos_ii.h"
#include "stdio.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/* Extern variables ---------------------------------------------------------*/
extern unsigned int system_tick_num;
extern void process_mac(void);


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */	
//__asm void wait()
//{
//	BX LR
//}
void HardFault_Handler(void)
{		
//		__asm
//		{
//			mov r0,r1
//		}
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {

  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  /* Update the LocalTime by adding SYSTEMTICK_PERIOD_MS each SysTick interrupt */
	OS_CPU_SR  cpu_sr;
	
	system_tick_num++;
   OS_ENTER_CRITICAL();  //保存全局中断标志,关总中断/* Tell uC/OS-II that we are starting an ISR*/
   OSIntNesting++;
   OS_EXIT_CRITICAL();	  //恢复全局中断标志

   OSTimeTick();     /* Call uC/OS-II's OSTimeTick(),在os_core.c文件里定义,主要判断延时的任务是否计时到*/ 
   OSIntExit();  //在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换 
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


extern void LwIP_Pkt_Handle(void);
/**
  * @brief  This function handles ETH Handler.
  * @param  None
  * @retval None
  */
void ETH_IRQHandler(void)
{
	OS_CPU_SR cpu_sr;
	OS_ENTER_CRITICAL();    // 关中断                               
    OSIntNesting++;	   		//中断嵌套层数，通知ucos
    OS_EXIT_CRITICAL();	   	//开中断
	/* Handles all the received frames */
    /* check if any packet received */
    while(ETH_CheckFrameReceived())
    { 
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
	/* Clear the Eth DMA Rx IT pending bits */
	ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
	ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
	OSIntExit();//中断退出，通知ucos，（该句必须加）
}



/******************************************************************************
                           DMA2_Stream7_-TC(For USART1-TX)
******************************************************************************/
extern void USART1_DMA_TC_Hook(void);

void DMA2_Stream7_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_Stream7,DMA_IT_TCIF7) != RESET)
	{
		USART1_DMA_TC_Hook();
	}
	
	DMA_ClearFlag(DMA2_Stream7,DMA_FLAG_TCIF7);
}


/******************************************************************************
                           DMA1_Channel-7-TC (For USART2-TX)
******************************************************************************/
extern void USART2_DMA_TC_Hook(void);

void DMA1_Stream6_IRQHandler(void)  
{
	if(DMA_GetITStatus(DMA1_Stream6,DMA_IT_TCIF6) != RESET)
	{
		USART2_DMA_TC_Hook();
	}
	DMA_ClearFlag(DMA1_Stream6,DMA_FLAG_TCIF6);
}


/******************************************************************************
                           USART1_RXNE_IT
******************************************************************************/
extern void USART1_Hook(void);

#include"print_cells.h"

static INT8U sus_print_task[MAX_CELL_NUM] = {0xFF,0xFF,0xFF};

void USART1_IRQHandler(void)
{
	uint8_t ch;
	//The sentences are commented below are used to receive a byte one by one (in RXNE situation)
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //process the received byte
	{
		ch = USART_ReceiveData(USART1);
		if(ch != Xoff && ch != Xon)
		{
			USART1_Hook();
		}
		else if(ch == Xoff)  //驱动板缓冲区满
		{
			sus_print_task[0] = OSPrioHighRdy;
			OSTaskSuspend(OSPrioHighRdy);
		}
		else if(ch == Xon)    //驱动板缓冲区空
		{
			if(sus_print_task[0] != (INT8U)0xFF)   //检测之前是否已经挂起对应的打印线程
			{
				OSTaskResume(sus_print_task[0]);     //恢复打印线程
				sus_print_task[0] = 0xFF;
			}		
		}
	}
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}


/******************************************************************************
                           UART4_RXNE_IT
******************************************************************************/
extern void UART4_Hook(void);

void UART4_IRQHandler(void)
{
	uint8_t ch;
	//The sentences are commented below are used to receive a byte one by one (in RXNE situation)
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //process the received byte
	{
		ch = USART_ReceiveData(UART4);
		if(ch != Xoff && ch != Xon)
		{
			UART4_Hook();
		}
		else if(ch == Xoff)
		{
			sus_print_task[1] = OSPrioHighRdy;
			OSTaskSuspend(OSPrioHighRdy);
		}
		else if(ch == Xon)
		{
			if(sus_print_task[1] != (INT8U)0xFF)   //检测之前是否已经挂起对应的打印线程
			{
				OSTaskResume(sus_print_task[1]);     //恢复打印线程
				sus_print_task[1] = 0xFF;
			}		
		}
	}
	USART_ClearITPendingBit(UART4, USART_IT_RXNE);
}


/******************************************************************************
                           USART3_RXNE_IT
******************************************************************************/
#include "cc.h"

void USART3_IRQHandler(void)
{
	uint8_t ch;
	//The sentences are commented below are used to receive a byte one by one (in RXNE situation)
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //process the received byte
	{
//		ch = USART_ReceiveData(USART3);
//		if(ch != Xoff && ch != Xon)
//		{
//			USART3_Hook();
//		}
//		else if(ch == Xoff)
//		{
//			sus_print_task[3] = OSPrioHighRdy;
//			OSTaskSuspend(OSPrioHighRdy);
//		}
//		else if(ch == Xon)
//		{
//			if(sus_print_task[3] != (INT8U)0xFF)   //检测之前是否已经挂起对应的打印线程
//			{
//				OSTaskResume(sus_print_task[3]);     //恢复打印线程
//				sus_print_task[3] = 0xFF;
//			}		
//		}
	}
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}

void USART6_IRQHandler(void)
{
	uint8_t ch;
	//The sentences are commented below are used to receive a byte one by one (in RXNE situation)
	if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)  //process the received byte
	{
		ch = USART_ReceiveData(USART6);
		if(ch != Xoff && ch != Xon)
		{
			UART6_Hook();
		}
		else if(ch == Xoff)
		{
			sus_print_task[2] = OSPrioHighRdy;
			OSTaskSuspend(OSPrioHighRdy);
		}
		else if(ch == Xon)
		{
			if(sus_print_task[2] != (INT8U)0xFF)   //检测之前是否已经挂起对应的打印线程
			{
				OSTaskResume(sus_print_task[2]);     //恢复打印线程
				sus_print_task[2] = 0xFF;
			}		
		}
	}
	USART_ClearITPendingBit(USART6, USART_IT_RXNE);
}