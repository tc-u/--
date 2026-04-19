#include "stm32f10x.h"                  // Device header
#include"Delay.h"

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStrucure;
	GPIO_InitStrucure.GPIO_Mode =GPIO_Mode_IPU;
	GPIO_InitStrucure.GPIO_Pin =GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStrucure.GPIO_Speed =GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStrucure);
	
	GPIO_InitStrucure.GPIO_Pin =GPIO_Pin_15;
	GPIO_Init(GPIOB,&GPIO_InitStrucure);
	
}

uint8_t Key_GetNum(void)
{
	uint8_t KeyNum=0;
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)==0)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)==0);
		Delay_ms(20);
		KeyNum=1;
	}
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12)==0)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12)==0);
		Delay_ms(20);
		KeyNum=2;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==0)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==0);
		Delay_ms(20);
		KeyNum=3;
	}
	return KeyNum;
}