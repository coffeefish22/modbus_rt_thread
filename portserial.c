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
 * File: $Id: portserial.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#include "rtthread.h"

#include "myconfig.h"


#if (USE_SERIAL_X_FOR_SLAVE== 4)
#define USARTX UART4
#define UART4_GPIO_TX	GPIO_Pin_10
#define UART4_GPIO_RX	GPIO_Pin_11
#define UART4_GPIO			GPIOC
#endif

#if(USE_SERIAL_X_FOR_SLAVE== 3)
#define USARTX USART3
#define GPIOX		GPIOD
#define GPIO_PIN_TX	GPIO_Pin_8
#define GPIO_PIN_RX	GPIO_Pin_9
#endif

#if (USE_SERIAL_X_FOR_SLAVE== 0)

#define USARTX UART5

#endif



#if(USE_SERIAL_X_FOR_SLAVE== 5)

#define USARTX 	UART5
#define GPIOTX		GPIOC
#define GPIORX		GPIOD
#define GPIO_PIN_TX	GPIO_Pin_12
#define GPIO_PIN_RX	GPIO_Pin_2
#endif

#if (USE_SERIAL_X_FOR_SLAVE== 0)
#else
/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);
/* ----------------------- Start implementation -----------------------------*/
#endif


void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
	extern void rt_hw_us_delay(int us);
	if (xRxEnable)
	{
		SLAVE_RS485_RECEIVE_MODE;
		USART_ITConfig(USARTX, USART_IT_RXNE, ENABLE);
		
	}
	else
	{
		SLAVE_RS485_SEND_MODE;
		
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

void vMBPortClose(void)
{
	USART_ITConfig(USARTX, USART_IT_TXE | USART_IT_RXNE, DISABLE);
	USART_Cmd(USARTX, DISABLE);
}

//Ĭ��һ���ӻ� ����1 �����ʿ�����  ��ż���������
BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
		eMBParity eParity)
{
	#if (USE_SERIAL_X_FOR_SLAVE== 0)
	#else
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	#endif

	#if (USE_SERIAL_X_FOR_SLAVE== 4)
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = UART4_GPIO_RX;
	GPIO_Init(UART4_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = UART4_GPIO_TX;
	GPIO_Init(UART4_GPIO, &GPIO_InitStructure);

	#endif

	#if (USE_SERIAL_X_FOR_SLAVE== 5)
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_RX;
	GPIO_Init(GPIORX, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_TX;
	GPIO_Init(GPIOTX, &GPIO_InitStructure);


	//����485���ͺͽ���ģʽ
	// TODO   ��ʱ��дB13 ��֮����������ʱ���޸�
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	#endif


	#if(USE_SERIAL_X_FOR_SLAVE== 3)
	
	//======================ʱ�ӳ�ʼ��=======================================


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOD, ENABLE);
	GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3,ENABLE);
	
	//======================IO��ʼ��=======================================	
	//USART1_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//USART1_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	//����485���ͺͽ���ģʽ
//    TODO   ��ʱ��дB13 ��֮����������ʱ���޸�
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	#endif
	
		#if (USE_SERIAL_X_FOR_SLAVE== 0)
	#else
	//======================���ڳ�ʼ��=======================================
	USART_InitStructure.USART_BaudRate = ulBaudRate;
	//����У��ģʽ
	switch (eParity)
	{
	case MB_PAR_NONE: //��У��
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		break;
	case MB_PAR_ODD: //��У��
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		break;
	case MB_PAR_EVEN: //żУ��
		USART_InitStructure.USART_Parity = USART_Parity_Even;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		break;
	default:
		return FALSE;
	}

	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;


	ENTER_CRITICAL_SECTION(); //��ȫ���ж�

	USART_Init(USARTX, &USART_InitStructure);
	SLAVE_RS485_RECEIVE_MODE;
	USART_ITConfig(USARTX, USART_IT_RXNE, ENABLE);

	//USART_ClearFlag(USARTX,USART_FLAG_TC);
	USART_Cmd(USARTX, ENABLE);

	//=====================�жϳ�ʼ��======================================
	//����NVIC���ȼ�����ΪGroup2��0-3��ռʽ���ȼ���0-3����Ӧʽ���ȼ�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	#if (USE_SERIAL_X_FOR_SLAVE== 5)
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	#endif
	#if (USE_SERIAL_X_FOR_SLAVE== 4)
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	#endif
	#if(USE_SERIAL_X_FOR_SLAVE== 3)
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	#endif
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	EXIT_CRITICAL_SECTION(); //��ȫ���ж�
	#endif
	return TRUE;
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
	USART_SendData(USARTX, ucByte);
	return TRUE;
}

