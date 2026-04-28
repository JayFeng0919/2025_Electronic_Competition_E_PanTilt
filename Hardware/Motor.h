#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"
#include <math.h>
#include "Delay.h"
#include <stdlib.h>

//===================== 硬件引脚宏定义 =====================
// 水平PAN轴
#define PAN_PUL_PIN    GPIO_Pin_6
#define PAN_PUL_PORT   GPIOA
#define PAN_DIR_PIN    GPIO_Pin_13
#define PAN_DIR_PORT   GPIOB

// 俯仰TILT轴
#define TILT_PUL_PIN   GPIO_Pin_7
#define TILT_PUL_PORT  GPIOB
#define TILT_DIR_PIN   GPIO_Pin_14
#define TILT_DIR_PORT  GPIOB

//===================== 电机与控制参数 =====================
#define PSC_VAL          71      // 固定预分频
#define ARR_MIN          3999      // 最大速度(250Hz)
#define ARR_MAX          9999     // 最小速度(100Hz)
#define DEAD_ZONE        4       // 死区（小于此值不动作，防微抖）

// 积分控制安全参数
#define I_SEP_THRESHOLD  35       // 积分分离阈值：像素偏差小于35才允许积分
#define I_LIMIT          2000  // 积分限幅最大值：防止累计过大导致疯车

void Motor_Init(void);
void Motor_SetPWM(uint16_t arr_pan, uint16_t arr_tilt);
void Motor_Stop(void);
int32_t Filter_Dx(int16_t new_dx);
int32_t Filter_Dy(int16_t new_dy);
void PID_PAN(int32_t dx);
void PID_TILT(int32_t dy);
void Motor_ClearPID(void);

#endif
