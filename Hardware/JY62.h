#ifndef __JY62_H
#define __JY62_H

#include "stm32f10x.h"

// 定义JY62传感器的数据结构
typedef struct {
    float pitch;   // 俯仰角
    float roll;    // 横滚角
    float yaw;     // 偏航角
    float ax;      // X轴加速度
    float ay;      // Y轴加速度
    float az;      // Z轴加速度
    float wx;      // X轴角速度
    float wy;      // Y轴角速度
    float wz;      // Z轴角速度
} JY62_Data;

// 函数声明
void JY62_Init(void);
void JY62_UpdateData(void);
float JY62_GetYaw(void);
void JY62_RotateToAngle(float targetAngle);

#endif
#ifndef __JY62_H
#define __JY62_H

#include "stm32f10x.h"

// 定义JY62传感器的数据结构
typedef struct {
    float pitch;   // 俯仰角
    float roll;    // 横滚角
    float yaw;     // 偏航角
    float ax;      // X轴加速度
    float ay;      // Y轴加速度
    float az;      // Z轴加速度
    float wx;      // X轴角速度
    float wy;      // Y轴角速度
    float wz;      // Z轴角速度
} JY62_Data;

// 函数声明
void JY62_Init(void);
void JY62_UpdateData(void);
float JY62_GetYaw(void);
void JY62_RotateToAngle(float targetAngle);

#endif
