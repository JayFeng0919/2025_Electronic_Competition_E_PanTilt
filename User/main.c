#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Laser.h"
#include "Motor.h"
#include "usart1.h"
#include "Button.h"

extern uint32_t exposure;

uint8_t comp = 60;
static uint32_t stable = 0;

//// 脱靶 左右来回扫描
//volatile uint8_t  lose_target_cnt = 0;       // 丢目标计数
//volatile uint8_t  is_lose_target   = 0;      // 1=脱靶 0=追踪
//volatile int8_t   scan_dir         = 1;     // 扫描方向 1=右 -1=左
//volatile uint16_t scan_step_cnt    = 0;     // 扫描步数计时
//uint16_t scan_switch_step = 120;    // 多少步后换向
//volatile uint16_t pwm_speed = 7999;

int main(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	OLED_Init();
	USART1_Init();
	Motor_Init();
	Laser_Init();
	Button_Init();
	
	while (1)
	{
		OLED_ShowSignedNum(1, 1, Vision_dx, 4);
		OLED_ShowSignedNum(1, 7, Vision_dy, 4);
		OLED_ShowNum(4, 6, exposure, 6);
		Button_Scan();
		if(key0_flag){
			AIM_ENABLE = !AIM_ENABLE;
			key0_flag = 0;
		}
		else if(key1_flag){
			P_PAN = 500, I_PAN = 25, D_PAN = 6000;
			P_TILT = 200, I_TILT = 5, D_TILT = 2000;
			ON_WIN_THRESHOLD = 32;
			comp = 30;
			key1_flag = 0;
		}
		else if(key2_flag){
			P_PAN = 1800, I_PAN = 50, D_PAN = 8000;	
			P_TILT = 200, I_TILT = 10, D_TILT = 3000;
			ON_WIN_THRESHOLD = 40000;
			comp = 3;
			key2_flag = 0;
		}
		else if(key3_flag){
			P_PAN *= 2;
			I_PAN -= 10;
			D_PAN *= 2;
			key3_flag = 0;
		}
		
		if(AIM_ENABLE == 1){
			OLED_ShowNum(2, 1, P_PAN, 4);
			OLED_ShowNum(2, 6, I_PAN, 4);
			OLED_ShowNum(2, 11, D_PAN, 5);
			OLED_ShowNum(3, 1, P_TILT, 4);
			OLED_ShowNum(3, 6, I_TILT, 4);
			OLED_ShowNum(3, 11, D_TILT, 5);
			
			if(Vision_dx == 999){
				Motor_Stop();
				Motor_ClearPID();
				Laser_Off();
			}
			
//			if(if_scan == 1 && Vision_dx == 999){
//				lose_target_cnt++;
//				if(lose_target_cnt >= 12){
//					is_lose_target = 1;
//				}
//			}
//			else{
//				is_lose_target = 0;
//				lose_target_cnt = 0;
//				scan_step_cnt = 0;
//				scan_switch_step = 120;
//			}
//			
//			if(is_lose_target == 1){
//				Motor_ClearPID();
//				if(scan_dir == 1){
//					GPIO_ResetBits(PAN_DIR_PORT, PAN_DIR_PIN);
//				}
//				else{
//					GPIO_SetBits(PAN_DIR_PORT, PAN_DIR_PIN);
//				}
//				Motor_SetPWM(pwm_speed, 9999);
//				TIM_SetCompare2(TIM4, 0);
//				scan_step_cnt++;
//				if(scan_step_cnt >= scan_switch_step){
//					scan_step_cnt = 0;
//					scan_dir = -scan_dir;
//					scan_switch_step += 120;
//					if(scan_switch_step > 480){
//						scan_switch_step = 100;
//					}
//				}
//			}
			else if(Vision_dx != 999){
				// 1. 滤波：去除图像毛刺
				int32_t dx = Filter_Dx(Vision_dx);
				int32_t dy = Filter_Dy(Vision_dy);

				// 2. PID控制：无摆头、超顺滑追踪
				PID_PAN(dx);
				PID_TILT(dy);
					
				// 3. 是否开启激光笔
				if(dx * dx + dy * dy <= ON_WIN_THRESHOLD){
					stable++;
				}
				else{
					stable = 0;
				}
				if(stable >= comp){
					Laser_On();
				}
				else{
					Laser_Off();
				}
			}	
		}
			
		else{
			Motor_Stop();
			Motor_ClearPID();
			Laser_Off();
			OLED_Clear();
		}
	}
}
