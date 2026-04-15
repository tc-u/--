#include "stm32f10x.h"                  // Device header
#include"PWM.h"

void Motor_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_4 | GPIO_Pin_5|GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	PWM_Init ();
	
}

void Motor_SetLeftSpeed(float Speed)
{
	if(Speed>=0)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_5);
		GPIO_ResetBits(GPIOA,GPIO_Pin_4);
		PWM_SetCompare1(Speed);
	}
	else
	{		
		GPIO_SetBits(GPIOA,GPIO_Pin_4);
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);
		PWM_SetCompare1(-Speed);
	}
}

void Motor_SetRightSpeed(float Speed)
{
	if(Speed>=0)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_6);
		GPIO_ResetBits(GPIOA,GPIO_Pin_7);
		PWM_SetCompare2(Speed);
	}
	else
	{		
		GPIO_SetBits(GPIOA,GPIO_Pin_7);
		GPIO_ResetBits(GPIOA,GPIO_Pin_6);
		PWM_SetCompare2(-Speed);
	}
}


void Motor_SetSpeed(float Speed1, float Speed2)
{
    // 设置左电机方向
    if(Speed1 >= 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_5);
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        PWM_SetCompare1(Speed1);
    }
    else
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
        PWM_SetCompare1(-Speed1);
    }
    
    // 设置右电机方向
    if(Speed2 >= 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_6);
        GPIO_ResetBits(GPIOA, GPIO_Pin_7);
        PWM_SetCompare2(Speed2);
    }
    else
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_7);
        GPIO_ResetBits(GPIOA, GPIO_Pin_6);
        PWM_SetCompare2(-Speed2);
    }
}
