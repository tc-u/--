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

typedef enum {
    STATE_START_2,        // 起始状态          0
	STATE_ROTATE_N45_2,       // 旋转至-45       1
    STATE_A_TO_C1_2,       // A点到C1点          2
	STATE_ROTATE_0_2,       // 旋转至0          3
	STATE_C1_TO_C2_2,       // C1点到C2点          4
    STATE_C2_TO_B_2,       // C2点到B点（圆弧）  5
	STATE_ROTATE_N135_2,       // 旋转至-135       6
    STATE_B_TO_D1_2,       // B点到D1点          7
	STATE_ROTATE_180_2,       // 旋转至-90       8
	STATE_D1_TO_D2_2,       // D1点到D2点          9
    STATE_D2_TO_A_2,       // D2点到A点（圆弧）	10
    STATE_STOP_2          // 停止状态          11
} CarState2;

CarState currentState = STATE_START;    // 小车当前状态
CarState2 currentState2 = STATE_START_2;    // 小车当前状态2

uint8_t ApointCounter = 0;  		 	  // 经过点的计数器

uint16_t WhiteDetectCount=0;

int32_t encoderCount = 0;

uint8_t Mode=1;
uint8_t LapCount=0;

uint8_t initialYawRecorded = 0; // 是否记录了初始偏航角
float initialYaw = 0.0f; // 初始偏航角


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
	
	OLED_ShowString(1,1,"Mode:");
	OLED_ShowString(2,1,"Lap:");
	OLED_ShowString(3,1,"Yaw:");
	OLED_ShowString(4,1,"State:");

	Target1=30;
	Target2=30;	

	while(1)
	{	
		uint8_t key = Key_GetNum();

		// 按键1控制Mode切换
		if(key == 1)
		{
			// 在Mode 1和2之间切换
			if(Mode == 1)
			{
				Mode = 2;
			}
			else
			{
				Mode = 1;
			}
		}
		OLED_ShowSignedNum(1,6,Mode,1);

		// 按键2控制LapCount增加
		if(key == 2)
		{
			LapCount++;
			if(LapCount>3)
			{
				LapCount = 1;
			}	
		}
		OLED_ShowSignedNum(2,5,LapCount,1);
		
		// 更新JY62数据
		JY62_UpdateData();		
		// 显示当前偏航角
		OLED_ShowFloat(3,5,JY62_GetYaw(),2);
		
		// 显示当前状态
		if(Mode == 1)
		{
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
		}
		
		if(Mode == 2)
		{
			switch(currentState2)
			{
				case STATE_START_2:
					OLED_ShowString(4,7,"START");
					break;
				case STATE_ROTATE_N45_2:
					OLED_ShowString(4,7,"ROTATE_N45");
					break;
				case STATE_A_TO_C1_2:
					OLED_ShowString(4,7,"A->C1  ");
					break;
				case STATE_ROTATE_0_2:
					OLED_ShowString(4,7,"ROTATE_0");
					break;
				case STATE_C1_TO_C2_2:
					OLED_ShowString(4,7,"C1->C2  ");
					break;
				case STATE_C2_TO_B_2:
					OLED_ShowString(4,7,"C2->B  ");
					break;
				case STATE_ROTATE_N135_2:
					OLED_ShowString(4,7,"ROTATE_N135");
					break;
				case STATE_B_TO_D1_2:
					OLED_ShowString(4,7,"B->D1  ");
					break;
				case STATE_ROTATE_180_2:
					OLED_ShowString(4,7,"ROTATE_180");
					break;
				case STATE_D1_TO_D2_2:
					OLED_ShowString(4,7,"D1->D2  ");
					break;
				case STATE_D2_TO_A_2:
					OLED_ShowString(4,7,"D2->A  ");
					break;
				case STATE_STOP_2:
					OLED_ShowString(4,7,"STOP  ");
					break;
			}
		}
 
		// 测试旋转功能
		if(Flag_rotate==1)
		{
			Flag_rotate=0;
			LED_ON();
			JY62_RotateToAngle(90);  // 旋转30度
			LED_OFF();
		}

		// 启动按钮控制
        if(initialYawRecorded == 0&&key==3) // 按键3作为启动按钮
        {
            // 记录初始角度
            JY62_UpdateData();
            initialYaw = JY62_GetYaw();
            initialYawRecorded = 1;
            if(Mode == 1)
            {
                currentState = STATE_A_TO_B;
            }
            if(Mode == 2)
            {
                currentState2 = STATE_ROTATE_N45_2;
            }
            ApointCounter = 0;
        }

		// 声光提示
		if(Flag_LED_ON==1 && Flag_Buzzer_ON==1)
		{
			LED_ON();
			Buzzer_ON();
			Delay_ms(300);
			LED_OFF();
			Buzzer_OFF();
			Flag_LED_ON=0;
			Flag_Buzzer_ON=0;
		}

		if(Flag_LED_OFF==1)
		{
			LED_OFF();
			Flag_LED_OFF=0;
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
		// 在switch语句外部声明变量
		float ErrorDiff_xun;
		int16_t leftSpeed;
		int16_t rightSpeed;

		float currentYaw;
		float angleError;
		float angleOut;
		float angleTarget;

		float Kp_angle;
		float Ki_angle;
		float Kd_angle;
		static float angleErrorSum;
		static float lastAngleError;
		float angleErrorDiff;

		int baseSpeed;

		JY62_UpdateData();

		// 处理Mode 1的状态
		if(Mode == 1)
		{
			switch(currentState)
			{
				case STATE_START: 
					break;
				case STATE_A_TO_B://角度环
				case STATE_C_TO_D://角度环
					if(currentState == STATE_A_TO_B)//A->B
					{
						angleTarget = 0.0f;
					}
					else//C->D
					{
						angleTarget = 180.0f;
					}
					currentYaw = JY62_GetYaw();
					angleError = angleTarget - currentYaw;

					// 角度误差归一化到-180~180度
					if (angleError > 180) {
						angleError -= 360;
					} else if (angleError < -180) {
						angleError += 360;
					}

					// 角度环PID参数
					Kp_angle = 10.0f;
					Ki_angle = 0.1f;
					Kd_angle = 1.0f;
					angleErrorSum = 0.0f;
					lastAngleError = 0.0f;

					angleErrorSum += angleError;
					angleErrorDiff = angleError - lastAngleError;

					angleOut = Kp_angle * angleError + Ki_angle * angleErrorSum + Kd_angle * angleErrorDiff;

					// 限制角度环输出范围
					if (angleOut > 300) angleOut = 300;
					if (angleOut < -300) angleOut = -300;

					// 基础速度
					baseSpeed = 300;

					// 计算左右电机速度
					leftSpeed = baseSpeed - angleOut;
					rightSpeed = baseSpeed + angleOut;

					// 限制速度范围
					if (leftSpeed > 1000) leftSpeed = 1000;
					if (leftSpeed < 0) leftSpeed = 0;
					if (rightSpeed > 1000) rightSpeed = 1000;
					if (rightSpeed < 0) rightSpeed = 0;

					// 控制电机
					Motor_SetLeftSpeed(leftSpeed);
					Motor_SetRightSpeed(rightSpeed);
					
					// 更新上一次角度误差
					lastAngleError = angleError;

					if(Track_ReadAll()!=0x00)//判定是否识别到黑线
					{
						if(currentState==STATE_A_TO_B)
						{
							currentState =STATE_B_TO_C;
						}
						else
						{
							currentState = STATE_D_TO_A ;
						}
						Flag_LED_ON =1;
						Flag_Buzzer_ON =1;				
					}
					break;

					// Actual1 = Encoder_GetLeft();
					// Actual2 = Encoder_GetRight();
				
					// ErrorLast1=ErrorNow1;
					// ErrorLast2=ErrorNow2;
				
					// ErrorNow1=Target1-Actual1;
					// ErrorNow2=Target2-Actual2;
				
					// ErrorSum1+=ErrorNow1;
					// ErrorSum2+=ErrorNow2;

					// Out1=Kp_su*ErrorNow1 +Ki_su*ErrorSum1 +Kd_su*(ErrorNow1-ErrorLast1);
					// Out2=Kp_su*ErrorNow2 +Ki_su*ErrorSum2 +Kd_su*(ErrorNow2-ErrorLast2);
				
					// if(Out1>1000){Out1=1000;}
					// if(Out2>1000){Out2=1000;}
					// if(Out1<0){Out1=0;}
					// if(Out2<0){Out2=0;}

					// Motor_SetLeftSpeed(Out1);
					// Motor_SetRightSpeed(Out2);
					
					// if(Track_ReadAll()!=0x00)//判定是否识别到黑线
					// {
					// 	if(currentState==STATE_A_TO_B)
					// 	{
					// 		currentState =STATE_B_TO_C;
					// 	}
					// 	else
					// 	{
					// 		currentState = STATE_D_TO_A ;
					// 	}
					// 	Flag_LED_ON =1;
					// 	Flag_Buzzer_ON =1;				
					// }
					// break;

				case STATE_B_TO_C://循迹环
				case STATE_D_TO_A://循迹环
					ErrorNow_xun = Track_GetError();
		
					// 计算积分项
					ErrorSum_xun += ErrorNow_xun; 
					
					// 计算微分项
					ErrorDiff_xun = ErrorNow_xun - ErrorLast_xun;
					
					// PID计算
					Out_xun = Kp_xun * ErrorNow_xun + Ki_xun* ErrorSum_xun+ Kd_xun * ErrorDiff_xun;
					
					// 限制PID输出范围
					if (Out_xun > 300) Out_xun = 300;
					if (Out_xun < -300) Out_xun = -300;
					
					// 计算左右电机速度
					leftSpeed = 300 - Out_xun;
					rightSpeed = 300 + Out_xun;
					
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
					}
					else
					{
						WhiteDetectCount=0;
					}	

					if(WhiteDetectCount>3)//判断是否已经识别到全白
					{
						WhiteDetectCount=0;
						if(currentState==STATE_B_TO_C)
						{
							JY62_RotateToAngle(initialYaw+180.0);
							Encoder_Reset();
							// 重置PID参数
							ErrorSum1 = 0;
							ErrorSum2 = 0;
							ErrorLast1 = 0;
							ErrorLast2 = 0;
							currentState =STATE_C_TO_D;		
							Delay_ms(300);
											
						}
						else
						{
							JY62_RotateToAngle(initialYaw);
							currentState = STATE_STOP ;
						}
						Flag_LED_ON =1;
						Flag_Buzzer_ON =1;	
					}
					break;
					
				case STATE_STOP:
					break;
			}
		}
		
		// 处理Mode 2的状态
		if(Mode == 2)
		{ 
			switch(currentState2)
			{ 
				case STATE_START_2:
					break;	
				case STATE_ROTATE_N45_2:
					JY62_RotateToAngle(initialYaw-45.0);
					currentState2 = STATE_A_TO_C1_2;
					encoderCount = 0;	
					// Flag_LED_OFF =1;
					break;
				case STATE_ROTATE_N135_2:
					JY62_RotateToAngle(initialYaw-135.0);
					currentState2 = STATE_B_TO_D1_2;
					encoderCount = 0;	
					// Flag_LED_OFF =1;
					break;	
				case STATE_A_TO_C1_2:
				case STATE_C1_TO_C2_2:
				case STATE_B_TO_D1_2:
				case STATE_D1_TO_D2_2:
					if(currentState2 == STATE_A_TO_C1_2)//A->C1
					{
						angleTarget = -45.0f;
					}
					else if(currentState2 == STATE_C1_TO_C2_2)//C1->C2
					{
						angleTarget = 0.0f;
					}
					else if(currentState2 == STATE_B_TO_D1_2)//B->D1
					{
						angleTarget = -135.0f;
					}
					else if(currentState2 == STATE_D1_TO_D2_2)//D1->D2
					{
						angleTarget = 180.0f;
					}
					currentYaw = JY62_GetYaw();
					angleError = angleTarget - currentYaw;

					// 角度误差归一化到-180~180度
					if (angleError > 180) {
						angleError -= 360;
					} else if (angleError < -180) {
						angleError += 360;
					}

					// 方向环PID参数
					Kp_angle = 3.0f;
					Ki_angle = 0.5f;
					Kd_angle = 1.0f;
					angleErrorSum = 0.0f;
					lastAngleError = 0.0f;

					angleErrorSum += angleError;
					angleErrorDiff = angleError - lastAngleError;

					angleOut = Kp_angle * angleError + Ki_angle * angleErrorSum + Kd_angle * angleErrorDiff;

					// 限制方向环输出范围
					if (angleOut > 300) angleOut = 300;
					if (angleOut < -300) angleOut = -300;

					// 基础速度
					baseSpeed = 300;

					// 计算左右电机速度
					leftSpeed = baseSpeed - angleOut;
					rightSpeed = baseSpeed + angleOut;

					// 限制速度范围
					if (leftSpeed > 1000) leftSpeed = 1000;
					if (leftSpeed < 0) leftSpeed = 0;
					if (rightSpeed > 1000) rightSpeed = 1000;
					if (rightSpeed < 0) rightSpeed = 0;

					// 编码器计数检测
					// static int32_t encoderCount = 0;
					
					int16_t leftEncoder = Encoder_GetLeft();
					int16_t rightEncoder = Encoder_GetRight();

					if (leftEncoder >= 1000||rightEncoder >= 1000)
					{ 
						leftEncoder = 0;
						rightEncoder = 0;
					}
					
					// 累积编码器计数（取平均值）
					encoderCount += (leftEncoder + rightEncoder) / 2;
					
					// 检查是否达到10500计数
					if(currentState2 == STATE_A_TO_C1_2)
					{
						if(encoderCount >= 10000)
						{
							// 停止电机
							Motor_SetLeftSpeed(0);
							Motor_SetRightSpeed(0);
							currentState2 = STATE_C1_TO_C2_2;
						}						
					}

					else if(currentState2 == STATE_B_TO_D1_2)
					{
						if(encoderCount >= 9800)
						{
							// 停止电机
							Motor_SetLeftSpeed(0);
							Motor_SetRightSpeed(0);
							currentState2 = STATE_D1_TO_D2_2;
						}			
					}
					

					// 控制电机
					Motor_SetLeftSpeed(leftSpeed);
					Motor_SetRightSpeed(rightSpeed);
					
					// 更新上一次角度误差
					lastAngleError = angleError;

					if(Track_ReadAll()!=0x00)//判定是否识别到黑线
					{
						if(currentState2==STATE_C1_TO_C2_2)
						{
							currentState2 =STATE_C2_TO_B_2;
							
						}
						else if(currentState2==STATE_D1_TO_D2_2)
						{
							currentState2 = STATE_D2_TO_A_2;
						}
						Flag_LED_ON =1;
						Flag_Buzzer_ON =1;				
					}

					break;			
				case STATE_C2_TO_B_2:
				case STATE_D2_TO_A_2:
					ErrorNow_xun = Track_GetError();
		
					// 计算积分项
					ErrorSum_xun += ErrorNow_xun; 
					
					// 计算微分项
					ErrorDiff_xun = ErrorNow_xun - ErrorLast_xun;
					
					// PID计算
					Out_xun = Kp_xun * ErrorNow_xun + Ki_xun* ErrorSum_xun+ Kd_xun * ErrorDiff_xun;
					
					// 限制PID输出范围
					if (Out_xun > 200) Out_xun = 200;
					if (Out_xun < -200) Out_xun = -200;
					
					// 计算左右电机速度
					leftSpeed = 200 - Out_xun;
					rightSpeed = 200 + Out_xun;
					
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
					}
					else
					{
						WhiteDetectCount=0;
					}	

					if(WhiteDetectCount>2)//判断是否已经识别到全白
					{
						WhiteDetectCount=0;
						if(currentState2==STATE_C2_TO_B_2)
						{
							
							Encoder_Reset();
							// 重置PID参数
							// ErrorSum_xun = 0;
							// ErrorSum_xun = 0;
							// ErrorLast_xun = 0;
							// ErrorLast_xun = 0;
							currentState2 =STATE_ROTATE_N135_2;	
								
							// Delay_ms(300);
											
						}
						else//D2->A
						{
							ApointCounter++;
							if(ApointCounter>=LapCount)
							{
								currentState2 = STATE_STOP_2;	
							}
							else
							currentState2 = STATE_ROTATE_N45_2;
								
						}
						// Motor_SetLeftSpeed(0);
						// Motor_SetRightSpeed(0);
						LED_ON();
						Buzzer_ON();	
						Delay_ms(300);
						LED_OFF();
						Buzzer_OFF();	

					}
					break;

				// case STATE_C_TO_B_2:
				// 	JY62_RotateToAngle(initialYaw-30.0);
				// 	break;
				// case STATE_B_TO_D_2:
				// 	JY62_RotateToAngle(initialYaw+30.0);
				// 	break;
				// case STATE_D_TO_A_2:
				// 	JY62_RotateToAngle(initialYaw-130.0);
				// 	break;
				case STATE_STOP_2:
					JY62_RotateToAngle(initialYaw);
					break;
			}
		}
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}
