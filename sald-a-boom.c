#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "spi.h"
#include "ff.h"

#include "buttons.h"
#include "player.h"

int main() {
    stdio_init_all();

    // TODO: LED is on (boot)

    init_buttons();

    bool ok = init_player();

    // TODO: if ok LED is off, otherwise loop

    // uint8_t buttons = 0x00;

    bool playing = false;

    while (true) {

        playing = poll_player();

        if (!ok) {
            printf("f_mount failed");
        } else if (!playing) {
            // play_wave("AIHB.WAV");
            play_wave("NOPE.WAV");
        }

        // buttons = read_buttons();
        // if (buttons > 0 && !is_playing()) {
        //     play_audio("TRACK_01.WAV");
        // }

    }
}
