#include "pico/stdlib.h"

#include "buttons.h"

static void init_input(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

static void init_output(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, 1);
}

void init_buttons(buttons_t *buttons) {
    init_input(GPIO_ROW_0);
    init_input(GPIO_ROW_1);
    init_input(GPIO_ROW_2);
    init_input(GPIO_ROW_3);

    init_output(GPIO_COL_0);
    init_output(GPIO_COL_1);

    buttons->state = 0x00;
    buttons->previous = 0x00;
    buttons->pressed = false;
    buttons->released = false;
    buttons->changed = false;
}

void poll_buttons(buttons_t *buttons) {
    // Debounce date
    static absolute_time_t date;
    if (is_nil_time(date)) {
        date = delayed_by_ms(get_absolute_time(), BUTTONS_DEBOUNCE);
    }

    // previous state
    static uint8_t raw_old = 0x00;

    // current state
    uint8_t raw_new = 0x00;

    // previous state XOR current state
    uint8_t mask;

    // Reset one-shot events
    buttons->changed = false;
    buttons->pressed = false;
    buttons->released = false;

    // First output
    gpio_put(GPIO_COL_0, 0);
    sleep_ms(BUTTONS_SLEEP);
    if (!gpio_get(GPIO_ROW_3)) {
        raw_new = 0x08;
    } else if (!gpio_get(GPIO_ROW_2)) {
        raw_new = 0x04;
    } else if (!gpio_get(GPIO_ROW_1)) {
        raw_new = 0x02;
    } else if (!gpio_get(GPIO_ROW_0)) {
        raw_new = 0x01;
    }
    gpio_put(GPIO_COL_0, 1);

    // Second output
    gpio_put(GPIO_COL_1, 0);
    sleep_ms(BUTTONS_SLEEP);
    if (!gpio_get(GPIO_ROW_3)) {
        raw_new |= 0x80;
    } else if (!gpio_get(GPIO_ROW_2)) {
        raw_new |= 0x40;
    } else if (!gpio_get(GPIO_ROW_1)) {
        raw_new |= 0x20;
    } else if (!gpio_get(GPIO_ROW_0)) {
        raw_new |= 0x10;
    }
    gpio_put(GPIO_COL_1, 1);

    // State update
    if (raw_new != raw_old) {
        raw_old = raw_new;
        date = delayed_by_ms(get_absolute_time(), BUTTONS_DEBOUNCE);
    } else if (raw_new != buttons->state && time_reached(date)) {
        buttons->previous = buttons->state;
        buttons->state = raw_new;

        mask = buttons->previous ^ buttons->state;

        buttons->changed = true;
        buttons->pressed = (buttons->state & mask) > 0;
        buttons->released = (buttons->previous & mask) > 0;
    }
}
