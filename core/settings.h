#pragma once

#include <stdbool.h>
#include <stdint.h>

// Magic = "SAB!" (Sald-A-Boom!)
#define SETTINGS_MAGIC 0x53414221u

typedef struct {
    uint32_t    magic;
    uint8_t     version;
    int         mode;
    int         bank;
} settings_t;

void save_settings(const settings_t *settings);

// Returns `true` when settings are correctly load and valid.
bool load_settings(settings_t *settings);
