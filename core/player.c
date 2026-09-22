#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "audio.h"
#include "debug.h"
#include "ff.h"
#include "player.h"
#include "quack.h"

// http://soundfile.sapp.org/doc/WaveFormat/
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

static FATFS fs;

static FIL file;

// Toggles WAVE file streaming (has precedence over quacking)
static volatile bool file_open = false;

// Toggles the quack sound if less then QUACK_SAMPLES
static volatile unsigned int quack_offset = QUACK_SAMPLES;

// Player status.
static volatile bool player_paused = false;

bool init_player(void) {
    FRESULT res;

    init_audio();

    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        DEBUG_PRINTF("f_mount failed: %d\n", res);
        return false;
    }

    return true;
}

static void close_file(void) {
    if (file_open) {
        f_close(&file);
        file_open = false;
    }
}

static bool open_file(const TCHAR* path) {
    if (file_open) {
        close_file();
    }
    file_open = f_open(&file, path, FA_READ) == FR_OK;
    return file_open;
}

static bool open_wave(const TCHAR* path) {
    if (!open_file(path)) {
        return false;
    }

    wav_header_t header;
    UINT bytes_read = 0;
    FRESULT res = f_read(&file, &header, sizeof(header), &bytes_read);
    if (res != FR_OK || bytes_read != sizeof(header)) {
        close_file();
        return false;
    }

    // Allow supported WAVE format only
    if (
        memcmp(header.riff, "RIFF", 4) != 0 ||
        memcmp(header.wave, "WAVE", 4) != 0 ||
        memcmp(header.fmt, "fmt ", 4) != 0 ||
        header.fmt_size != 16 ||                    // PCM
        header.audio_format != 1 ||                 // PCM
        header.num_channels != 1 ||                 // mono
        header.sample_rate != 22050 ||              // 22 KHz
        header.bits_per_sample != 8 ||              // 8 bit
        memcmp(header.data, "data", 4) != 0
    ) {
        close_file();
        return false;
    }

    return true;
}

static void poll_wave(audio_chunk_t *chunk) {
    UINT bytes_read = 0;
    FRESULT res = f_read(&file, chunk->data, AUDIO_BUFFER_SIZE, &bytes_read);

    if (res == FR_OK && bytes_read > 0) {
        chunk->size = bytes_read;
    } else {
        close_file();
    }
}

static void poll_quack(audio_chunk_t *chunk) {
    unsigned int length = QUACK_SAMPLES - quack_offset;
    if (length > AUDIO_BUFFER_SIZE) {
        length = AUDIO_BUFFER_SIZE;
    }

    memcpy(chunk->data, &quack_buffer[quack_offset], length);

    quack_offset += length;
    chunk->size = length;
}

void poll_player(void) {
    if (!audio_enabled()) {
        return;
    }

    audio_chunk_t *chunk = request_audio();
    if (chunk == NULL) {
        // Waiting for next chunk to send
        return;
    }

    if (file_open) {
        // Streaming WAVE file
        poll_wave(chunk);
    } else if (quack_offset < QUACK_SAMPLES) {
        // Still quacking
        poll_quack(chunk);
    } else if (audio_drained()) {
        // Audio was fully playied
        disable_audio(true);
    }
}

bool is_playing(void) {
    return audio_enabled() && !player_paused && (
        file_open ||
        quack_offset < QUACK_SAMPLES ||
        !audio_drained()
    );
}

bool play_wave(const TCHAR* path) {
    bool result;

    if (audio_enabled()) {
        disable_audio(true);
    }

    result = open_wave(path);

    if (!result) {
        // Starts the quacking!
        quack_offset = 0;
    }
    enable_audio();

    return result;
}

void stop_player(void) {
    disable_audio(true);
    close_file();
    quack_offset = QUACK_SAMPLES;
    player_paused = false;
}

bool is_paused(void) {
    return player_paused;
}

void pause_player(void) {
    if (!is_playing()) {
        return;
    }
    player_paused = true;
    disable_audio(false);
}

void resume_player(void) {
    if (!player_paused) {
        return;
    }
    player_paused = false;
    enable_audio();
}

void set_volume(uint8_t value) {
    if (value < MIN_VOLUME) {
        volume_audio(MIN_VOLUME);
    } else {
        volume_audio(value);
    }
}
