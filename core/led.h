#pragma once

#include <stdbool.h>

#define GPIO_LED 25

void init_led(void);

void enable_led(void);

void disable_led(void);

bool led_enabled(void);
