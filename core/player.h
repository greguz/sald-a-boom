#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "ff.h"

// Initializes FS and PWM side.
bool init_player(void);

// Starts a WAVE file playback.
// Plays a quack sound if the file cannot be loaded for some reason.
void play_wave(TCHAR* path);

// Returns true when it's still playing something.
bool is_playing(void);

void poll_player(void);
