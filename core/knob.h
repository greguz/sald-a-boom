#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "debug.h"

#define PIN_KNOB 27

#define ADC_KNOB PIN_KNOB - 26

void init_knob(void) {
    adc_init();

    adc_gpio_init(PIN_KNOB);
}

// Returns an integer value between 0 and 4095 (12 bit).
uint16_t read_knob(void) {
    adc_select_input(ADC_KNOB);
    return adc_read();
}
