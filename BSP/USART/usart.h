#ifndef usart_h
#define usart_h
#include "stm32f10x.h"
#include <stdio.h>

void usart_init(u32 BRT);

void lyeusart_send(uint16_t);

int fputc(int ch,FILE *f);
#endif