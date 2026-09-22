#pragma once

#include <stdbool.h>
#include <stdint.h>

// Sleep between output state change (milliseconds)
#define BUTTONS_SLEEP       1

// Stable buttons state debounce (milliseconds)
#define BUTTONS_DEBOUNCE    20

// Buttons grid digital inputs (pullup)
#define GPIO_ROW_0          10
#define GPIO_ROW_1          11
#define GPIO_ROW_2          12
#define GPIO_ROW_3          13

// Buttons grid digital outputs
#define GPIO_COL_0          14
#define GPIO_COL_1          15

typedef struct {
    // Previous stable buttons state.
    uint8_t previous;

    // Current stable buttons state.
    uint8_t state;

    // One-shot event: new buttons state available.
    bool changed;

    // One-shot event: a button was pressed (any button).
    bool pressed;

    // One-shot event: a button was released (any button).
    bool released;
} buttons_t;

void init_buttons(buttons_t *buttons);

void poll_buttons(buttons_t *buttons);

static inline bool buttons_changed(const buttons_t *buttons) {
    return buttons->changed;
}

static inline bool button_pressed(const buttons_t *buttons) {
    return buttons->pressed;
}

static inline bool button_released(const buttons_t *buttons) {
    return buttons->released;
}
