#include "hardware/adc.h"
#include "pico/time.h"

#include "adc.h"
#include "debug.h"

#define ADC_INPUT(gpio) (gpio - 26)

// The Pico's ADC is 12-bit: 0 to 4095
#define ADC_MAX 4095

void init_adc(void) {
    adc_init();

    adc_gpio_init(GPIO_PRESSURE);
    adc_gpio_init(GPIO_KNOB);
}

bool has_pressure(void) {
    // Returning value
    static bool res = false;

    // Last update date
    static absolute_time_t date;

    // Min/Max value since last update
    static uint16_t min = ADC_MAX;
    static uint16_t max = 0;

    // Initialize date time at first run
    if (is_nil_time(date)) {
        date = delayed_by_ms(get_absolute_time(), PRESSURE_INTERVAL);
    }

    // Update Min/Max
    adc_select_input(ADC_INPUT(GPIO_PRESSURE));
    uint16_t value = adc_read();
    if (value < min) {
        min = value;
    }
    if (value > max) {
        max = value;
    }

    // Handle time tick
    if (time_reached(date)) {
        // Shift date
        date = delayed_by_ms(date, PRESSURE_INTERVAL);

        // Update pressure status
        res = (max - min) >= PRESSURE_THRESHOLD;

        // TODO: ugly
        if (res) {
            DEBUG_PRINTF("%u\n", (unsigned)(max - min));
        }

        // Reset delta
        min = ADC_MAX;
        max = 0;
    }

    return res;
}

uint16_t read_knob(void) {
    adc_select_input(ADC_INPUT(GPIO_KNOB));
    return adc_read();
}
