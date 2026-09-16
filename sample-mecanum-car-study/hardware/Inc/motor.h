#ifndef MOTOR_H
#define MOTOR_H
#include <stdint.h>

/* 实测回中范围后再调整；默认 120～136（含端点）均视为回中。 */
#define JOYSTICK_CENTER       128
#define JOYSTICK_DEADZONE     8
#define MOTOR_ROTATION_SCALE  0.4f

#if JOYSTICK_DEADZONE < 0 || JOYSTICK_DEADZONE >= 127
#error JOYSTICK_DEADZONE must be between 0 and 126
#endif

void motor_init(void);
void motor_stop(void);
uint8_t motor_joystick_is_centered(uint8_t value);
/* 参数仍是 0～255 的原始摇杆值，不是转速；正方向沿用原项目。 */
void motor(float joystick_forward, float joystick_sideways, float joystick_turn);
#endif
