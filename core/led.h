#define PIN_LED 25

void init_led(void) {
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);
    gpio_put(PIN_LED, 0);
}

void enable_led(void) {
    gpio_put(PIN_LED, 1);
}

void disable_led(void) {
    gpio_put(PIN_LED, 0);
}

bool led_enabled(void) {
    return gpio_get_out_level(PIN_LED);
}
