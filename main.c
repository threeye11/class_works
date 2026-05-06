#include "stm32f10x.h"
#include "led.h"
#include "key.h"
#include "core_delay.h"
#include "bsp_usart1.h"
#include "lcd.h"
#include "24cxx.h" 
#include "myiic.h"
#include "touch.h" 
#include "time.h"
#include "adc.h"
#include "ds18b20.h"
#include "spi.h"
#include "24l01.h" 
#include "bsp_SysTick.h"
#include "bsp_esp8266.h"
#include "bsp_esp8266_test.h"

#include "my_gui.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"


int main()
{

	CPU_TS_TmrInit();
	key_init();
	USART1_Config();
	time_init(10,7200);
	adc_init();
	adc_dma_init();
	NRF24L01_Init();
	
	lv_init(); 
	lv_port_disp_init(); 
	lv_port_indev_init();

	ESP8266_Init ();
	
//	while(DS18B20_Init())	//DS18B20初始化	
//	{
//		LCD_ShowString(60,130,200,16,16,"DS18B20 Error");
//		delay_ms(200);
//		printf("ds18b20 error!\r\n");
// 		delay_ms(200);
//	}
//	printf("ds18b20 OK!\r\n");	
			
 	while(NRF24L01_Check())	//检查NRF24L01是否在位.	
	{
		printf("NRF24L01 Error\r\n");
		delay_ms(200);
	}
	printf("NRF24L01 OK!!!\r\n");
	NRF24L01_RX_Mode();

  ESP8266_StaTcpClient_Unvarnish_ConfigTest();
 	
	my_gui();	
	while(1)
	{
		delay_ms(5);
//		led0_control(on);
//		delay_ms(500);
//		led0_control(off);
//		delay_ms(500);
		lv_timer_handler();

//		if(key0==key_scan())	//KEY0按下,则执行校准程序
//		{
//		  TP_Adjust();  //屏幕校准 
//			TP_Save_Adjdata();	 
//		}
	}	
		
	
	
	
	return 0;
}

