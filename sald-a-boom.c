#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "spi.h"
#include "ff.h"

#include "buttons.h"
#include "debug.h"
#include "knob.h"
#include "led.h"
#include "player.h"
#include "pressure.h"

// No buttons pressed.
#define PRESSED_NOTHING 0x00

// Button 4 and 5
#define PRESSED_BANKS   0x18

// Button 1 and 8
#define PRESSED_MODES   0x81

// Milliseconds period between volume adjustments
#define KNOB_INTERVAL   100

// Play this track to hear a duck!
#define DUCK_TRACK      42

// See "mode_keyboard()" function comment.
#define MODE_KEYBOARD   0x01

// See "mode_sax()" function comment.
#define MODE_SAX        0x02

//
#define MODE_SELECT     0x81

//
#define BANK_SELECT     42

// milliseconds
#define CONFIG_TIMEOUT  5000

// Currently enabled mode
volatile uint8_t current_mode = 0x00;

// From 1 to 8 (matches number of buttons).
volatile uint8_t current_bank = 1;

// Wait for current audio to finish its playback.
// Also ignores any button change.
volatile bool wait_playback = false;

uint8_t get_track_number(uint8_t buttons) {
    if (buttons == PRESSED_NOTHING) {
        return 0;
    }

    // "last button pressed" selects the note, like a guitar
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

void play_track(uint8_t track_number) {
    // Track filename.
    // 12 ASCII chars plus null terminator.
    // Example: TRACK_13.WAV (bank 1, third button pressed)
    char filename[13] = "TRACK_99.WAV";

    // Banks from 1 to 8 (one bank per button)
    // Tracks from 0 to 9 (one track per button plus "no buttons pressed" special track)
    if (current_bank >= 1 && current_bank <= 8 && track_number >= 0 && track_number <= 8) {
        sprintf(filename, "TRACK_%u%u.WAV", current_bank, track_number);
    }

    DEBUG_PRINTF("play %s file\n", filename);
    play_wave(filename);
}

void play_duck(void) {
    wait_playback = true;
    play_track(DUCK_TRACK);
}

void poll_volume() {
    static absolute_time_t date;

    // TODO: remove debug code
    uint16_t a;
    uint8_t b;

    if (is_nil_time(date)) {
        date = delayed_by_ms(get_absolute_time(), KNOB_INTERVAL);
    }

    if (time_reached(date)) {
        date = delayed_by_ms(date, KNOB_INTERVAL);

        a = read_knob();
        b = a * 255 / 4095;
        // DEBUG_PRINTF("volume %i (%i)\n", a, b);

        set_volume(b);
    }
}

// The default operation mode.
// You press something, it will play something.
// You release the button, it will stop playing.
void mode_keyboard(uint8_t buttons, bool changed) {
    if (buttons == PRESSED_NOTHING) {
        stop_player();
    } else if (changed || !is_playing()) {
        play_track(get_track_number(buttons));
    }
}

// Sax mode.
// To play something you need to blow into the microphone.
// This mode adds the "zero" track (blowing without pressing any button).
// When you blow, it will play something.
// When you stop, it will pause the playback.
// If you don't change the buttons, blowing again will resume the track.
// Changing buttons will always reset the playback.
void mode_sax(uint8_t buttons, bool changed) {
    if (!has_pressure()) {
        pause_player();
    } else if (!changed && is_paused()) {
        resume_player();
    } else if (!is_playing()) {
        play_track(get_track_number(buttons));
    }
}

// Select which bank is enabled (tracklist selection).
void select_bank(uint8_t buttons) {
    switch (buttons) {
        case 0x01:
            current_bank = 1;
            break;
        case 0x02:
            current_bank = 2;
            break;
        case 0x04:
            current_bank = 3;
            break;
        case 0x08:
            current_bank = 4;
            break;
        case 0x10:
            current_bank = 5;
            break;
        case 0x20:
            current_bank = 6;
            break;
        case 0x40:
            current_bank = 7;
            break;
        case 0x80:
            current_bank = 8;
            break;
    }

    if (current_bank != BANK_SELECT) {
        play_duck();
    }
}

// Select which playback mode to use.
void select_mode(uint8_t buttons) {
    switch (buttons) {
        case 0x01:
            current_mode = MODE_KEYBOARD;
            break;
        case 0x02:
            current_mode = MODE_SAX;
            break;
    }

    if (current_mode != MODE_SELECT) {
        play_duck();
    }
}

void sald_a_boom(uint8_t new_buttons) {
    static absolute_time_t date;
    static uint8_t old_buttons = 0x00;
    if (is_nil_time(date)) {
        date = delayed_by_ms(get_absolute_time(), CONFIG_TIMEOUT);
    }

    bool changed = new_buttons != old_buttons;
    if (changed) {
        DEBUG_PRINTF("buttons changed: %x\n", new_buttons);
        date = delayed_by_ms(get_absolute_time(), CONFIG_TIMEOUT);
    }

    if (new_buttons == PRESSED_BANKS && time_reached(date)) {
        DEBUG_PRINTF("entered bank selection mode\n");
        current_bank = BANK_SELECT;
        play_duck();
    } else if (new_buttons == PRESSED_MODES && time_reached(date)) {
        DEBUG_PRINTF("entered mode selection mode\n");
        current_mode = MODE_SELECT;
        play_duck();
    } else {
        switch (current_mode) {
            case MODE_SAX:
                mode_sax(new_buttons, changed);
                break;
            default:
                mode_keyboard(new_buttons, changed);
                break;
        }
    }

    old_buttons = new_buttons;
}

int main() {
    uint8_t buttons = 0x00;

    stdio_init_all();

    // Hardware initialization
    init_buttons();
    init_knob();
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

    while (true) {
        poll_player();
        poll_volume();

        if (wait_playback) {
            if (is_playing()) {
                continue;
            } else {
                wait_playback = false;
            }
        }

        buttons = read_buttons();
        if (current_bank == BANK_SELECT) {
            select_bank(buttons);
        } else if (current_mode == MODE_SELECT) {
            select_mode(buttons);
        } else {
            sald_a_boom(buttons);
        }
    }
}
