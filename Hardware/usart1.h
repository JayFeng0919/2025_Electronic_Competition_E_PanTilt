#ifndef __USART1_H
#define __USART1_H

#include "stm32f10x.h"

// 接收视觉数据
extern int16_t Vision_dx;
extern int16_t Vision_dy;

void USART1_Init(void);

#endif
