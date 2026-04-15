#include "stm32f10x.h"
#include "Track.h"

/**
 * @brief 初始化红外循迹模块
 */
void Track_Init(void)
{
    // 使能GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 配置AD0、AD1、AD2为推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = AD0_PIN | AD1_PIN | AD2_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 配置OUT为浮空输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = OUT_PIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 初始通道设置为0
    GPIO_ResetBits(GPIOB , AD0_PIN);
    GPIO_ResetBits(GPIOB, AD1_PIN);
    GPIO_ResetBits(GPIOB, AD2_PIN);
}

/**
 * @brief 选择通道
 * @param channel 通道号（0-7）
 */
static void Track_SelectChannel(uint8_t channel)
{
    // 设置AD0
    if (channel & 0x01)
        GPIO_SetBits(GPIOB, AD0_PIN);
    else
        GPIO_ResetBits(GPIOB, AD0_PIN);
    
    // 设置AD1
    if (channel & 0x02)
        GPIO_SetBits(GPIOB, AD1_PIN);
    else
        GPIO_ResetBits(GPIOB, AD1_PIN);
    
    // 设置AD2
    if (channel & 0x04)
        GPIO_SetBits(GPIOB, AD2_PIN);
    else
        GPIO_ResetBits(GPIOB, AD2_PIN);
}

/**
 * @brief 读取单个通道灰度值
 * @param channel 通道号（0-7）
 * @return 0：检测到黑线，1：检测到白线
 */
uint8_t Track_ReadChannel(uint8_t channel)
{
    // 选择通道
    Track_SelectChannel(channel);
    
    // 短暂延时，确保通道切换完成
    for (volatile uint16_t i = 0; i < 10; i++);
    
    // 读取OUT引脚状态
    return GPIO_ReadInputDataBit(GPIOB, OUT_PIN);
}

/**
 * @brief 读取所有通道灰度值
 * @return 8位二进制数，每一位对应一个通道（0：检测到黑线，1：检测到白线）
 */
uint8_t Track_ReadAll(void)
{
    uint8_t result = 0;
    
    for (uint8_t i = 0; i < 8; i++)
    {
        // 读取每个通道的值，左移对应位
        result |= (Track_ReadChannel(i) << i);
    }
    
    return result;
}

/**
 * @brief 获取循迹偏差值
 * @return 偏差值（-3.5到3.5，0为中心）
 *         负值：小车偏左，需要右转
 *         正值：小车偏右，需要左转
 */
float Track_GetError(void)
{
    uint8_t value = Track_ReadAll();
    int8_t activeChannels = 0;
    float errorSum = 0;
    
    // 8个通道的位置权重（从左到右）
    float weights[] = {-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5};
    
    for (uint8_t i = 0; i < 8; i++)
    {
        // 如果该通道检测到黑线（值为0）
        if (!(value & (1 << i)))
        {
            errorSum += weights[i];
            activeChannels++;
        }
    }
    
    // 如果没有检测到黑线，返回一个大偏差（脱线）
    if (activeChannels == 0)
    {
        return 10.0;  // 脱线标志
    }
    
    // 返回平均偏差
    return errorSum / activeChannels;
}
