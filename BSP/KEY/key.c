#include "key.h"


void key_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
//	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPD;
//	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
}


int key_scan(void)
{
		int key_value=0;
/*°æ±¾1
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)==0)
		{
			delay_ms(20);
			if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)==0)
				key_value=key0;
		}
		
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15)==0)
		{
			delay_ms(20);
			if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15)==0)
				key_value=key1;
		}
		
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)==1)
		{
			delay_ms(20);
			if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)==1)
				key_value=wk;
		}	
*/
//°æ±¾2
		if(k1==0||k1==0)
		{
			delay_ms(20);
			if(k0==0)
				key_value=key0;
			if(k1==0)
				key_value=key1;
		}
		return key_value;
}