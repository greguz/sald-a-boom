#include "button.h"

void init_button(button_t *button) {
    button->state = false;
    button->changed = false;
    button->raw_state = false;
    button->raw_valid = get_absolute_time();
}

void poll_button(button_t *button, bool raw_state) {
    // Reset one-shot event
    button->changed = false;

    if (raw_state != button->raw_state) {
        // Raw signal changed: restart debounce timer
        button->raw_state = raw_state;
        button->raw_valid = delayed_by_ms(get_absolute_time(), BTN_DEBOUNCE);
    } else if (raw_state != button->state && time_reached(button->raw_valid)) {
        // Raw signal has remained stable
        button->state = raw_state;
        button->changed = true;
    }
}
