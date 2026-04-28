#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f10x.h"

// ==================== 引脚定义 ====================
#define KEY0_Pin    GPIO_Pin_0
#define KEY0_Port   GPIOA
#define KEY0_CLK    RCC_APB2Periph_GPIOA

#define KEY1_Pin    GPIO_Pin_1
#define KEY1_Port   GPIOA
#define KEY1_CLK    RCC_APB2Periph_GPIOA

#define KEY2_Pin    GPIO_Pin_2
#define KEY2_Port   GPIOA
#define KEY2_CLK    RCC_APB2Periph_GPIOA

#define KEY3_Pin    GPIO_Pin_3
#define KEY3_Port   GPIOA
#define KEY3_CLK    RCC_APB2Periph_GPIOA

// ==================== 函数声明 ====================
void Button_Init(void);  // 按键中断初始化

// ==================== 外部全局变量 ====================
extern volatile uint8_t AIM_ENABLE;
extern volatile int16_t P_PAN;
extern volatile int16_t I_PAN;
extern volatile int16_t D_PAN;
extern volatile int16_t P_TILT;
extern volatile int16_t I_TILT;
extern volatile int16_t D_TILT;

#endif
