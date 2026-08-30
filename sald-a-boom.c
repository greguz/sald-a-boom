#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "spi.h"
#include "ff.h"
#include "audio.h"
#include "buttons.h"
#include "wav.h"

FATFS fs;

FIL file;

volatile bool file_open = false;

void close_file(void) {
    if (file_open) {
        f_close(&file);
        file_open = false;
    }
}

bool open_file(void) {
    if (file_open) {
        close_file();
    }
    file_open = f_open(&file, "AIHB.WAV", FA_READ) == FR_OK;
    return file_open;
}

bool send_audio(void) {
    if (!file_open) {
        if (!open_file()) {
            return false;
        }
        if (f_lseek(&file, sizeof(wav_header_t)) != FR_OK) {
            close_file();
            return false;
        }
    }

    AudioChunk *chunk = grab_chunk();
    if (chunk == NULL) {
        return false;
    }

    unsigned int size = 0;
    if (f_read(&file, chunk->data, AUDIO_BUFFER_SIZE, &size) != FR_OK) {
        close_file();
        return false;
    }

    if (size == 0) {
        close_file();
        return false;
    }

    chunk->size = size;
    play_audio();
    return true;
}

int main() {
    stdio_init_all();

    init_audio();

    init_buttons();

    // ---------------- Micro SD setup ----------------

    // TODO: main logic

    // TODO: play quack at startup

    FRESULT res = f_mount(&fs, "", 1);

    uint8_t buttons = 0x00;

    while (true) {

        if (res == FR_OK) {
            send_audio();
        } else {
            printf("f_mount failed: %d\n", res);
        }

        // buttons = read_buttons();
        // if (buttons > 0 && !is_playing()) {
        //     play_audio("TRACK_01.WAV");
        // }

    }
}
