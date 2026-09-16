#ifndef MOCK_HARDWARE_H
#define MOCK_HARDWARE_H
#include "stm32f10x.h"
#include <stddef.h>
extern uint16_t mock_ccr[4];
extern uint16_t mock_gpio_a;
extern uint16_t mock_gpio_b;
extern uint32_t mock_apb2;
extern uint16_t mock_period;
extern uint16_t mock_prescaler;
void mock_frame(const uint8_t *bytes, size_t count);
void mock_assert_poll_complete(void);
#endif
