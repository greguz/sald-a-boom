#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "spi.h"
#include "ff.h"

#include "quack.h"

// Use GPIO 6 to output audio (PWM).
#define AUDIO_PIN 6

typedef struct __attribute__((packed)) {
    // RIFF header
    char     riff[4];              // "RIFF"
    uint32_t file_size;
    char     wave[4];              // "WAVE"

    // fmt chunk
    char     fmt[4];               // "fmt "
    uint32_t fmt_size;             // 16 for PCM
    uint16_t audio_format;         // 1 = PCM
    uint16_t num_channels;         // 1 = mono, 2 = stereo
    uint32_t sample_rate;          // e.g. 44100
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;      // e.g. 16

    // data chunk
    char     data[4];              // "data"
    uint32_t data_size;
} wav_header_t;

int audio_pin_slice;

// This is the current position in your audio data.
int cur_sample = 0;

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

void pwm_irh() {
    pwm_clear_irq(audio_pin_slice);

    pwm_set_gpio_level(AUDIO_PIN, quack_buffer[cur_sample++]);

    if (cur_sample >= QUACK_SAMPLES) {
        cur_sample = 0;
    }
}

int main() {
    stdio_init_all();

    // ---------------- Audio setup ----------------

    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);

    audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_irh);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();

    // It divides the Pico's PWM clock.
    //
    // 8-bit PCM 16 KHz:
    //
    // 125 MHz
    // ÷ divider
    // ÷ 256
    // = 16 KHz
    //
    // divider = 30.517578125
    pwm_config_set_clkdiv(&config, 31.0f);

    // PWM value is a 8 bit value (from 0 to 255).
    pwm_config_set_wrap(&config, 255);

    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(AUDIO_PIN, 0);

    // ---------------- Micro SD setup ----------------

    // TODO: main logic

    // TODO: play quack at startup


    FRESULT res = f_mount(&fs, "", 1);

    while (true) {
        printf("Hello, world!\n");

        if (res == FR_OK) {
            list_root_directory();
            test_wav();
        } else {
            printf("f_mount failed: %d\n", res);
        }

        sleep_ms(1000);
    }
}
