#ifndef led_h
#define led_h
#include "stm32f10x.h"

typedef enum
{
	off=0,
	on=!off
}led_state;

//#define off 0
//#define on 1


void led_init(void);


void led0_control(led_state n);

void led1_control(led_state n);


#endif

