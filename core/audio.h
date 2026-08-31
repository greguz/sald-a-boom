#pragma once

#include <stdbool.h>
#include <stdint.h>

// Use GPIO 6 to output audio (PWM).
#define PIN_AUDIO 6

// Bytes per chunk. Two chunks are kept in flight (double buffering).
#define AUDIO_BUFFER_SIZE 4096

typedef struct {
    // Binary audio to play.
    uint8_t data[AUDIO_BUFFER_SIZE];
    // How many bytes are valid inside `data` buffer.
    volatile unsigned int size;
} AudioChunk;

// Initialize lower-level audio hardware (PWM).
void init_audio(void);

// Enables audio output.
// You can request an audio buffer before calling this function.
void enable_audio(void);

// Mutes the audio and resets its internal state.
void disable_audio(void);

// Returns true when audio is enabled.
bool audio_enabled(void);

// Returns true when the internal audio buffer is empty.
bool audio_drained(void);

// Request an audio buffer to fill.
AudioChunk *request_audio(void);
