#include "pico/stdlib.h"

#include "buttons.h"

// Buttons grid digital inputs (pullup)
#define GPIO_ROW_0          10
#define GPIO_ROW_1          11
#define GPIO_ROW_2          12
#define GPIO_ROW_3          13

// Buttons grid digital outputs
#define GPIO_COL_0          14
#define GPIO_COL_1          15

button_t buttons[BUTTONS_COUNT];

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

void init_buttons(void) {
    for (int i = 0; i < BUTTONS_COUNT; i++) {
        init_button(&buttons[i]);
    }

    init_input(GPIO_ROW_0);
    init_input(GPIO_ROW_1);
    init_input(GPIO_ROW_2);
    init_input(GPIO_ROW_3);

    init_output(GPIO_COL_0);
    init_output(GPIO_COL_1);
}

static void poll_gpio(button_t *button, uint gpio) {
    poll_button(button, !gpio_get(gpio));
}

void poll_buttons(void) {
    // First output
    gpio_put(GPIO_COL_0, 0);
    sleep_ms(MATRIX_SLEEP);
    poll_gpio(&buttons[0], GPIO_ROW_0);
    poll_gpio(&buttons[1], GPIO_ROW_1);
    poll_gpio(&buttons[2], GPIO_ROW_2);
    poll_gpio(&buttons[3], GPIO_ROW_3);
    gpio_put(GPIO_COL_0, 1);

    // Second output
    gpio_put(GPIO_COL_1, 0);
    sleep_ms(MATRIX_SLEEP);
    poll_gpio(&buttons[4], GPIO_ROW_0);
    poll_gpio(&buttons[5], GPIO_ROW_1);
    poll_gpio(&buttons[6], GPIO_ROW_2);
    poll_gpio(&buttons[7], GPIO_ROW_3);
    gpio_put(GPIO_COL_1, 1);
}

bool buttons_changed(void) {
    for (int i = 0; i < BUTTONS_COUNT; i++) {
        if (button_changed(&buttons[i])) {
            return true;
        }
    }
    return false;
}
