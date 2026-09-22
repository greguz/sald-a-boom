#pragma once

#include <stdio.h>

// TODO: make it configurable from CMake?
#define ENABLE_DEBUG 1

#if ENABLE_DEBUG
#define DEBUG_PRINTF(...) fprintf(stderr, "[DEBUG] " __VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif
