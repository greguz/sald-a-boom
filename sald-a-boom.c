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

// Milliseconds period between volume adjustments
#define KNOB_INTERVAL   100

// Playback mode: see "mode_keyboard()" function comment.
#define MODE_KEYBOARD   0

// Playback mode: see "mode_sax()" function comment.
#define MODE_SAX        1

// Playback mode: see "mode_toggle()" function comment.
#define MODE_TOGGLE     2

// Special mode: choose active mode.
#define MODE_SELECT     -24

// Special mode: choose active bank.
#define BANK_SELECT     -42

// Buttons press history size.
// This will also define secret sequences size.
#define HISTORY_SIZE    8

// Pressed buttons history.
// First item is the latest pressed button.
static int history[HISTORY_SIZE];

// Buttons sequence to enter into bank selection state.
static const int bank_sequence[HISTORY_SIZE] = {1, 1, 8, 8, 1, 1, 8, 8};

// Buttons sequence to enter into mode selection state.
static const int mode_sequence[HISTORY_SIZE] = {4, 4, 5, 5, 4, 4, 5, 5};

// Wait for current audio to finish its playback.
// Also ignores any button change.
static bool wait_playback = false;

// Currently enabled mode.
static int current_mode = MODE_KEYBOARD;

// First bank (first button).
static int current_bank = 1;

static void play_track(int track_number) {
    TCHAR filename[22] = "/ERROR.WAV";

    if (current_bank >= 0 && current_bank < 100 && track_number >= 0 && track_number < 100) {
        snprintf(
            filename,
            sizeof(filename),
            "/BANK_%02d/TRACK_%02d.WAV", current_bank, track_number
        );
    }

    DEBUG_PRINTF("play %s file\n", filename);
    play_wave(filename);
}

static void play_notification(void) {
    TCHAR filename[12] = "/NOTIFY.WAV";
    wait_playback = true;
    play_wave(filename);
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

static int get_track_number(void) {
    // "last button pressed" selects the note, like a guitar
    for (int n = BUTTONS_COUNT; n > 0; n--) {
        if (button_pressed(&buttons[n - 1])) {
            return n;
        }
    }
    return 0;
}

// The default operation mode.
// You press something, it will play something.
// You release the button, it will stop playing.
static void mode_keyboard(void) {
    int track_number = get_track_number();

    if (track_number == 0) {
        stop_player();
    } else if (buttons_changed() || !is_playing()) {
        play_track(track_number);
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
    } else if (!buttons_changed() && is_paused()) {
        resume_player();
    } else if (!is_playing()) {
        play_track(get_track_number());
    }
}

// Toggle mode (pause/resume).
// First click: start track.
// Second click on same button: pause track.
// Third click on same button: resume track.
// Change button: start new track.
static void mode_toggle(void) {
    static int current_track = -1;

    for (int n = 1; n <= BUTTONS_COUNT; n++) {
        if (button_was_pressed(&buttons[n - 1])) {
            if (current_track == n && is_playing()) {
                DEBUG_PRINTF("pause track\n");
                pause_player();
            } else if (current_track == n && is_paused()) {
                DEBUG_PRINTF("resume track\n");
                resume_player();
            } else {
                play_track(n);
            }

            current_track = n;
        }
    }
}

// Select which bank is enabled (tracklist selection).
static void select_bank() {
    if (history[0] >= 1 && history[0] <= BUTTONS_COUNT) {
        current_bank = history[0];
        DEBUG_PRINTF("bank changed: %i\n", current_bank);
        play_notification();
    }
}

// Select which playback mode to use.
static void select_mode() {
    switch (history[0]) {
        case 1:
            current_mode = MODE_KEYBOARD;
            break;
        case 2:
            current_mode = MODE_SAX;
            break;
    }

    if (current_mode != MODE_SELECT) {
        DEBUG_PRINTF("mode changed: %i\n", current_mode);
        play_notification();
    }
}

static void init_history(void) {
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history[i] = -1;
    }
}

static void shift_history(int value) {
    for (int i = HISTORY_SIZE - 1; i > 0; i--) {
        history[i] = history[i - 1];
    }
    history[0] = value;
}

static void poll_history(void) {
    bool event = false;
    int result = 0;

    for (int i = 0; i < BUTTONS_COUNT; i++) {
        if (button_was_pressed(&buttons[i])) {
            event = true;
        }

        if (button_pressed(&buttons[i])) {
            if (result == 0) {
                result = i + 1;
            } else {
                result = -1;
            }
        }
    }

    if (event && result != 0) {
        DEBUG_PRINTF("button pressed: %d\n", result);
        shift_history(result);
    }
}

static bool sequence_matches(const int *sequence) {
    for (size_t i = 0; i < HISTORY_SIZE; i++) {
        if (history[i] != sequence[i]) {
            return false;
        }
    }
    return true;
}

static void sald_a_boom(void) {
    if (sequence_matches(bank_sequence)) {
        DEBUG_PRINTF("entered bank selection mode\n");
        shift_history(-1); // break sequence to prevent re-entering this mode
        current_bank = BANK_SELECT;
        play_notification();
    } else if (sequence_matches(mode_sequence)) {
        DEBUG_PRINTF("entered mode selection mode\n");
        shift_history(-1); // break sequence to prevent re-entering this mode
        current_mode = MODE_SELECT;
        play_notification();
    } else {
        switch (current_mode) {
            case MODE_SAX:
                mode_sax();
                break;
            case MODE_TOGGLE:
                mode_toggle();
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
    init_buttons();
    init_led();
    init_history();

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
        poll_buttons();
        poll_history();

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
