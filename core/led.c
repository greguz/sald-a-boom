#include "led.h"

#include "hardware/gpio.h"

void init_led(void) {
    gpio_init(GPIO_LED);
    gpio_set_dir(GPIO_LED, GPIO_OUT);
    gpio_put(GPIO_LED, 0);
}

void enable_led(void) {
    gpio_put(GPIO_LED, 1);
}

void disable_led(void) {
    gpio_put(GPIO_LED, 0);
}

bool led_enabled(void) {
    return gpio_get_out_level(GPIO_LED);
}
