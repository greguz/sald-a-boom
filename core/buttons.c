#include "pico/stdlib.h"

#include "buttons.h"

void init_button_input(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

void init_button_output(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, 1);
}

void init_buttons() {
    init_button_input(PIN_ROW_0);
    init_button_input(PIN_ROW_1);
    init_button_input(PIN_ROW_2);
    init_button_input(PIN_ROW_3);

    init_button_output(PIN_COL_0);
    init_button_output(PIN_COL_1);
}

uint8_t read_buttons() {
    uint8_t buttons = 0x00;

    gpio_put(PIN_COL_0, 0);
    sleep_ms(BUTTONS_SLEEP);

    if (!gpio_get(PIN_ROW_3)) {
        buttons = 0x08;
    } else if (!gpio_get(PIN_ROW_2)) {
        buttons = 0x04;
    } else if (!gpio_get(PIN_ROW_1)) {
        buttons = 0x02;
    } else if (!gpio_get(PIN_ROW_0)) {
        buttons = 0x01;
    }

    gpio_put(PIN_COL_0, 1);
    gpio_put(PIN_COL_1, 0);
    sleep_ms(BUTTONS_SLEEP);

    if (!gpio_get(PIN_ROW_3)) {
        buttons |= 0x80;
    } else if (!gpio_get(PIN_ROW_2)) {
        buttons |= 0x40;
    } else if (!gpio_get(PIN_ROW_1)) {
        buttons |= 0x20;
    } else if (!gpio_get(PIN_ROW_0)) {
        buttons |= 0x10;
    }

    gpio_put(PIN_COL_1, 1);
    sleep_ms(BUTTONS_SLEEP);

    return buttons;
}
