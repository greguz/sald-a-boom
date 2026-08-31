#pragma once

#include <stdbool.h>
#include <stdint.h>

// Milliseconds
#define BUTTONS_SLEEP   1

// Buttons grid digital inputs (pullup)
#define PIN_ROW_0       10
#define PIN_ROW_1       11
#define PIN_ROW_2       12
#define PIN_ROW_3       13

// Buttons grid digital outputs
#define PIN_COL_0       14
#define PIN_COL_1       15

void init_buttons();

uint8_t read_buttons();
