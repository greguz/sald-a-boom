#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "spi.h"
#include "ff.h"

#include "buttons.h"
#include "debug.h"
#include "led.h"
#include "player.h"
#include "pressure.h"

// No buttons pressed.
#define PRESSED_NOTHING 0x00

// Button 4 and 5
#define PRESSED_BANKS 0x18

// Button 1 and 8
#define PRESSED_MODES 0x81

// Track filename.
// 12 ASCII chars plus null terminator.
// Example: TRACK_13.WAV (bank 1, third button pressed)
char filename[13] = "TRACK_99.WAV";

// From 1 to 8 (matches number of buttons).
volatile uint8_t bank_number = 1;

// Like a guitar.
uint8_t get_track_number(uint8_t buttons) {
    if (buttons == PRESSED_NOTHING) {
        return 0;
    }
    if (buttons & 0x80) {
        return 8;
    }
    if (buttons & 0x40) {
        return 7;
    }
    if (buttons & 0x20) {
        return 6;
    }
    if (buttons & 0x10) {
        return 5;
    }
    if (buttons & 0x08) {
        return 4;
    }
    if (buttons & 0x04) {
        return 3;
    }
    if (buttons & 0x02) {
        return 2;
    }
    if (buttons & 0x01) {
        return 1;
    }
}

void set_filename(uint8_t track_number) {
    if (bank_number >= 1 && bank_number <= 8 && track_number >= 0 && track_number <= 8) {
        sprintf(filename, "TRACK_%u%u.WAV", bank_number, track_number);
    } else {
        strcpy(filename, "TRACK_99.WAV");
    }
}

void play_track(uint8_t buttons) {
    set_filename(get_track_number(buttons));
    DEBUG_PRINTF("play %s file\n", filename);
    play_wave(filename);
}

void mode_keyboard(uint8_t buttons, bool changed) {
    if (buttons == PRESSED_NOTHING) {
        stop_player();
    } else if (changed || !is_playing()) {
        play_track(buttons);
    }
}

void mode_pressure(uint8_t buttons, bool changed) {
    if (!has_pressure()) {
        stop_player();
    } else if (changed || !is_playing()) {
        play_track(buttons);
    }
}

int main() {
    stdio_init_all();

    // Hardware initialization
    init_buttons();
    init_led();
    init_pressure();

    // Application bootstrap
    enable_led();

    // Bootstrap failed (LED blinks)
    if (!init_player()) {
        while (true) {
            DEBUG_PRINTF("bootstrap failed\n");

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

    uint8_t prev = PRESSED_NOTHING;
    uint8_t next = PRESSED_NOTHING;
    bool changed = false;

    while (true) {
        poll_player();

        next = read_buttons();
        changed = next != prev;
        if (changed) {
            DEBUG_PRINTF("buttons changed: %x\n", next);
            prev = next;
        }

        mode_pressure(next, changed);
        // TODO: mode_keyboard(next, changed);
    }
}
