#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Laser.h"
#include "Motor.h"
#include "usart1.h"
#include "Button.h"

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
		Delay_ms(1);    // 1msÑÓ³Ù±ÜÃâ100%Õ¼ÓÃCPU
		if(AIM_ENABLE == 1){
			// 1. ÂË²¨£ºÈ¥³ýÍ¼ÏñÃ«´Ì
			int32_t dx = Filter_Dx(Vision_dx);
			int32_t dy = Filter_Dy(Vision_dy);

			// 2. PID¿ØÖÆ£ºÎÞ°ÚÍ·¡¢³¬Ë³»¬×·×Ù
			PID_PAN(dx);
			PID_TILT(dy);
			
			// 3. ÊÇ·ñ¿ªÆô¼¤¹â±Ê
			if(abs(dx * dx + dy * dy) <= ON_WIN_THRESHOLD){
				Laser_On();
			}
			else{
				Laser_Off();
			}
			
			// 4. OLEDÏÔÊ¾
			OLED_ShowSignedNum(1, 1, dx, 4);
			OLED_ShowSignedNum(1, 7, dy, 4);
			OLED_ShowNum(2, 1, P_PAN, 4);
			OLED_ShowNum(2, 6, I_PAN, 4);
			OLED_ShowNum(2, 11, D_PAN, 4);
			OLED_ShowNum(3, 1, P_TILT, 4);
			OLED_ShowNum(3, 6, I_TILT, 4);
			OLED_ShowNum(3, 11, D_TILT, 4);
		}
		else{
			Motor_Stop();
			Motor_ClearPID();
			Laser_Off();
			OLED_Clear();
		}
	}
}