BOOL xMBPortSerialGetByte(CHAR * pucByte)
{
	*pucByte = USART_ReceiveData(USARTX);
	//rt_kprintf("%c  ",*pucByte);
	return TRUE;
}

	#if (USE_SERIAL_X_FOR_SLAVE== 0)
	#else
/* 
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void)
{
	pxMBFrameCBTransmitterEmpty();
}

/* 
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
	pxMBFrameCBByteReceived();
}
#endif

#if (USE_SERIAL_X_FOR_SLAVE== 4)
/*******************************************************************************
 * Function Name  : USART1_IRQHandler
 * Description    : This function handles USART1 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void UART4_IRQHandler(void)
{
	rt_interrupt_enter();


	//�����ж�
	if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(UART4, USART_IT_RXNE);

		
		prvvUARTRxISR();
	}
	//�����ж�
	if (USART_GetITStatus(UART4, USART_IT_TXE) == SET)
	{
	//USART_ClearITPendingBit(USART1, USART_IT_TXE);
		prvvUARTTxReadyISR();
	}

//	if(USART_GetITStatus(USART1,USART_IT_TC)!=RESET)
//	{
//		prvvUARTTxReadyISR();
	//	USART_ITConfig(USART1,USART_IT_TC,DISABLE);

		    //����жϱ�־
//   USART_ClearITPendingBit(USART1, USART_IT_TC);
//	}
	
	rt_interrupt_leave();
}

#endif

#if (USE_SERIAL_X_FOR_SLAVE== 1)
/*******************************************************************************
 * Function Name  : USART1_IRQHandler
 * Description    : This function handles USART1 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void USART1_IRQHandler(void)
{
	rt_interrupt_enter();


	//�����ж�
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);

		
		prvvUARTRxISR();
	}
	//�����ж�
	if (USART_GetITStatus(USART1, USART_IT_TXE) == SET)
	{
	//USART_ClearITPendingBit(USART1, USART_IT_TXE);
		prvvUARTTxReadyISR();
	}

//	if(USART_GetITStatus(USART1,USART_IT_TC)!=RESET)
//	{
//		prvvUARTTxReadyISR();
	//	USART_ITConfig(USART1,USART_IT_TC,DISABLE);

		    //����жϱ�־
//   USART_ClearITPendingBit(USART1, USART_IT_TC);
//	}
	
	rt_interrupt_leave();
}
#endif

#if (USE_SERIAL_X_FOR_SLAVE == 3)

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

	//�˾������overrun���λ�����⴮�ڽ����ж�����
	if(USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
	{
	   USART_ReceiveData(USART3);
	} 
	
	//�����ж�
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		prvvUARTRxISR();
	}
	//�����ж�
	if (USART_GetITStatus(USART3, USART_IT_TXE) == SET)
	{
		prvvUARTTxReadyISR();
	}
	rt_interrupt_leave();
}
#endif


#if (USE_SERIAL_X_FOR_SLAVE== 5)
/*******************************************************************************
 * Function Name  : USART1_IRQHandler
 * Description    : This function handles USART1 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void UART5_IRQHandler(void)
{
	rt_interrupt_enter();

	//�˾������overrun���λ�����⴮�ڽ����ж�����
	if(USART_GetFlagStatus(UART5, USART_FLAG_ORE) != RESET)
	{
	   USART_ReceiveData(UART5);
	} 

	//�����ж�
	if (USART_GetITStatus(UART5, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(UART5, USART_IT_RXNE);

		
		prvvUARTRxISR();
	}
	//�����ж�
	if (USART_GetITStatus(UART5, USART_IT_TXE) == SET)
	{
	//USART_ClearITPendingBit(USART1, USART_IT_TXE);
		prvvUARTTxReadyISR();
	}

//	if(USART_GetITStatus(USART1,USART_IT_TC)!=RESET)
//	{
//		prvvUARTTxReadyISR();
	//	USART_ITConfig(USART1,USART_IT_TC,DISABLE);

		    //����жϱ�־
//   USART_ClearITPendingBit(USART1, USART_IT_TC);
//	}
	
	rt_interrupt_leave();
}

#endif


