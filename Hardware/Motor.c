#include "Motor.h"
#include "Button.h"

// 滤波缓存
static int16_t dx_buf[3] = {0};
static int16_t dy_buf[3] = {0};
static uint8_t filter_idx_dx = 0;
static uint8_t filter_idx_dy = 0;

// PID上一误差和历史误差累积
static volatile int32_t last_dx = 0;
static volatile int32_t last_dy = 0;
static volatile int16_t integral_dx = 0;
static volatile int16_t integral_dy = 0;

static volatile uint16_t current_arr_pan = ARR_MAX;
static volatile uint16_t current_arr_tilt = ARR_MAX;

//=========================================================================
// GPIO初始化
//=========================================================================
static void Motor_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    // PAN_DIR
    GPIO_InitStruct.GPIO_Pin = PAN_DIR_PIN;
    GPIO_Init(PAN_DIR_PORT, &GPIO_InitStruct);

    // TILT_DIR
    GPIO_InitStruct.GPIO_Pin = TILT_DIR_PIN;
    GPIO_Init(TILT_DIR_PORT, &GPIO_InitStruct);

    // PA6 复用功能(TIM3_CH1)
    GPIO_InitStruct.GPIO_Pin = PAN_PUL_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
	
    // PB7 复用功能(TIM4_CH2)
    GPIO_InitStruct.GPIO_Pin = TILT_PUL_PIN;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

//=========================================================================
// TIM3_CH1 TIM4_CH2 硬件PWM初始化 (PA6 PB7)
//=========================================================================
static void Motor_TIM3_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_InitStruct.TIM_Period = ARR_MAX;
    TIM_InitStruct.TIM_Prescaler = PSC_VAL;
    TIM_InitStruct.TIM_ClockDivision = 0;
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_InitStruct);

    // PWM模式1，50%占空比初始
	TIM_OCStructInit(&TIM_OCInitStruct);
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_Pulse = 5000;

    TIM_OC1Init(TIM3, &TIM_OCInitStruct);

    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
	
	current_arr_pan = ARR_MAX;
}

static void Motor_TIM4_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_InitStruct.TIM_Period = ARR_MAX;
    TIM_InitStruct.TIM_Prescaler = PSC_VAL;
    TIM_InitStruct.TIM_ClockDivision = 0;
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_InitStruct);

    // PWM模式1，50%占空比初始
    TIM_OCStructInit(&TIM_OCInitStruct);
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_Pulse = 5000;
    TIM_OC2Init(TIM4, &TIM_OCInitStruct);

    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);

    current_arr_tilt = ARR_MAX;
}

//=========================================================================
// 动态设置PWM频率 (ARR自动计算，CCR=ARR/2，恒50%占空比)
//=========================================================================
void Motor_SetPWM(uint16_t arr_pan, uint16_t arr_tilt)
{
    // 限幅保护
    if(arr_pan < ARR_MIN) arr_pan = ARR_MIN;
    if(arr_pan > ARR_MAX) arr_pan = ARR_MAX;
    if(arr_tilt < ARR_MIN) arr_tilt = ARR_MIN;
    if(arr_tilt > ARR_MAX) arr_tilt = ARR_MAX;

    if(arr_pan != current_arr_pan)
    {
        TIM_SetAutoreload(TIM3, arr_pan);
        TIM_SetCompare1(TIM3, arr_pan / 2);
        current_arr_pan = arr_pan;
    }

    // 更新TILT轴
    if(arr_tilt != current_arr_tilt)
    {
        TIM_SetAutoreload(TIM4, arr_tilt);
        TIM_SetCompare2(TIM4, arr_tilt / 2);
        current_arr_tilt = arr_tilt;
    }
}

//=========================================================================
// 停止电机
//=========================================================================
void Motor_Stop(void)
{
    TIM_SetCompare1(TIM3, 0);
    TIM_SetCompare2(TIM4, 0);
}

