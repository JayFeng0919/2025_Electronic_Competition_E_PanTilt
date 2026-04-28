#ifndef __LASER_H
#define __LASER_H

#include "stm32f10x.h"

#define Laser_Port   GPIOB
#define Laser_Pin    GPIO_Pin_6
#define ON_WIN_THRESHOLD       400

void Laser_Init(void);
void Laser_On(void);
void Laser_Off(void);

#endif
