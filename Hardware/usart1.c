#include "usart1.h"
#include "Motor.h"

int16_t Vision_dx = 0;
int16_t Vision_dy = 0;

static uint8_t buf[6];
static uint8_t index = 0;

void USART1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitS;
    USART_InitTypeDef USART_InitS;
    NVIC_InitTypeDef NVIC_InitS;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // PA9 TX
    GPIO_InitS.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitS.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitS);

    // PA10 RX
    GPIO_InitS.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitS.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitS);

    // 中断配置
    NVIC_InitS.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitS.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitS.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitS.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitS);

    USART_InitS.USART_BaudRate = 115200;
    USART_InitS.USART_WordLength = USART_WordLength_8b;
    USART_InitS.USART_StopBits = USART_StopBits_1;
    USART_InitS.USART_Parity = USART_Parity_No;
    USART_InitS.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitS.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitS);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

// 串口接收中断
void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        uint8_t ch = USART_ReceiveData(USART1);
        buf[index++] = ch;

        if(index == 6 && buf[0]==0xAA && buf[5]==0xDD)
        {
            // 解析 dx dy
            Vision_dx = ((int16_t)buf[1]<<8) | buf[2];
            Vision_dy = ((int16_t)buf[3]<<8) | buf[4];
            index = 0;
        }
        if(index >=6) index=0;
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