//=========================================================================
// 3点滑动平均滤波（去毛刺）
//=========================================================================
int32_t Filter_Dx(int16_t new_dx)
{
    dx_buf[filter_idx_dx] = new_dx;
    filter_idx_dx = (filter_idx_dx + 1) % 3;
    return (dx_buf[0]+dx_buf[1]+dx_buf[2])/3;
}

int32_t Filter_Dy(int16_t new_dy)
{
    dy_buf[filter_idx_dy] = new_dy;
    filter_idx_dy = (filter_idx_dy + 1) % 3;
    return (dy_buf[0]+dy_buf[1]+dy_buf[2])/3;
}

//=========================================================================
// PID控制器（输出直接映射为ARR值，控制速度）
//=========================================================================
void PID_PAN(int32_t dx)
{
    if(dx > -DEAD_ZONE && dx < DEAD_ZONE){
        TIM_SetCompare1(TIM3, 0);
        last_dx = 0;
		integral_dx = 0; // 死区时重置积分
        return;
    }
	// 1. 积分分离逻辑：偏差足够小（说明快追上了），才开始累加积分
    if(abs(dx) < I_SEP_THRESHOLD) {
        integral_dx += dx;
        // 2. 积分抗饱和限幅：把积分圈禁在安全范围内
        if(integral_dx > I_LIMIT) integral_dx = I_LIMIT;
        else if(integral_dx < -I_LIMIT) integral_dx = -I_LIMIT;
    }
    else {
        // 偏差过大（小车猛打弯丢掉目标），立刻清空积分，只用 PD 猛追
        integral_dx = 0;
    }

    int32_t output = (P_PAN * dx + I_PAN * integral_dx + D_PAN * (dx - last_dx)) / 100;
    last_dx = dx;

    // 方向控制
    if(output > 0) GPIO_ResetBits(PAN_DIR_PORT, PAN_DIR_PIN);
    else GPIO_SetBits(PAN_DIR_PORT, PAN_DIR_PIN);

    // 输出转ARR
	uint16_t temp = (uint16_t)(abs(output) * 10);
    if(temp > (ARR_MAX - ARR_MIN)) temp = ARR_MAX - ARR_MIN;
    uint16_t arr = ARR_MAX - temp;
    Motor_SetPWM(arr, current_arr_tilt);
}

void PID_TILT(int32_t dy)
{
    if(dy > -DEAD_ZONE && dy < DEAD_ZONE){
        TIM_SetCompare2(TIM4, 0);
        last_dy = 0;
		integral_dy = 0; // 死区时重置积分
        return;
    }
	if(abs(dy) < I_SEP_THRESHOLD) {
        integral_dy += dy;
        if(integral_dy > I_LIMIT) integral_dy = I_LIMIT;
        else if(integral_dy < -I_LIMIT) integral_dy = -I_LIMIT;
    }
    else {
        integral_dy = 0;
    }

    int32_t output = (P_TILT * dy + I_TILT * integral_dy + D_TILT * (dy - last_dy)) / 100;
    last_dy = dy;

    if(output > 0) GPIO_SetBits(TILT_DIR_PORT, TILT_DIR_PIN);
    else GPIO_ResetBits(TILT_DIR_PORT, TILT_DIR_PIN);

    uint16_t temp = (uint16_t)(abs(output) * 10);
    if(temp > (ARR_MAX - ARR_MIN)) temp = ARR_MAX - ARR_MIN;
    uint16_t arr = ARR_MAX - temp;
    Motor_SetPWM(current_arr_pan, arr);
}

//=========================================================================
// 总初始化
//=========================================================================
void Motor_Init(void)
{
    Motor_GPIO_Init();
    Motor_TIM3_PWM_Init();
	Motor_TIM4_PWM_Init();
    Motor_Stop();
}

void Motor_ClearPID(void) {
    integral_dx = 0;
    integral_dy = 0;
    last_dx = 0;
    last_dy = 0;
}
