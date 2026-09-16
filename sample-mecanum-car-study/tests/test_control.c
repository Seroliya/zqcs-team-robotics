#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mock_hardware.h"

/* 将真正的主程序编入测试；不运行无限循环，逐次调用同一 control_step。 */
#define main firmware_main
#include "../hardware/Src/main.c"
#undef main

static void assert_stopped(void)
{
    assert(mock_ccr[0] == 0 && mock_ccr[1] == 0 &&
           mock_ccr[2] == 0 && mock_ccr[3] == 0);
    assert(mock_gpio_a == 0);
}
static void run_frame(uint8_t forward, uint8_t sideways, uint8_t turn,
                      uint16_t pressed, uint8_t mode, uint8_t marker)
{
    uint8_t frame[9] = {0xFF, mode, marker, (uint8_t)~pressed,
                       (uint8_t)~(pressed >> 8), turn, 128, sideways, forward};
    mock_frame(frame, 9);
    control_step();
    mock_assert_poll_complete();
}
static void normal(uint8_t forward, uint8_t sideways, uint8_t turn, uint16_t pressed)
{ run_frame(forward, sideways, turn, pressed, 0x73, 0x5A); }

static void test_pwm(void)
{
    unsigned ch, i;
    const float input[] = {0.0f, -0.01f, 0.00001f, 0.5f, 1.0f, 1.4f,
                           NAN, INFINITY, -INFINITY};
    const uint16_t expected[] = {0, 0, 0, 10000, 20000, 20000, 0, 20000, 0};
    for (i = 0; i < sizeof input / sizeof input[0]; i++)
    {
        pwm_set1(input[i]); pwm_set2(input[i]);
        pwm_set3(input[i]); pwm_set4(input[i]);
        for (ch = 0; ch < 4; ch++) assert(mock_ccr[ch] == expected[i]);
    }
    motor_stop();
    puts("PASS: PWM zero, fractional, full-scale, overflow, NaN/Inf");
}

static void assert_wheels(int a, int b, int c, int d)
{
    const int expected[4] = {a,b,c,d};
    const uint16_t positive[4] = {GPIO_Pin_8, GPIO_Pin_10, GPIO_Pin_5, GPIO_Pin_3};
    const uint16_t negative[4] = {GPIO_Pin_9, GPIO_Pin_11, GPIO_Pin_4, GPIO_Pin_2};
    unsigned i;
    for (i = 0; i < 4; i++)
    {
        int signed_value = (int)mock_ccr[i];
        if (mock_gpio_a & negative[i]) signed_value = -signed_value;
        if (expected[i] > 0) assert(mock_gpio_a & positive[i]);
        if (expected[i] < 0) assert(mock_gpio_a & negative[i]);
        assert(abs(signed_value - expected[i]) <= 1);
    }
}

static void test_motor(void)
{
    unsigned i, j, k, axis;
    const uint8_t values[] = {0,1,64,119,120,127,128,129,136,137,192,254,255};
    motor(0,128,128); assert_wheels(10000,10000,10000,10000);
    motor(255,128,128); assert_wheels(-10000,-10000,-10000,-10000);
    motor(128,0,128); assert_wheels(10000,-10000,-10000,10000);
    motor(128,128,0); assert_wheels(-8000,-8000,8000,8000);
    /* 未限幅时是 0.6,-0.4,0.4,1.4；应整体除以 1.4，不能逐轮截断。 */
    motor(0,0,0); assert_wheels(8571,-5714,5714,20000);

    for (i = 0; i < 256; i++)
    {
        assert(motor_joystick_is_centered((uint8_t)i) == (i >= 120 && i <= 136));
        motor((float)i, 128, 128);
        if (i >= 120 && i <= 136) assert_stopped();
    }
    motor(119,128,128); assert(mock_ccr[0] > 0);
    motor(137,128,128); assert(mock_ccr[0] > 0);
    motor(NAN,128,128); assert_stopped();
    for (i = 0; i < sizeof values; i++)
        for (j = 0; j < sizeof values; j++)
            for (k = 0; k < sizeof values; k++)
            {
                motor(values[i], values[j], values[k]);
                for (axis = 0; axis < 4; axis++) assert(mock_ccr[axis] <= 20000);
            }
    motor_stop();
    puts("PASS: axes, all 256 center checks, deadzone edges, 2197 mixed inputs, ratios");
}

