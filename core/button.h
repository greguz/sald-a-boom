#pragma once

#include <stdbool.h>
#include "pico/time.h"

// milliseconds
#define BTN_DEBOUNCE 25

// Abstract debounced button.
typedef struct {
    // Current stable button state (true if pressed).
    bool state;

    // One-shot event since last poll.
    bool changed;

    // Unstable button state (hardware level).
    bool raw_state;

    // Debounced validity of `raw_state`.
    absolute_time_t raw_valid;
} button_t;

void init_button(button_t *button);

void poll_button(button_t *button, bool raw_state);

static inline bool button_pressed(const button_t *button) {
    return button->state;
}

static inline bool button_changed(const button_t *button) {
    return button->changed;
}

static inline bool button_was_pressed(const button_t *button) {
    return button_changed(button) && button_pressed(button);
}

static inline bool button_was_released(const button_t *button) {
    return button_changed(button) && !button_pressed(button);
}
