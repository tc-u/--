#include "stm32f10x.h"
#ifndef __TRACK_H
#define __TRACK_H

// 传感器引脚定义
#define AD0_PIN  GPIO_Pin_0
#define AD1_PIN  GPIO_Pin_1
#define AD2_PIN  GPIO_Pin_10
#define OUT_PIN  GPIO_Pin_11

// 通道定义
#define CHANNEL_0 0
#define CHANNEL_1 1
#define CHANNEL_2 2
#define CHANNEL_3 3
#define CHANNEL_4 4
#define CHANNEL_5 5
#define CHANNEL_6 6
#define CHANNEL_7 7

// 初始化函数
void Track_Init(void);

// 读取单个通道灰度值（0：检测到黑线，1：检测到白线）
uint8_t Track_ReadChannel(uint8_t channel);

// 读取所有通道灰度值（返回8位二进制数，每一位对应一个通道）
uint8_t Track_ReadAll(void);

// 简化的循迹状态获取
float Track_GetError(void);

#endif
