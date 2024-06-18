/*
 * FreeModbus Libary: STM32 Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master Functions $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

//如果是485作为主Modbus则要将此位置1
#define RS485_FOR_MASTER_MODBUS 0

#define SerialXForModbus	(1)

#if SerialXForModbus == 1
#define USARTX USART1

#define GPIOX				GPIOA
#define GPIO_PIN_TX	GPIO_Pin_9
#define GPIO_PIN_RX	GPIO_Pin_10
#endif

#if SerialXForModbus == 2
#define USARTX USART2

#define GPIOX				GPIOA
#define GPIO_PIN_TX	GPIO_Pin_2
#define GPIO_PIN_RX	GPIO_Pin_3
#endif

#if SerialXForModbus == 3		//
#define USARTX USART3

#define GPIOX		GPIOD
#define GPIO_PIN_TX	GPIO_Pin_8
#define GPIO_PIN_RX	GPIO_Pin_9
#endif

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED
/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);
/* ----------------------- Start implementation -----------------------------*/

void vMBMasterPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
	if (xRxEnable)
	{
	#if RS485_FOR_MASTER_MODBUS
		MASTER_RS485_RECEIVE_MODE;
	#endif
		USART_ITConfig(USARTX, USART_IT_RXNE, ENABLE);
	}
	else
	{
	#if RS485_FOR_MASTER_MODBUS
		MASTER_RS485_SEND_MODE;
	#endif
		USART_ITConfig(USARTX, USART_IT_RXNE, DISABLE);
	}
	if (xTxEnable)
	{
		USART_ITConfig(USARTX, USART_IT_TXE, ENABLE);
	}
	else
	{
		USART_ITConfig(USARTX, USART_IT_TXE, DISABLE);
	}
}

void vMBMasterPortClose(void)
{
	USART_ITConfig(USARTX, USART_IT_TXE | USART_IT_RXNE, DISABLE);
	USART_Cmd(USARTX, DISABLE);
}
//默认一个主机 串口2 波特率可设置  奇偶检验可设置
BOOL xMBMasterPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
		eMBParity eParity)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//======================时钟初始化=======================================
	#if (SerialXForModbus ==2)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	#endif

	#if (SerialXForModbus ==1)
	  //使能USART1，GPIOA
 	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_USART1, ENABLE);
	#endif

	#if (SerialXForModbus ==3)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOD, ENABLE);
	GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
		/* Enable USART3 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	#endif
	
	//======================IO初始化=======================================	
	//USART2_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;


	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_TX;
	GPIO_Init(GPIOX, &GPIO_InitStructure);
	//USART2_RX

#if ENABLE_BJ
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//针对adm287E重新定义为内置上拉即正常
#else
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
#endif
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_RX;
	GPIO_Init(GPIOX, &GPIO_InitStructure);
	
#if RS485_FOR_MASTER_MODBUS	
	//配置485发送和接收模式
//    TODO   暂时先写B13 等之后组网测试时再修改
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = PIN_RS485_COM;
	GPIO_Init(GPIO_RS485_COM, &GPIO_InitStructure);
#endif	
	//======================串口初始化=======================================
	USART_InitStructure.USART_BaudRate = ulBaudRate;
	//设置校验模式
	switch (eParity)
	{
	case MB_PAR_NONE: //无校验
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		break;
	case MB_PAR_ODD: //奇校验
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		break;
	case MB_PAR_EVEN: //偶校验
		USART_InitStructure.USART_Parity = USART_Parity_Even;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		break;
	default:
		return FALSE;
	}

	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	//if (ucPORT != 2)
	//	return FALSE;

	USART_Init(USARTX, &USART_InitStructure);
	USART_ITConfig(USARTX, USART_IT_RXNE, ENABLE);
	USART_Cmd(USARTX, ENABLE);

	ENTER_CRITICAL_SECTION(); //关全局中断



	//=====================中断初始化======================================
	//设置NVIC优先级分组为Group2：0-3抢占式优先级，0-3的响应式优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
#if (SerialXForModbus ==1)
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
#endif
#if (SerialXForModbus ==2)
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
#endif	
#if (SerialXForModbus ==3)
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
#endif	

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	EXIT_CRITICAL_SECTION(); //开全局中断

	return TRUE;
}

BOOL xMBMasterPortSerialPutByte(CHAR ucByte)
{
//控制发送灯指示.(heat3加热的指示灯)
//	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
	USART_SendData(USARTX, ucByte);
//	GPIO_SetBits(GPIOA,GPIO_Pin_1);
	return TRUE;
}

BOOL xMBMasterPortSerialGetByte(CHAR * pucByte)
{
	*pucByte = USART_ReceiveData(USARTX);
	return TRUE;
}

/* 
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void)
{
	pxMBMasterFrameCBTransmitterEmpty();
}

/* 
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
	pxMBMasterFrameCBByteReceived();
}

#if(SerialXForModbus ==1)
/**
  * @brief  USART1中断服务函数
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{

	rt_interrupt_enter();
		//此句是清除overrun溢出位，避免串口接收中断死机
	if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
	{
	   USART_ReceiveData(USART1);
	} 
	//接收中断
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		prvvUARTRxISR();
	}
	//发送中断
	if (USART_GetITStatus(USART1, USART_IT_TXE) == SET)
	{
		prvvUARTTxReadyISR();
	}
	rt_interrupt_leave();

}

#endif


#if (SerialXForModbus == 2)
/*******************************************************************************
 * Function Name  : USART2_IRQHandler
 * Description    : This function handles USART2 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void USART2_IRQHandler(void)
{
	rt_interrupt_enter();
	//接收中断
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		prvvUARTRxISR();
	}
	//发送中断
	if (USART_GetITStatus(USART2, USART_IT_TXE) == SET)
	{
		prvvUARTTxReadyISR();
	}
	rt_interrupt_leave();
}
#endif

#if (SerialXForModbus == 3)
/*******************************************************************************
 * Function Name  : USART3_IRQHandler
 * Description    : This function handles USART2 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void USART3_IRQHandler(void)
{
	rt_interrupt_enter();
	//接收中断
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		prvvUARTRxISR();
	}
	//发送中断
	if (USART_GetITStatus(USART3, USART_IT_TXE) == SET)
	{
		prvvUARTTxReadyISR();
	}
	rt_interrupt_leave();
}
#endif


#endif
