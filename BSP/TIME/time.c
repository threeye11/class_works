#include "time.h"

void time_init(uint16_t arr,uint16_t psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	
	NVIC_InitTypeDef NVIC_InitStructure;	
	
	//1.打开TIM时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	//2.初始化tim
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=arr-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc-1;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	
	//3.初始化NVIC
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	//4.配置中断更新
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
	//5.打开定时器
	TIM_Cmd(TIM3, ENABLE);
	
}