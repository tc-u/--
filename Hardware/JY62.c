#include "JY62.h"
#include "Delay.h"
#include "Motor.h"
#include <math.h>

// 全局变量
JY62_Data jy62Data;
u8 jy62RxBuffer[11];  // 接收缓冲区
u8 jy62RxIndex = 0;   // 接收索引
u8 jy62DataReady = 0; // 数据准备标志

/**
 * @brief 初始化JY62传感器的串口通信
 * @param 无
 * @retval 无
 */
void JY62_Init(void) {
    // 1. 使能GPIO和USART时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    
    // 2. 配置GPIO引脚
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 配置PB10为USART3_TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 配置PB11为USART3_RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 3. 配置USART3
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;  // JY62默认波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &USART_InitStructure);
    
    // 4. 使能USART3接收中断
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    
    // 5. 使能USART3
    USART_Cmd(USART3, ENABLE);
    
    // 6. 配置NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief USART3中断处理函数
 * @param 无
 * @retval 无
 */
void USART3_IRQHandler(void) {
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
        u8 byte = USART_ReceiveData(USART3);
        
        // 数据帧格式：0x55 0x53 + 9字节数据 + 校验和
        if (jy62RxIndex == 0 && byte != 0x55) {
            return;  // 不是帧头，丢弃
        }
        
        jy62RxBuffer[jy62RxIndex++] = byte;
        
        if (jy62RxIndex == 11) {
            // 检查帧头和校验和
            if (jy62RxBuffer[0] == 0x55 && jy62RxBuffer[1] == 0x53) {
                // 计算校验和
                u8 checksum = 0;
                for (u8 i = 0; i < 10; i++) {
                    checksum += jy62RxBuffer[i];
                }
                
                if (checksum == jy62RxBuffer[10]) {
                    // 解析数据
                    jy62Data.roll = ((int16_t)(jy62RxBuffer[3] << 8 | jy62RxBuffer[2])) / 100.0f;
                    jy62Data.pitch = ((int16_t)(jy62RxBuffer[5] << 8 | jy62RxBuffer[4])) / 100.0f;
                    jy62Data.yaw = ((int16_t)(jy62RxBuffer[7] << 8 | jy62RxBuffer[6])) / 100.0f;
                    jy62Data.wx = ((int16_t)(jy62RxBuffer[9] << 8 | jy62RxBuffer[8])) / 100.0f;
                    
                    jy62DataReady = 1;
                }
            }
            jy62RxIndex = 0;  // 重置索引
        }
        
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

/**
 * @brief 更新JY62传感器数据
 * @param 无
 * @retval 无
 */
void JY62_UpdateData(void) {
    if (jy62DataReady) {
        // 数据已更新，这里可以添加其他处理
        jy62DataReady = 0;
    }
}

/**
 * @brief 获取当前偏航角
 * @param 无
 * @retval 当前偏航角（度）
 */
float JY62_GetYaw(void) {
    return jy62Data.yaw;
}

/**
 * @brief 旋转到指定角度
 * @param targetAngle 目标角度（度）
 * @retval 无
 */
void JY62_RotateToAngle(float targetAngle) {
    // 计算目标角度与当前角度的差值
    float currentYaw = JY62_GetYaw();
    float angleDiff = targetAngle - currentYaw;
    
    // 角度归一化到-180~180度
    if (angleDiff > 180) {
        angleDiff -= 360;
    } else if (angleDiff < -180) {
        angleDiff += 360;
    }
    
    // 简单的P控制
    float Kp = 5.0f;  // 比例系数
    int16_t speed = (int16_t)(angleDiff * Kp);
    
    // 限制速度范围
    if (speed > 500) speed = 500;
    if (speed < -500) speed = -500;
    
    // 旋转
    while (1) {
        currentYaw = JY62_GetYaw();
        angleDiff = targetAngle - currentYaw;
        
        // 角度归一化
        if (angleDiff > 180) {
            angleDiff -= 360;
        } else if (angleDiff < -180) {
            angleDiff += 360;
        }
        
        // 检查是否到达目标角度（误差小于1度）
        if (fabs(angleDiff) < 1.0f) {
            break;
        }
        
        // 更新速度
        speed = (int16_t)(angleDiff * Kp);
        if (speed > 500) speed = 500;
        if (speed < -500) speed = -500;
        
        // 设置电机速度
        Motor_SetLeftSpeed(500 - speed);
        Motor_SetRightSpeed(500 + speed);
        
        Delay_ms(10);
    }
    
    // 停止电机
    Motor_SetSpeed(0, 0);
}
