#include "Laser.h"

void Laser_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = Laser_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(Laser_Port, &GPIO_InitStructure);
	
	GPIO_SetBits(Laser_Port, Laser_Pin);
}

void Laser_On(void){
	GPIO_ResetBits(Laser_Port, Laser_Pin);
}
	
void Laser_Off(void){
	GPIO_SetBits(Laser_Port, Laser_Pin);
}
