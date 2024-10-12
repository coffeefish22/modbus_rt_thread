
/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR(void);

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{

	uint16_t PrescalerValue = 0;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//====================================时钟初始化===========================
	//使能定时器3时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	//====================================定时器初始化===========================
	//定时器时间基配置说明
	//HCLK为72MHz，APB1经过2分频为36MHz
	//TIM3的时钟倍频后为72MHz（硬件自动倍频,达到最大）
	//TIM3的分频系数为3599，时间基频率为72 / (1 + Prescaler) = 20KHz,基准为50us
	//TIM最大计数值为usTim1Timerout50u
	
	PrescalerValue = (uint16_t) (SystemCoreClock / 20000) - 1;
	//定时器1初始化
	TIM_TimeBaseStructure.TIM_Period = (uint16_t) usTim1Timerout50us;
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
	//预装载使能
	TIM_ARRPreloadConfig(TIM7, ENABLE);
	//====================================中断初始化===========================
	//设置NVIC优先级分组为Group2：0-3抢占式优先级，0-3的响应式优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//清除溢出中断标志位
	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	//定时器3溢出中断关闭
	TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
	//定时器3禁能
	TIM_Cmd(TIM7, DISABLE);
	return TRUE;
}

void vMBPortTimersEnable()
{
	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM7, 0);
	TIM_Cmd(TIM7, ENABLE);
}

void vMBPortTimersDisable()
{
	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
	TIM_SetCounter(TIM7, 0);
	TIM_Cmd(TIM7, DISABLE);
}

void prvvTIMERExpiredISR(void)
{
	(void) pxMBPortCBTimerExpired();
}

void TIM7_IRQHandler(void)
{
	rt_interrupt_enter();
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
	{
		
		TIM_ClearFlag(TIM7, TIM_FLAG_Update);	     //清中断标记
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);	 //清除定时器T3溢出中断标志位
		prvvTIMERExpiredISR();
	}
	rt_interrupt_leave();
}
