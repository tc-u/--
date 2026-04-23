#include "stm32f10x.h"                  // Device header
#include"Delay.h"
#include"OLED.h"
#include"Motor.h" 
#include"Track.h"
#include"Encoder.h"
#include"Timer.h"
#include"Buzzer.h"
#include"LED.h"
#include"JY62.h"
#include"Key.h"

//uint8_t KeyNum;
//int16_t LSpeed;
//int16_t RSpeed;

// 行驶状态枚举
typedef enum {
    STATE_START,        // 起始状态          0
    STATE_A_TO_B,       // A点到B点          1
    STATE_B_TO_C,       // B点到C点（圆弧）  2
    STATE_C_TO_D,       // C点到D点          3
    STATE_D_TO_A,       // D点到A点（圆弧）	 4
    STATE_STOP          // 停止状态          5
} CarState;

CarState currentState = STATE_START;    // 小车当前状态
uint8_t pointCounter = 0;  		 	  // 经过点的计数器

uint16_t WhiteDetectCount=0;

uint8_t Flag_LED_ON=0;
uint8_t Flag_LED_OFF=0;
uint8_t Flag_Buzzer_ON=0;
uint8_t Flag_Buzzer_OFF=0; 

uint8_t Flag_zhi=1;
uint8_t Flag_xun=0;
uint8_t Flag_rotate=0;
				
//速度环参数
float Target1,Target2,Actual1,Actual2,Out1,Out2;
float Kp_su=10,Ki_su=2,Kd_su=0;
float ErrorNow1,ErrorNow2,ErrorLast1,ErrorLast2,ErrorSum1,ErrorSum2;
//循迹环参数
float Out_xun;
float Kp_xun=350*0.6,Ki_xun=4,Kd_xun=550;
float ErrorNow_xun,ErrorLast_xun,ErrorSum_xun;
 
int main(void)
{
	OLED_Init();
	Motor_Init();
	Track_Init();
	Timer_Init();
	Encoder_Init();
	Buzzer_Init();
	LED_Init();
	JY62_Init();
	Key_Init();
	
	OLED_ShowString(1,1,"LSpeed:");
	OLED_ShowString(2,1,"RSpeed:");
	OLED_ShowString(3,1,"Yaw:");
	OLED_ShowString(4,1,"State:");
		
	while(1)
	{	
		uint8_t key = Key_GetNum();

		OLED_ShowSignedNum(1,8,Actual1,3);
		OLED_ShowSignedNum(2,8,Actual2,3);
		
		// 更新JY62数据
		JY62_UpdateData();
		
		// 显示当前偏航角
		OLED_ShowFloat(3,5,JY62_GetYaw(),2);
		
		// 显示当前状态
        switch(currentState)
        {
            case STATE_START:
                OLED_ShowString(4,7,"START");
                break;
            case STATE_A_TO_B:
                OLED_ShowString(4,7,"A->B  ");
                break;
            case STATE_B_TO_C:
                OLED_ShowString(4,7,"B->C  ");
                break;
            case STATE_C_TO_D:
                OLED_ShowString(4,7,"C->D  ");
                break;
            case STATE_D_TO_A:
                OLED_ShowString(4,7,"D->A  ");
                break;
            case STATE_STOP:
                OLED_ShowString(4,7,"STOP  ");
                break;
        }

		
		
//		Motor_SetLeftSpeed (600);
//		Motor_SetRightSpeed (600);
//		Motor_SetSpeed (1000,1000);
//		Track_Task();
		
		Target1=60;
		Target2=60;

		// 测试旋转功能
		if(Flag_rotate==1)
		{
			Flag_rotate=0;
			LED_ON();
			JY62_RotateToAngle(90);  // 旋转90度
			LED_OFF();
		}

		// 声光提示
		if(Flag_LED_ON==1)
		{
			LED_ON();
			Delay_ms(500);
			LED_OFF();
			Flag_LED_ON=0;
		}
		if(Flag_LED_OFF==1)
		{
			LED_OFF();
			Flag_LED_OFF=0;
		}
		if(Flag_Buzzer_ON==1)
		{
			Buzzer_ON();
			Delay_ms(500);
			Buzzer_OFF();
			Flag_Buzzer_ON=0;
		}
		if(Flag_Buzzer_OFF==1)
		{
			Buzzer_OFF();
			Flag_Buzzer_OFF=0;
		}
	}
}

void TIM3_IRQHandler()//10ms
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		if(Flag_zhi==1)//速度环
		{
			Actual1 = Encoder_GetLeft();
			Actual2 = Encoder_GetRight();
		
			ErrorLast1=ErrorNow1;
			ErrorLast2=ErrorNow2;
		
			ErrorNow1=Target1-Actual1;
			ErrorNow2=Target2-Actual2;
		
			ErrorSum1+=ErrorNow1;
			ErrorSum2+=ErrorNow2;

			Out1=Kp_su*ErrorNow1 +Ki_su*ErrorSum1 +Kd_su*(ErrorNow1-ErrorLast1);
			Out2=Kp_su*ErrorNow2 +Ki_su*ErrorSum2 +Kd_su*(ErrorNow2-ErrorLast2);
		
			if(Out1>1000){Out1=1000;}
			if(Out2>1000){Out2=1000;}
			if(Out1<0){Out1=0;}
			if(Out2<0){Out2=0;}

			Motor_SetLeftSpeed(Out1);
			Motor_SetRightSpeed(Out2);
			
			if(Track_ReadAll()!=0xFF)//判定是否识别到黑线
			{
				Flag_zhi=0;
				Flag_xun=1;
				Flag_LED_ON =1;
				Flag_Buzzer_ON =1;				
			}
		}
		
		if(Flag_xun==1)//循迹环
		{
			ErrorNow_xun = Track_GetError();
        
        // 计算积分项
        ErrorSum_xun += ErrorNow_xun; 
        
        // 计算微分项
        float ErrorDiff_xun = ErrorNow_xun - ErrorLast_xun;
        
        // PID计算
        Out_xun = Kp_xun * ErrorNow_xun + Ki_xun* ErrorSum_xun+ Kd_xun * ErrorDiff_xun;
        
        // 限制PID输出范围
        if (Out_xun > 300) Out_xun = 300;
        if (Out_xun < -300) Out_xun = -300;
        
        // 计算左右电机速度
        int16_t leftSpeed = 300 - Out_xun;
        int16_t rightSpeed = 300 + Out_xun;
        
        // 限制速度范围
        if (leftSpeed > 1000) leftSpeed = 1000;
        if (leftSpeed < 0) leftSpeed = 0;
        if (rightSpeed > 1000) rightSpeed = 1000;
        if (rightSpeed < 0) rightSpeed = 0;
        
        // 控制电机
        Motor_SetLeftSpeed(leftSpeed);
        Motor_SetRightSpeed(rightSpeed);
        
        // 更新上一次误差
        ErrorLast_xun = ErrorNow_xun;
		  
		  	if(Track_ReadAll()==0x00)//判定是否识别到全白
			{
				WhiteDetectCount++;
				if(WhiteDetectCount>3)//判断是否已经识别到全白
				{
					WhiteDetectCount=0;
					Flag_xun=0;
					Flag_zhi=1;
					Flag_LED_OFF =1;
					Flag_Buzzer_OFF =1;	
				}
			}

		}
		
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}
