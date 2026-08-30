// Use GPIO 6 to output audio (PWM).
#define PIN_AUDIO 6

typedef struct {
    bool playing;
} CoreAudio;

void init_audio(CoreAudio *audio);

void play_audio(CoreAudio *audio);

void stop_audio(CoreAudio *audio);
