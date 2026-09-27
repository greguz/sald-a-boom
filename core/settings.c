#include "settings.h"

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

// TODO: redefined
#define FLASH_PAGE_SIZE     256

// TODO: redefined
#define FLASH_SECTOR_SIZE   4096

// TODO: use a LD file
#define FLASH_TARGET_OFFSET (2 * 1024 * 1024 - FLASH_SECTOR_SIZE)  // Example: 2 MB from flash start

void save_settings(const settings_t *settings) {
    // flash_range_erase/program must not be called while executing
    // code from the same flash.
    uint32_t ints = save_and_disable_interrupts();

    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);

    flash_range_program(
        FLASH_TARGET_OFFSET,
        (const uint8_t *)settings,
        sizeof(settings_t)
    );

    restore_interrupts(ints);
}

bool load_settings(settings_t *settings) {
    const uint8_t *flash_data =
        (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

    memcpy(settings, flash_data, sizeof(settings_t));

    return settings->magic == SETTINGS_MAGIC;
}