static void test_ps2(void)
{
    ps2_data data;
    unsigned i;
    uint8_t frame[9] = {0xFF,0x73,0x5A,0xFE,0x7F,3,250,77,199};
    for (i = 0; i < 256; i++)
    {
        frame[5] = (uint8_t)i;
        mock_frame(frame, 9);
        assert(ps2_read(&data) == 1);
        mock_assert_poll_complete();
        assert(data.btn1 == 1 && data.btn2 == 128);
        assert(data.RJoy_LR == i && data.RJoy_UD == 250);
        assert(data.LJoy_LR == 77 && data.LJoy_UD == 199);
    }
    frame[2] = 0;
    mock_frame(frame,9);
    assert(ps2_read(&data) == 0);
    assert(data.mode == 0 && data.btn1 == 0 && data.btn2 == 0);
    assert(data.RJoy_LR == 128 && data.RJoy_UD == 128 &&
           data.LJoy_LR == 128 && data.LJoy_UD == 128);
    memset(frame,0xFF,9); mock_frame(frame,9); assert(ps2_read(&data) == 0);
    memset(frame,0,9); mock_frame(frame,9); assert(ps2_read(&data) == 0);
    assert(ps2_read(NULL) == 0);
    puts("PASS: LSB-first wire simulation, 256 byte values, frame layout, invalid clearing");
}

static void test_control(void)
{
    uint8_t button;
    const uint8_t modes[] = {0,0x41,0x79,0xFF};
    unsigned i;
    control_ready = 0;
    normal(0,128,128,0); assert_stopped(); /* 上电已推杆，不能启动 */
    normal(128,128,128,0); assert_stopped();
    normal(0,128,128,0); assert(mock_ccr[0] > 0);

    for (i = 0; i < sizeof modes; i++)
    {
        run_frame(0,128,128,0,modes[i],0x5A); assert_stopped();
        normal(0,128,128,0); assert_stopped(); /* 连上但未回中 */
        normal(128,128,128,1); assert_stopped(); /* 按键未松 */
        normal(128,128,128,0); assert_stopped();
        normal(0,128,128,0); assert(mock_ccr[0] > 0);
    }
    run_frame(0,128,128,0,0x73,0); assert_stopped();
    normal(128,128,128,0); assert_stopped();

    /* 16 个按键均应在松开时触发一次，按住/持续松开都不重复。 */
    for (button = 0; button < PS2_BUTTON_COUNT; button++)
    {
        last_released_button = 255;
        normal(128,128,128,(uint16_t)(1U << button));
        normal(128,128,128,(uint16_t)(1U << button));
        assert(last_released_button == 255);
        normal(128,128,128,0); assert(last_released_button == button);
        last_released_button = 255;
        normal(128,128,128,0); assert(last_released_button == 255);
    }
    /* 按住时丢帧，不能伪造松开事件；恢复后也不能触发旧动作。 */
    normal(0,128,128,(uint16_t)(1U << PS2_BUTTON_CROSS));
    last_released_button = 255;
    run_frame(0,128,128,0,0xFF,0xFF);
    assert_stopped(); assert(last_released_button == 255);
    normal(128,128,128,0);
    normal(128,128,128,0);
    assert(last_released_button == 255);
    puts("PASS: startup, invalid-frame stop, neutral rearm, 16 buttons, no phantom release");
}

int main(void)
{
    motor_init(); /* 检查独立开 GPIOA 时钟，不再依赖 pwm_init */
    pwm_init();
    ps2_init();
    assert(mock_period == 19999 && mock_prescaler == 71);
    assert_stopped();
    test_pwm();
    test_motor();
    test_ps2();
    test_control();
    puts("All host tests passed (not a hardware or Keil build test).");
    return 0;
}
