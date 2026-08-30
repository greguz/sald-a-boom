#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "spi.h"
#include "ff.h"
#include "audio.h"
#include "buttons.h"
#include "wav.h"

CoreAudio audio;

FATFS fs;

void test_wav(void) {
    FIL file;
    FRESULT res;
    UINT bytes_read;
    wav_header_t header;

    res = f_open(&file, "QUACK.WAV", FA_READ);

    if (res != FR_OK) {
        printf("f_open failed: %d\n", res);
        return;
    }

    res = f_read(&file, &header, sizeof(header), &bytes_read);

    if (res != FR_OK || bytes_read != sizeof(header)) {
        printf("Failed to read WAV header\n");
        f_close(&file);
        return;
    }

    printf("Format:       %c%c%c%c\n",
           header.riff[0], header.riff[1],
           header.riff[2], header.riff[3]);

    printf("File size:    %lu\n",
           (unsigned long)header.file_size);

    printf("Channels:     %u\n", header.num_channels);
    printf("Sample rate:  %lu Hz\n",
           (unsigned long)header.sample_rate);
    printf("Bits/sample:  %u\n", header.bits_per_sample);
    printf("Data size:    %lu\n",
           (unsigned long)header.data_size);

    f_close(&file);
}

void list_root_directory(void) {
    FRESULT res;
    DIR dir;
    FILINFO fno;

    res = f_opendir(&dir, "/");

    if (res != FR_OK) {
        printf("f_opendir failed: %d\n", res);
        return;
    }

    printf("Root directory:\n");

    while (1) {
        res = f_readdir(&dir, &fno);

        if (res != FR_OK) {
            printf("f_readdir failed: %d\n", res);
            break;
        }

        // End of directory
        if (fno.fname[0] == '\0') {
            break;
        }

        if (fno.fattrib & AM_DIR) {
            printf("[DIR]  %s\n", fno.fname);
        } else {
            printf("       %s  (%lu bytes)\n",
                   fno.fname,
                   (unsigned long)fno.fsize);
        }
    }

    f_closedir(&dir);
}

int main() {
    stdio_init_all();

    init_audio(&audio);

    init_buttons();

    // ---------------- Micro SD setup ----------------

    // TODO: main logic

    // TODO: play quack at startup

    FRESULT res = f_mount(&fs, "", 1);

    uint8_t buttons = 0x00;

    while (true) {
        printf("Hello, world!\n");

        buttons = read_buttons();
        if (buttons > 0) {
            play_audio(&audio);
        } else {
            stop_audio(&audio);
        }

        if (res == FR_OK) {
            list_root_directory();
            test_wav();
        } else {
            printf("f_mount failed: %d\n", res);
        }

        sleep_ms(1000);
    }
}
