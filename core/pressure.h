#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "debug.h"

// MAX4466 electret microphone board
#define PIN_PRESSURE 26

// The Pico's ADC is 12-bit: 0 to 4095
#define PRESSURE_MAX 4095

// Time window in milliseconds
#define PRESSURE_WINDOW 50

// Delta value threshold inside time window
#define PRESSURE_THRESHOLD 200

void init_pressure(void) {
    adc_init();

    adc_gpio_init(PIN_PRESSURE);
    adc_select_input(0); // Just using the first channel
}

uint32_t millis(void) {
    return to_ms_since_boot(get_absolute_time());
}

bool has_pressure(void) {
    // Returning value
    static bool res = false;

    // Last update date
    static absolute_time_t date;

    // Min/Max value since last update
    static uint16_t min = PRESSURE_MAX;
    static uint16_t max = 0;

    // Initialize date time at first run
    if (is_nil_time(date)) {
        date = delayed_by_ms(get_absolute_time(), PRESSURE_WINDOW);
    }

    // Update Min/Max
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
        date = delayed_by_ms(date, PRESSURE_WINDOW);

        // Update pressure status
        res = (max - min) >= PRESSURE_THRESHOLD;

        // TODO: ugly
        if (res) {
            DEBUG_PRINTF("%u\n", (unsigned)(max - min));
        }

        // Reset delta
        min = PRESSURE_MAX;
        max = 0;
    }

    return res;
}
