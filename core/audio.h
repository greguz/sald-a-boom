#pragma once

#include <stdbool.h>
#include <stdint.h>

// Use GPIO 6 to output audio (PWM).
#define PIN_AUDIO 6

// Bytes per chunk. Two chunks are kept in flight (double buffering).
#define AUDIO_BUFFER_SIZE 4096

typedef struct {
    // Binary audio to play
    uint8_t data[AUDIO_BUFFER_SIZE];
    //
    volatile unsigned int size;
} AudioChunk;

void init_audio(void);

void play_audio(void);

void stop_audio(void);

bool is_playing(void);

AudioChunk *grab_chunk(void);
