#include "Button.h"
#include "Delay.h"

// 全局变量定义
volatile uint8_t AIM_ENABLE = 0;
volatile int16_t P_PAN = 110;
volatile int16_t I_PAN = 5;
volatile int16_t D_PAN = 600;
volatile int16_t P_TILT = 100;
volatile int16_t I_TILT = 3;
volatile int16_t D_TILT = 400;

// 按键中断初始化：PA0/PA1/PA2/PA3 上拉输入，下降沿中断
void Button_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStruct;
    EXTI_InitTypeDef        EXTI_InitStruct;
    NVIC_InitTypeDef        NVIC_InitStruct;

    // 开时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStruct.GPIO_Pin   = KEY0_Pin | KEY1_Pin | KEY2_Pin | KEY3_Pin;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IPU;  // 上拉输入
    GPIO_Init(KEY0_Port, &GPIO_InitStruct);

    // 映射GPIO到EXTI
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);

    // 配置EXTI中断
    EXTI_InitStruct.EXTI_Line    = EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3;
    EXTI_InitStruct.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;  // 下降沿触发（按下低电平）
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // 中断配置
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
	
    NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
}

// ==================== PA0 中断服务函数：启动/停止瞄准 ====================
void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) == SET)
    {
        Delay_ms(20);  // 软件消抖
        if(GPIO_ReadInputDataBit(KEY0_Port, KEY0_Pin) == 0)
        {
            AIM_ENABLE = !AIM_ENABLE;  // 翻转使能
        }
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

// ==================== PA1 PA2 PA3 中断服务函数：调节P/I/D ====================
void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1) == SET)
    {
        Delay_ms(20);
        if(GPIO_ReadInputDataBit(KEY1_Port, KEY1_Pin) == 0)
        {
            P_PAN += 10;
        }
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI2_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line2) == SET)
    {
        Delay_ms(20);
        if(GPIO_ReadInputDataBit(KEY2_Port, KEY2_Pin) == 0)
        {
            I_PAN += 10;
        }
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line3) == SET)
    {
        Delay_ms(20);
        if(GPIO_ReadInputDataBit(KEY3_Port, KEY3_Pin) == 0)
        {
            D_PAN += 10;
        }
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}
