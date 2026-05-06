#ifndef key_h
#define key_h
#include "stm32f10x.h"
#include "delay.h"


#define key0 1
#define key1 2
#define wk 3

#define k0 GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)
#define k1 GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15)
//#define wk_up GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)



void key_init(void);

int key_scan(void);




#endif


