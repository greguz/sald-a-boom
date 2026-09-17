#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "ff.h"

// Initializes FS and PWM side.
bool init_player(void);

// Starts a WAVE file playback.
// Plays a quack sound if the file cannot be loaded for some reason.
bool play_wave(TCHAR* path);

bool is_playing(void);

bool is_paused(void);

void poll_player(void);

void stop_player(void);

void pause_player(void);

void resume_player(void);
