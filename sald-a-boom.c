#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "spi.h"
#include "ff.h"

#include "adc.h"
#include "buttons.h"
#include "debug.h"
#include "led.h"
#include "player.h"

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

// (milliseconds)
#define CONFIG_TIMEOUT  5000

// Currently enabled mode
static uint8_t current_mode = 0x00;

// From 1 to 8 (matches number of buttons).
static uint8_t current_bank = 1;

// Wait for current audio to finish its playback.
// Also ignores any button change.
static bool wait_playback = false;

// Buttons state.
static buttons_t buttons;

static uint8_t get_track_number(void) {
    // "last button pressed" selects the note, like a guitar
    if (buttons.state & 0x80) {
        return 8;
    }
    if (buttons.state & 0x40) {
        return 7;
    }
    if (buttons.state & 0x20) {
        return 6;
    }
    if (buttons.state & 0x10) {
        return 5;
    }
    if (buttons.state & 0x08) {
        return 4;
    }
    if (buttons.state & 0x04) {
        return 3;
    }
    if (buttons.state & 0x02) {
        return 2;
    }
    if (buttons.state & 0x01) {
        return 1;
    }
    return 0;
}

static void play_track(uint8_t track_number) {
    // Track filename.
    // 12 ASCII chars plus null terminator.
    // Example: TRACK_13.WAV (bank 1, third button pressed)
    TCHAR filename[13] = "TRACK_99.WAV";

    // Banks from 1 to 8 (one bank per button)
    // Tracks from 0 to 9 (one track per button plus "no buttons pressed" special track)
    if (current_bank >= 1 && current_bank <= 8 && track_number >= 0 && track_number <= 8) {
        sprintf(filename, "TRACK_%u%u.WAV", current_bank, track_number);
    }

    DEBUG_PRINTF("play %s file\n", filename);
    play_wave(filename);
}

static void play_duck(void) {
    wait_playback = true;
    play_track(DUCK_TRACK);
}

static void poll_volume(void) {
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
static void mode_keyboard(void) {
    if (buttons.state == PRESSED_NOTHING) {
        stop_player();
    } else if (buttons_changed(&buttons) || !is_playing()) {
        play_track(get_track_number());
    }
}

// Sax mode.
// To play something you need to blow into the microphone.
// This mode adds the "zero" track (blowing without pressing any button).
// When you blow, it will play something.
// When you stop, it will pause the playback.
// If you don't change the buttons, blowing again will resume the track.
// Changing buttons will always reset the playback.
static void mode_sax(void) {
    if (!has_pressure()) {
        pause_player();
    } else if (!buttons_changed(&buttons) && is_paused()) {
        resume_player();
    } else if (!is_playing()) {
        play_track(get_track_number());
    }
}

// Select which bank is enabled (tracklist selection).
static void select_bank(void) {
    if (!button_pressed(&buttons)) {
        return;
    }

    switch (buttons.state) {
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
        DEBUG_PRINTF("bank changed: %i\n", current_bank);
        play_duck();
    }
}

// Select which playback mode to use.
static void select_mode(void) {
    if (!button_pressed(&buttons)) {
        return;
    }

    switch (buttons.state) {
        case 0x01:
            current_mode = MODE_KEYBOARD;
            break;
        case 0x02:
            current_mode = MODE_SAX;
            break;
    }

    if (current_mode != MODE_SELECT) {
        DEBUG_PRINTF("mode changed: %i\n", current_mode);
        play_duck();
    }
}

static void sald_a_boom(void) {
    static absolute_time_t date;
    if (is_nil_time(date)) {
        date = delayed_by_ms(get_absolute_time(), CONFIG_TIMEOUT);
    }

    if (buttons_changed(&buttons)) {
        DEBUG_PRINTF("buttons changed: %x\n", buttons.state);
        date = delayed_by_ms(get_absolute_time(), CONFIG_TIMEOUT);
    }

    if (buttons.state == PRESSED_BANKS && time_reached(date)) {
        DEBUG_PRINTF("entered bank selection mode\n");
        current_bank = BANK_SELECT;
        play_duck();
    } else if (buttons.state == PRESSED_MODES && time_reached(date)) {
        DEBUG_PRINTF("entered mode selection mode\n");
        current_mode = MODE_SELECT;
        play_duck();
    } else {
        switch (current_mode) {
            case MODE_SAX:
                mode_sax();
                break;
            default:
                mode_keyboard();
                break;
        }
    }
}

int main(void) {
    stdio_init_all();

    // Hardware initialization
    init_adc();
    init_buttons(&buttons);
    init_led();

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
        poll_buttons(&buttons);

        if (wait_playback) {
            if (is_playing()) {
                continue;
            } else {
                wait_playback = false;
            }
        }

        if (current_bank == BANK_SELECT) {
            select_bank();
        } else if (current_mode == MODE_SELECT) {
            select_mode();
        } else {
            sald_a_boom();
        }
    }
}
