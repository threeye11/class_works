#include "adc.h"

uint16_t adc_value;

void adc_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	//1.打开时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA, ENABLE);

	//2.初驶化ADC1-CH2---GPIOA2	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//3.初始化 ADC1
	ADC_InitStructure.ADC_ContinuousConvMode=ENABLE;
	ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;
	ADC_InitStructure.ADC_NbrOfChannel=1;
	ADC_InitStructure.ADC_ScanConvMode=ENABLE;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	//4.设置ADC转换时间
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5); 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_55Cycles5); 
	
	//5.使能ADC
	ADC_Cmd(ADC1, ENABLE);
	
	//6.复位校准
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1)!=RESET);
	
	//7.开启校准
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1)!=RESET);
	
	//8.开启转换
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
}

void adc_dma_init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	//1.打开时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	//2.初始化DMA1-ch1
	DMA_InitStructure.DMA_BufferSize=1;
	DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_M2M=DISABLE;
	DMA_InitStructure.DMA_MemoryBaseAddr=(uint32_t)(&adc_value);
	DMA_InitStructure.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryInc=DISABLE;
	DMA_InitStructure.DMA_Mode=DMA_Mode_Circular;
	DMA_InitStructure.DMA_PeripheralBaseAddr=(uint32_t)(&(ADC1->DR));
	DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralInc=DISABLE;
	DMA_InitStructure.DMA_Priority=DMA_Priority_High;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	//3.使能DMA
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	//4.使能ADC1-DMA
	ADC_DMACmd(ADC1, ENABLE);

}

