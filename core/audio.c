#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"

#include "audio.h"

// PWM duty cycle settings for Raspberry Pi Pico.
//
// cpu_frequency / clock_divider / pwm_wrap = pwm_duty_cycle
// 125 MHz       / 5.5           / 256      = ~88.78 kHz
//
// This is important for 3 different reasons:
// - The hardware RC filter must filter out this frequency
// - It will determine which bits per sample is supported
// - It will determine which WAVE frequency is supported
#define PWM_CLKDIV 5.5f
#define PWM_WRAP   255u

// Halfway of PWM_WRAP
#define PWM_SILENCE 0x80

volatile int pin_slice;

AudioChunk chunks[2];

// Which chunk is currently playing
volatile unsigned int chunk_index = 0;

// chunks[chunk_index].data[data_index]
volatile unsigned int data_index = 0;

// Current status
volatile bool playing = false;

// volume: 0 = silent, 255 = full volume
volatile uint8_t volume = 128;

volatile uint32_t ticks_counter = 0;

uint8_t adjust_volume(uint8_t sample) {
    int16_t centered = (int16_t)sample - 128;
    centered = (centered * volume) / 255;
    return (uint8_t)(centered + 128);
}

// Main idea from:
// https://gregchadwick.co.uk/blog/playing-with-the-pico-pt3/
// https://github.com/GregAC/pico-stuff/tree/main/pwm_audio
void pwm_irh(void) {
    pwm_clear_irq(pin_slice);

    // Change the PWM output every 4 ticks (PWM frequency is 4x of WAVE frequency).
    if (++ticks_counter < 4) {
        return;
    }
    ticks_counter = 0;

    if (!playing) {
        pwm_set_gpio_level(PIN_AUDIO, PWM_SILENCE);
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
        pwm_set_gpio_level(
            PIN_AUDIO,
            adjust_volume(chunk->data[data_index++])
        );
    } else {
        pwm_set_gpio_level(PIN_AUDIO, PWM_SILENCE);
    }
}

void init_audio(void) {
    chunks[0].size = 0;
    chunks[1].size = 0;
    ticks_counter = 0;

    gpio_set_function(PIN_AUDIO, GPIO_FUNC_PWM);

    pin_slice = pwm_gpio_to_slice_num(PIN_AUDIO);

    pwm_clear_irq(pin_slice);
    pwm_set_irq_enabled(pin_slice, false);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_irh);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();

    pwm_config_set_clkdiv(&config, PWM_CLKDIV);

    pwm_config_set_wrap(&config, PWM_WRAP);

    pwm_init(pin_slice, &config, true);

    pwm_set_gpio_level(PIN_AUDIO, PWM_SILENCE);
}

bool audio_enabled(void) {
    return playing;
}

void enable_audio(void) {
    if (playing) {
        return;
    }
    playing = true;
    ticks_counter = 0;
    pwm_set_irq_enabled(pin_slice, true);
}

void disable_audio(bool reset) {
    if (!playing) {
        return;
    }

    // Mask at the NVIC so no handler can run while the state is reset.
    irq_set_enabled(PWM_IRQ_WRAP, false);

    pwm_set_irq_enabled(pin_slice, false);

    playing = false;
    if (reset) {
        ticks_counter = 0;
        chunks[0].size = 0;
        chunks[1].size = 0;
        chunk_index = 0;
        data_index = 0;
    }

    // Drop the wrap latched while masked, otherwise the next play_audio()
    // takes the handler immediately.
    pwm_clear_irq(pin_slice);

    pwm_set_gpio_level(PIN_AUDIO, PWM_SILENCE);

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

void volume_audio(uint8_t value) {
    volume = value;
}
