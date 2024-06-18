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
 * File: $Id: porttimer_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master Functions$
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

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
	//====================================ʱ�ӳ�ʼ��===========================
	//ʹ�ܶ�ʱ��2ʱ��
	#if USER_TIM_DEFAULT
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	#else
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);
	#endif
	
	//====================================��ʱ����ʼ��===========================
	//��ʱ��ʱ�������˵��
	//HCLKΪ72MHz��APB1����2��ƵΪ36MHz
	//TIM2��ʱ�ӱ�Ƶ��Ϊ72MHz��Ӳ���Զ���Ƶ,�ﵽ���
	//TIM2�ķ�Ƶϵ��Ϊ3599��ʱ���Ƶ��Ϊ72 / (1 + Prescaler) = 20KHz,��׼Ϊ50us
	//TIM������ֵΪusTim1Timerout50u	
	usPrescalerValue = (uint16_t) (SystemCoreClock / 20000) - 1;
	//����T35��ʱ������ֵ
	usT35TimeOut50us = usTimeOut50us; 

	#if 1
	//Ԥװ��ʹ��
	TIM_ARRPreloadConfig(TIM4, ENABLE);
	//====================================�жϳ�ʼ��===========================
	//����NVIC���ȼ�����ΪGroup2��0-3��ռʽ���ȼ���0-3����Ӧʽ���ȼ�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//�������жϱ�־λ
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	//��ʱ��3����жϹر�
	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
	//��ʱ��3����
	TIM_Cmd(TIM4, DISABLE);
	
	#else
	
	//Ԥװ��ʹ��
	TIM_ARRPreloadConfig(TIM8, ENABLE);
	//====================================�жϳ�ʼ��===========================
	//����NVIC���ȼ�����ΪGroup2��0-3��ռʽ���ȼ���0-3����Ӧʽ���ȼ�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = TIM8_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//�������жϱ�־λ
	TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
	//��ʱ��3����жϹر�
	TIM_ITConfig(TIM8, TIM_IT_Update, DISABLE);
	//��ʱ��3����
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
		
		TIM_ClearFlag(TIM4, TIM_FLAG_Update);	     //���жϱ��
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);	 //�����ʱ��TIM2����жϱ�־λ
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
		
		TIM_ClearFlag(TIM8, TIM_FLAG_Update);	     //���жϱ��
		TIM_ClearITPendingBit(TIM8, TIM_IT_Update);	 //�����ʱ��TIM2����жϱ�־λ
		prvvTIMERExpiredISR();
	}
//	rt_interrupt_leave();
}
#endif

#endif
