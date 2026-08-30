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
