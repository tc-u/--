#include "stm32f10x.h"                  // Device header

void LightSensor_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStrucure;
	GPIO_InitStrucure.GPIO_Mode =GPIO_Mode_IPU;
	GPIO_InitStrucure.GPIO_Pin =GPIO_Pin_13;
	GPIO_InitStrucure.GPIO_Speed =GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStrucure);

}

uint8_t LightSensor_GetNum(void)
{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13);
}
