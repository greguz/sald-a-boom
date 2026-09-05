#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "spi.h"
#include "ff.h"

#include "buttons.h"
#include "led.h"
#include "player.h"

int main() {
    stdio_init_all();

    // Hardware initialization
    init_buttons();
    init_led();

    // Application bootstrap
    enable_led();

    bool ok = init_player();

    // Bootstrap failed (LED blinks)
    if (!ok) {
        while (true) {
            printf("f_mount failed");

            if (led_enabled()) {
                disable_led();
            } else {
                enable_led();
            }
            sleep_ms(2000);
        }
    }

    // Bootstrap completed (run the application)
    disable_led();

    // uint8_t buttons = 0x00;
    while (true) {
        poll_player();

        if (!is_playing()) {
            play_wave("AIHB.WAV");
            // play_wave("NOPE.WAV");
        }

        // buttons = read_buttons();
        // if (buttons > 0 && !is_playing()) {
        //     play_audio("TRACK_01.WAV");
        // }
    }
}
