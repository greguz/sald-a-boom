#pragma once

#include <stdbool.h>
#include <stdint.h>

// MAX4466 electret microphone board
#define GPIO_PRESSURE       26

// Potentiometer
#define GPIO_KNOB           27

// Time window in milliseconds
#define PRESSURE_INTERVAL   50

// Delta value threshold inside time window
#define PRESSURE_THRESHOLD  500

void init_adc(void);

// Returns true when pressure was detected.
bool has_pressure(void);

// Returns an integer value between 0 and 4095 (12 bit).
uint16_t read_knob(void);
