#include "pwm.h"
#include "stm32f10x.h"

void pwm_init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef timer;
    TIM_OCInitTypeDef channel;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    /* 独占 TIM3。复位确保输出比较寄存器从零开始。 */
    TIM_DeInit(TIM3);
    TIM_TimeBaseStructInit(&timer);
    timer.TIM_ClockDivision = TIM_CKD_DIV1;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    timer.TIM_Period = PWM_PERIOD_COUNTS - 1U;
    timer.TIM_Prescaler = PWM_PRESCALER_DIV - 1U;
    TIM_InternalClockConfig(TIM3);
    TIM_TimeBaseInit(TIM3, &timer);
    TIM_OCStructInit(&channel);
    channel.TIM_OCMode = TIM_OCMode_PWM1;
    channel.TIM_OCPolarity = TIM_OCPolarity_High;
    channel.TIM_OutputState = TIM_OutputState_Enable;
    channel.TIM_Pulse = 0;
    TIM_OC1Init(TIM3, &channel);
    TIM_OC2Init(TIM3, &channel);
    TIM_OC3Init(TIM3, &channel);
    TIM_OC4Init(TIM3, &channel);

    /* TODO_DRV8833：保留旧 EN 引脚，未来重配定时器通道。
     * TIM3_CH1=PA6，CH2=PA7，CH3=PB0，CH4=PB1。
     */
    GPIO_StructInit(&gpio);
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &gpio);
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &gpio);
    /* 保留 CCR 直接更新，零输出不等待下一个 20 ms 周期。
     * 未来若启用 CCR 预装载，必须同时重做立即停车/换向时序。
     */
    TIM_Cmd(TIM3, ENABLE);
}

static uint16_t duty_to_compare(float duty)
{
    if (!(duty > 0.0f))
    {
        return 0; /* 0、负值、NaN；绝不能对零占空比再减 1。 */
    }
    if (duty >= 1.0f)
    {
        return PWM_PERIOD_COUNTS;
    }
    /* PWM1 向上计数：CNT < CCR 时高电平。
     * ARR=19999，共20000个计数；CCR=0 -> 0%，CCR=20000 -> 100%。
     */
    return (uint16_t)(duty * PWM_PERIOD_COUNTS);
}

void pwm_set1(float duty) { TIM_SetCompare1(TIM3, duty_to_compare(duty)); }
void pwm_set2(float duty) { TIM_SetCompare2(TIM3, duty_to_compare(duty)); }
void pwm_set3(float duty) { TIM_SetCompare3(TIM3, duty_to_compare(duty)); }
void pwm_set4(float duty) { TIM_SetCompare4(TIM3, duty_to_compare(duty)); }

void pwm_stop_all(void)
{
    TIM_SetCompare1(TIM3, 0);
    TIM_SetCompare2(TIM3, 0);
    TIM_SetCompare3(TIM3, 0);
    TIM_SetCompare4(TIM3, 0);
}
