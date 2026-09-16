/* 仅用于电脑测试：模拟引脚及比较寄存器，不模拟真实 PWM 波形/电流。 */
#include "mock_hardware.h"
#include "delay.h"
#include <assert.h>
#include <string.h>

uint16_t mock_ccr[4];
uint16_t mock_gpio_a;
uint16_t mock_gpio_b;
uint32_t mock_apb2;
uint16_t mock_period;
uint16_t mock_prescaler;
static uint8_t incoming[9];
static uint8_t outgoing[9];
static size_t input_count;
static size_t read_bit;
static size_t sent_bit;

void mock_frame(const uint8_t *bytes, size_t count)
{
    assert(count <= sizeof incoming);
    memset(incoming, 0xFF, sizeof incoming);
    memcpy(incoming, bytes, count);
    memset(outgoing, 0, sizeof outgoing);
    input_count = count;
    read_bit = 0;
    sent_bit = 0;
}

void mock_assert_poll_complete(void)
{
    static const uint8_t request[9] = {1, 0x42, 0, 0, 0, 0, 0, 0, 0};
    assert(read_bit == 72 && sent_bit == 72);
    assert(memcmp(outgoing, request, sizeof request) == 0);
    assert((mock_gpio_b & (GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15)) ==
           (GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15));
}

void RCC_APB1PeriphClockCmd(uint32_t periph, FunctionalState state)
{
    assert(periph == RCC_APB1Periph_TIM3 && state == ENABLE);
}
void RCC_APB2PeriphClockCmd(uint32_t periph, FunctionalState state)
{
    assert(state == ENABLE);
    mock_apb2 |= periph;
}
void GPIO_StructInit(GPIO_InitTypeDef *gpio) { memset(gpio, 0, sizeof *gpio); }
void GPIO_Init(GPIO_TypeDef *port, GPIO_InitTypeDef *gpio)
{
    (void)gpio;
    if (port == GPIOA) assert(mock_apb2 & RCC_APB2Periph_GPIOA);
    else assert(port == GPIOB && (mock_apb2 & RCC_APB2Periph_GPIOB));
}
static void assert_direction_disabled(void)
{
    assert(mock_ccr[0] == 0 && mock_ccr[1] == 0 &&
           mock_ccr[2] == 0 && mock_ccr[3] == 0);
}
void GPIO_SetBits(GPIO_TypeDef *port, uint16_t pins)
{
    if (port == GPIOA)
    {
        assert_direction_disabled();
        mock_gpio_a |= pins;
    }
    else
    {
        assert(port == GPIOB);
        mock_gpio_b |= pins;
    }
}
void GPIO_ResetBits(GPIO_TypeDef *port, uint16_t pins)
{
    if (port == GPIOA)
    {
        assert_direction_disabled();
        mock_gpio_a &= (uint16_t)~pins;
    }
    else
    {
        assert(port == GPIOB);
        if ((pins & GPIO_Pin_15) && !(mock_gpio_b & GPIO_Pin_14))
        {
            assert(sent_bit < input_count * 8);
            if (mock_gpio_b & GPIO_Pin_13)
                outgoing[sent_bit / 8] |= (uint8_t)(1U << (sent_bit % 8));
            sent_bit++;
        }
        mock_gpio_b &= (uint16_t)~pins;
    }
}
uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef *port, uint16_t pin)
{
    uint8_t value;
    assert(port == GPIOB && pin == GPIO_Pin_12);
    assert(!(mock_gpio_b & GPIO_Pin_14)); /* CS 必须有效 */
    assert(!(mock_gpio_b & GPIO_Pin_15)); /* 原协议在低半周期末采样 */
    assert(read_bit < input_count * 8 && sent_bit == read_bit + 1);
    value = (uint8_t)((incoming[read_bit / 8] >> (read_bit % 8)) & 1U);
    read_bit++;
    return value;
}
void TIM_DeInit(TIM_TypeDef *timer)
{
    assert(timer == TIM3);
    memset(mock_ccr, 0, sizeof mock_ccr);
}
void TIM_TimeBaseStructInit(TIM_TimeBaseInitTypeDef *timer)
{ memset(timer, 0, sizeof *timer); }
void TIM_InternalClockConfig(TIM_TypeDef *timer) { assert(timer == TIM3); }
void TIM_TimeBaseInit(TIM_TypeDef *timer, TIM_TimeBaseInitTypeDef *config)
{
    assert(timer == TIM3);
    mock_period = config->TIM_Period;
    mock_prescaler = config->TIM_Prescaler;
}
void TIM_OCStructInit(TIM_OCInitTypeDef *channel)
{ memset(channel, 0, sizeof *channel); }
void TIM_OC1Init(TIM_TypeDef *t, TIM_OCInitTypeDef *c)
{ assert(t == TIM3); mock_ccr[0] = c->TIM_Pulse; }
void TIM_OC2Init(TIM_TypeDef *t, TIM_OCInitTypeDef *c)
{ assert(t == TIM3); mock_ccr[1] = c->TIM_Pulse; }
void TIM_OC3Init(TIM_TypeDef *t, TIM_OCInitTypeDef *c)
{ assert(t == TIM3); mock_ccr[2] = c->TIM_Pulse; }
void TIM_OC4Init(TIM_TypeDef *t, TIM_OCInitTypeDef *c)
{ assert(t == TIM3); mock_ccr[3] = c->TIM_Pulse; }
void TIM_Cmd(TIM_TypeDef *timer, FunctionalState state)
{ assert(timer == TIM3 && state == ENABLE); }
void TIM_SetCompare1(TIM_TypeDef *t, uint16_t value)
{ assert(t == TIM3); mock_ccr[0] = value; }
void TIM_SetCompare2(TIM_TypeDef *t, uint16_t value)
{ assert(t == TIM3); mock_ccr[1] = value; }
void TIM_SetCompare3(TIM_TypeDef *t, uint16_t value)
{ assert(t == TIM3); mock_ccr[2] = value; }
void TIM_SetCompare4(TIM_TypeDef *t, uint16_t value)
{ assert(t == TIM3); mock_ccr[3] = value; }
void delay_init(void) {}
void delay_us(uint16_t us) { (void)us; }
void delay_ms(uint16_t ms) { (void)ms; }
