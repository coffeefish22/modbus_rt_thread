/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
//定时器需要根据需求配置
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbport.h"

#define USER_TIM_DEFAULT	1

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED
/* ----------------------- Variables ----------------------------------------*/
static USHORT usT35TimeOut50us;
static USHORT usPrescalerValue = 0;

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR(void);

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBMasterPortTimersInit(USHORT usTimeOut50us)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	//====================================时钟初始化===========================
	//使能定时器2时钟
	#if USER_TIM_DEFAULT
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	#else
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);
	#endif
	
	//====================================定时器初始化===========================
	//定时器时间基配置说明
	//HCLK为72MHz，APB1经过2分频为36MHz
	//TIM2的时钟倍频后为72MHz（硬件自动倍频,达到最大）
	//TIM2的分频系数为3599，时间基频率为72 / (1 + Prescaler) = 20KHz,基准为50us
	//TIM最大计数值为usTim1Timerout50u	
	usPrescalerValue = (uint16_t) (SystemCoreClock / 20000) - 1;
	//保存T35定时器计数值
	usT35TimeOut50us = usTimeOut50us; 

	#if 1
	//预装载使能
	TIM_ARRPreloadConfig(TIM4, ENABLE);
	//====================================中断初始化===========================
	//设置NVIC优先级分组为Group2：0-3抢占式优先级，0-3的响应式优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//清除溢出中断标志位
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	//定时器3溢出中断关闭
	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
	//定时器3禁能
	TIM_Cmd(TIM4, DISABLE);
	
	#else
	
	//预装载使能
	TIM_ARRPreloadConfig(TIM8, ENABLE);
	//====================================中断初始化===========================
	//设置NVIC优先级分组为Group2：0-3抢占式优先级，0-3的响应式优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = TIM8_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//清除溢出中断标志位
	TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
	//定时器3溢出中断关闭
	TIM_ITConfig(TIM8, TIM_IT_Update, DISABLE);
	//定时器3禁能
	TIM_Cmd(TIM8, DISABLE);

	#endif
	
	return TRUE;
}

void vMBMasterPortTimersT35Enable()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* Set current timer mode,don't change it.*/
	vMBMasterSetCurTimerMode(MB_TMODE_T35);

	TIM_TimeBaseStructure.TIM_Prescaler = usPrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = (uint16_t) usT35TimeOut50us;
	
	#if USER_TIM_DEFAULT
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM4, 0);
	TIM_Cmd(TIM4, ENABLE);
	#else
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

	TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
	TIM_ITConfig(TIM8, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM8, 0);
	TIM_Cmd(TIM8, ENABLE);
	#endif
	
}

void vMBMasterPortTimersConvertDelayEnable()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* Set current timer mode,don't change it.*/
	vMBMasterSetCurTimerMode(MB_TMODE_CONVERT_DELAY);

	TIM_TimeBaseStructure.TIM_Prescaler = usPrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = (uint16_t)(MB_MASTER_DELAY_MS_CONVERT * 1000 / 50);

	#if USER_TIM_DEFAULT
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM4, 0);
	TIM_Cmd(TIM4, ENABLE);
	#else
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

	TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
	TIM_ITConfig(TIM8, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM8, 0);
	TIM_Cmd(TIM8, ENABLE);
	#endif
	
}

void vMBMasterPortTimersRespondTimeoutEnable()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* Set current timer mode,don't change it.*/
	vMBMasterSetCurTimerMode(MB_TMODE_RESPOND_TIMEOUT);

	TIM_TimeBaseStructure.TIM_Prescaler = usPrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = (uint16_t)(MB_MASTER_TIMEOUT_MS_RESPOND * 1000 / 50);
	#if USER_TIM_DEFAULT
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM4, 0);
	TIM_Cmd(TIM4, ENABLE);
	#else
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

	TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
	TIM_ITConfig(TIM8, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM8, 0);
	TIM_Cmd(TIM8, ENABLE);
	#endif
}

void vMBMasterPortTimersDisable()
{
#if USER_TIM_DEFAULT
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
	TIM_SetCounter(TIM4, 0);
	TIM_Cmd(TIM4, DISABLE);
#else
	TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
	TIM_ITConfig(TIM8, TIM_IT_Update, DISABLE);
	TIM_SetCounter(TIM8, 0);
	TIM_Cmd(TIM8, DISABLE);
#endif
}

void prvvTIMERExpiredISR(void)
{
	(void) pxMBMasterPortCBTimerExpired();
}

#if USER_TIM_DEFAULT

void TIM4_IRQHandler(void)
{
	rt_interrupt_enter();
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		
		TIM_ClearFlag(TIM4, TIM_FLAG_Update);	     //清中断标记
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);	 //清除定时器TIM2溢出中断标志位
		prvvTIMERExpiredISR();
	}
	rt_interrupt_leave();
}

#else

void TIM8_UP_IRQHandler(void)
{
//	rt_interrupt_enter();
	if (TIM_GetITStatus(TIM8, TIM_IT_Update) != RESET)
	{
		
		TIM_ClearFlag(TIM8, TIM_FLAG_Update);	     //清中断标记
		TIM_ClearITPendingBit(TIM8, TIM_IT_Update);	 //清除定时器TIM2溢出中断标志位
		prvvTIMERExpiredISR();
	}
//	rt_interrupt_leave();
}
#endif

#endif
