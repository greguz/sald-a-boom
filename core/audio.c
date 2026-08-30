#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "audio.h"
#include "quack.h"

int audio_pin_slice;

// This is the current position in your audio data.
int sample_index = 0;

bool playing = false;

void pwm_irh() {
    pwm_clear_irq(audio_pin_slice);

    if (playing) {
        pwm_set_gpio_level(PIN_AUDIO, quack_buffer[sample_index++]);
        if (sample_index >= QUACK_SAMPLES) {
            sample_index = 0;
        }
    } else {
        pwm_set_gpio_level(PIN_AUDIO, 0x00);
    }
}

void init_audio(CoreAudio *audio) {
    audio->playing = false;

    gpio_set_function(PIN_AUDIO, GPIO_FUNC_PWM);

    audio_pin_slice = pwm_gpio_to_slice_num(PIN_AUDIO);

    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, true);
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
    pwm_config_set_clkdiv(&config, 31.0f);

    // PWM value is a 8 bit value (from 0 to 255).
    pwm_config_set_wrap(&config, 255);

    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(PIN_AUDIO, 0);
}

void play_audio(CoreAudio *audio) {
    if (!playing) {
        sample_index = 0;
        playing = true;
    }
}

void stop_audio(CoreAudio *audio) {
    playing = false;
}
