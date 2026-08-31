#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "audio.h"

#define SILENCE 0x80

int audio_pin_slice;

AudioChunk chunks[2];

// Which chunk is currently playing
volatile unsigned int chunk_index = 0;

// chunks[chunk_index].data[data_index]
volatile unsigned int data_index = 0;

// Current status
volatile bool audio_playing = false;

// Main idea from:
// https://gregchadwick.co.uk/blog/playing-with-the-pico-pt3/
// https://github.com/GregAC/pico-stuff/tree/main/pwm_audio
void pwm_irh(void) {
    pwm_clear_irq(audio_pin_slice);

    if (!audio_playing) {
        pwm_set_gpio_level(PIN_AUDIO, SILENCE);
        return;
    }

    AudioChunk *chunk = &chunks[chunk_index];

    if (chunk->size > 0 && data_index >= chunk->size) {
        chunk->size = 0;
        chunk_index ^= 1;
        data_index = 0;
        chunk = &chunks[chunk_index];
    }

    if (data_index < chunk->size) {
        pwm_set_gpio_level(PIN_AUDIO, chunk->data[data_index++]);
    } else {
        pwm_set_gpio_level(PIN_AUDIO, SILENCE);
    }
}

void init_audio(void) {
    chunks[0].size = 0;
    chunks[1].size = 0;

    gpio_set_function(PIN_AUDIO, GPIO_FUNC_PWM);

    audio_pin_slice = pwm_gpio_to_slice_num(PIN_AUDIO);

    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, false);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_irh);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();

    // It divides the Pico's PWM clock.
    //
    // 8-bit PCM 16 KHz target:
    //
    // 125 MHz
    // ÷ divider
    // ÷ 256
    // = 16 KHz
    //
    // divider = 30.517578125
    pwm_config_set_clkdiv(&config, 30.5f);

    // PWM value is a 8 bit value (from 0 to 255).
    pwm_config_set_wrap(&config, 255);

    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(PIN_AUDIO, SILENCE);
}

bool audio_enabled(void) {
    return audio_playing;
}

void enable_audio(void) {
    if (audio_playing) {
        return;
    }

    audio_playing = true;
    pwm_set_irq_enabled(audio_pin_slice, true);
}

void disable_audio(void) {
    if (!audio_playing) {
        return;
    }

    // Mask at the NVIC so no handler can run while the state is reset.
    irq_set_enabled(PWM_IRQ_WRAP, false);

    pwm_set_irq_enabled(audio_pin_slice, false);

    audio_playing = false;
    chunks[0].size = 0;
    chunks[1].size = 0;
    chunk_index = 0;
    data_index = 0;

    // Drop the wrap latched while masked, otherwise the next play_audio()
    // takes the handler immediately.
    pwm_clear_irq(audio_pin_slice);

    pwm_set_gpio_level(PIN_AUDIO, SILENCE);

    irq_set_enabled(PWM_IRQ_WRAP, true);
}

AudioChunk *request_audio(void) {
    unsigned int i = chunk_index;
    if (chunks[i].size == 0) {
        return &chunks[i];
    }
    if (chunks[i ^ 1].size == 0) {
        return &chunks[i ^ 1];
    }
    return NULL;
}

bool audio_drained(void) {
    return chunks[chunk_index].size == 0;
}
