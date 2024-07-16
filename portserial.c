//使用到的串口初始化
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

//默认一个从机 串口1 波特率可设置  奇偶检验可设置
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


	//配置485发送和接收模式
	// TODO   暂时先写B13 等之后组网测试时再修改
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	#endif


	#if(USE_SERIAL_X_FOR_SLAVE== 3)
	
	//======================时钟初始化=======================================


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOD, ENABLE);
	GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3,ENABLE);
	
	//======================IO初始化=======================================	
	//USART1_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//USART1_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	//配置485发送和接收模式
//    TODO   暂时先写B13 等之后组网测试时再修改
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	#endif
	
		#if (USE_SERIAL_X_FOR_SLAVE== 0)
	#else
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
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;


	ENTER_CRITICAL_SECTION(); //关全局中断

	USART_Init(USARTX, &USART_InitStructure);
	SLAVE_RS485_RECEIVE_MODE;
	USART_ITConfig(USARTX, USART_IT_RXNE, ENABLE);

	//USART_ClearFlag(USARTX,USART_FLAG_TC);
	USART_Cmd(USARTX, ENABLE);

	//=====================中断初始化======================================
	//设置NVIC优先级分组为Group2：0-3抢占式优先级，0-3的响应式优先级
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

	EXIT_CRITICAL_SECTION(); //开全局中断
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


	//接收中断
	if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(UART4, USART_IT_RXNE);

		
		prvvUARTRxISR();
	}
	//发送中断
	if (USART_GetITStatus(UART4, USART_IT_TXE) == SET)
	{
	//USART_ClearITPendingBit(USART1, USART_IT_TXE);
		prvvUARTTxReadyISR();
	}

//	if(USART_GetITStatus(USART1,USART_IT_TC)!=RESET)
//	{
//		prvvUARTTxReadyISR();
	//	USART_ITConfig(USART1,USART_IT_TC,DISABLE);

		    //清除中断标志
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


	//接收中断
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);

		
		prvvUARTRxISR();
	}
	//发送中断
	if (USART_GetITStatus(USART1, USART_IT_TXE) == SET)
	{
	//USART_ClearITPendingBit(USART1, USART_IT_TXE);
		prvvUARTTxReadyISR();
	}

//	if(USART_GetITStatus(USART1,USART_IT_TC)!=RESET)
//	{
//		prvvUARTTxReadyISR();
	//	USART_ITConfig(USART1,USART_IT_TC,DISABLE);

		    //清除中断标志
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

	//此句是清除overrun溢出位，避免串口接收中断死机
	if(USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
	{
	   USART_ReceiveData(USART3);
	} 
	
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

	//此句是清除overrun溢出位，避免串口接收中断死机
	if(USART_GetFlagStatus(UART5, USART_FLAG_ORE) != RESET)
	{
	   USART_ReceiveData(UART5);
	} 

	//接收中断
	if (USART_GetITStatus(UART5, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(UART5, USART_IT_RXNE);

		
		prvvUARTRxISR();
	}
	//发送中断
	if (USART_GetITStatus(UART5, USART_IT_TXE) == SET)
	{
	//USART_ClearITPendingBit(USART1, USART_IT_TXE);
		prvvUARTTxReadyISR();
	}

//	if(USART_GetITStatus(USART1,USART_IT_TC)!=RESET)
//	{
//		prvvUARTTxReadyISR();
	//	USART_ITConfig(USART1,USART_IT_TC,DISABLE);

		    //清除中断标志
//   USART_ClearITPendingBit(USART1, USART_IT_TC);
//	}
	
	rt_interrupt_leave();
}

#endif


