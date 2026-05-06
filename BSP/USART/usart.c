#include "usart.h"


void usart_init(u32 BRT)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;	
	//1.打开GPIO、usart1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1, ENABLE);
	
	//2.初始化GPI0
	//初始化PA9---USART1-TX
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//初始化PA9---USART1-RX
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//3.usart1初始化
	USART_InitStructure.USART_BaudRate=BRT;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);
	
	//4.初始化NVIC	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//5.配置串口中断触发模式
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	
	//6.使能串口
	USART_Cmd(USART1, ENABLE);
}

void lyeusart_send(uint16_t tep)
{
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_SendData(USART1, tep);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC)!=SET);
}


int fputc(int ch,FILE *f)
{
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_SendData(USART1, (uint16_t)ch);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC)!=SET);	
	return ch;
}

