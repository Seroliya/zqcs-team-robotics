#include "motor.h"
#include "pwm.h"
#include "stm32f10x.h"

#define MOTOR_DIRECTION_PINS (GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | \
                              GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_3 | GPIO_Pin_2)

void motor_init(void)
{
    GPIO_InitTypeDef gpio;
    /* TODO_DRV8833：仍是 L298N 的 8 个方向引脚，未来按新接口重配。 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_ResetBits(GPIOA, MOTOR_DIRECTION_PINS);
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = MOTOR_DIRECTION_PINS;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
}

uint8_t motor_joystick_is_centered(uint8_t value)
{
    return value >= JOYSTICK_CENTER - JOYSTICK_DEADZONE &&
           value <= JOYSTICK_CENTER + JOYSTICK_DEADZONE;
}

/* 0～255 -> -1～1，沿用原方向：原始值小于中心时为正。
 * 回中区为 0；从死区边缘连续增长，端点 0/255 对应 +1/-1。
 */
static float motor_map(float value)
{
    float offset;
    if (!(value >= 0.0f && value <= 255.0f))
    {
        return 0.0f; /* 拒绝非法数值，包括 NaN。 */
    }
    offset = JOYSTICK_CENTER - value;
    if (offset > JOYSTICK_DEADZONE)
    {
        return (offset - JOYSTICK_DEADZONE) /
               (JOYSTICK_CENTER - JOYSTICK_DEADZONE);
    }
    if (offset < -JOYSTICK_DEADZONE)
    {
        return (offset + JOYSTICK_DEADZONE) /
               (255 - JOYSTICK_CENTER - JOYSTICK_DEADZONE);
    }
    return 0.0f;
}

static float absolute_value(float value)
{
    return value < 0.0f ? -value : value;
}

/* TODO_DRV8833：L298N 用 IN1/IN2 控方向，另有 EN 接收 PWM。
 * DRV8833 的双输入控制不同，不能原样复用这段输出。
 */
static void l298n_set_direction(uint16_t pin1, uint16_t pin2, float output)
{
    GPIO_ResetBits(GPIOA, pin1 | pin2);
    if (output > 0.0f)
    {
        GPIO_SetBits(GPIOA, pin1);
    }
    else if (output < 0.0f)
    {
        GPIO_SetBits(GPIOA, pin2);
    }
    /* 零输出时两个方向引脚保持低。 */
}

void motor_stop(void)
{
    /* TODO_DRV8833：重做新驱动的滑行/制动输入组合。
     * 当前 EN=0 只撤掉驱动，不保证车轮瞬间静止。
     */
    pwm_stop_all();
    GPIO_ResetBits(GPIOA, MOTOR_DIRECTION_PINS);
}

void motor(float joystick_forward, float joystick_sideways, float joystick_turn)
{
    float forward = motor_map(joystick_forward);
    float sideways = motor_map(joystick_sideways);
    float turn = motor_map(joystick_turn) * MOTOR_ROTATION_SCALE;
    float output[4];
    float largest = 1.0f;
    float magnitude;
    uint8_t wheel;

    /* 沿用原四轮组合和编号，实际轮位需架空确认。
     * 这是开环输出比例，不是编码器测得的轮速，没有 PID。
     */
    output[0] = (forward + sideways) / 2.0f - turn;
    output[1] = (forward - sideways) / 2.0f - turn;
    output[2] = (forward - sideways) / 2.0f + turn;
    output[3] = (forward + sideways) / 2.0f + turn;

    /* 超过 1 时四轮一起缩小，保留四轮之间的比例。 */
    for (wheel = 0; wheel < 4; wheel++)
    {
        magnitude = absolute_value(output[wheel]);
        if (magnitude > largest)
        {
            largest = magnitude;
        }
    }
    for (wheel = 0; wheel < 4; wheel++)
    {
        output[wheel] /= largest;
    }

    /* TODO_DRV8833：下面整段是将输出落到旧驱动上的位置。
     * 先关 EN，再改方向，最后给新 PWM，避免用旧占空比切换方向。
     * 尚未实现电机反转缓冲/斜坡，相关参数需实车再定。
     */
    pwm_stop_all();
    l298n_set_direction(GPIO_Pin_8, GPIO_Pin_9, output[0]);
    l298n_set_direction(GPIO_Pin_10, GPIO_Pin_11, output[1]);
    l298n_set_direction(GPIO_Pin_5, GPIO_Pin_4, output[2]);
    l298n_set_direction(GPIO_Pin_3, GPIO_Pin_2, output[3]);
    pwm_set1(absolute_value(output[0]));
    pwm_set2(absolute_value(output[1]));
    pwm_set3(absolute_value(output[2]));
    pwm_set4(absolute_value(output[3]));
}
