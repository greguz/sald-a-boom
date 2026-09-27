#include "button.h"

// Columns (pull-up digital output)
#define MATRIX_OUTPUTS  2

// Rows (digital input)
#define MATRIX_INPUTS   4

// Sleep between output state change (milliseconds)
#define MATRIX_SLEEP    1

// Total number of buttons
#define BUTTONS_COUNT   (MATRIX_OUTPUTS * MATRIX_INPUTS)

extern button_t buttons[BUTTONS_COUNT];

void init_buttons(void);

void poll_buttons(void);

// Returns `true` when any button has its state changed since last poll.
bool buttons_changed(void);
