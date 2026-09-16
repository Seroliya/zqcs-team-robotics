#ifndef PWM_H
#define PWM_H
/* 暂时保留原频率，不替电机选择新参数。
 * 正常时钟配置下 TIM3 时钟为 72 MHz：
 * PWM频率 = 72000000 / 72 / 20000 = 50 Hz，原注释的 kHz 数值不正确。
 * 寄存器 ARR = 周期计数数 - 1；PSC = 分频倍数 - 1。
 * TODO_DRV8833：结合 TT 电机、驱动板和实测重新选择频率与引脚。
 */
#define PWM_PERIOD_COUNTS  20000U
#define PWM_PRESCALER_DIV  72U
/* CCR 需要能表示 PERIOD_COUNTS，才能得到精确的 100% 占空比。 */
#if PWM_PERIOD_COUNTS < 1 || PWM_PERIOD_COUNTS > 65535
#error PWM_PERIOD_COUNTS must fit a 16-bit compare register
#endif
#if PWM_PRESCALER_DIV < 1 || PWM_PRESCALER_DIV > 65536
#error PWM_PRESCALER_DIV is outside the timer range
#endif

void pwm_init(void);
void pwm_stop_all(void);
/* duty 是 0.0～1.0；负值/NaN 归零，超过 1 的值限制为 1。 */
void pwm_set1(float duty);
void pwm_set2(float duty);
void pwm_set3(float duty);
void pwm_set4(float duty);
#endif
