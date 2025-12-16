#ifndef DRIVERS_BUZZER_H
#define DRIVERS_BUZZER_H

#include <stdbool.h>
#include <stdint.h>

#include "board_pins.h"

// ---------------------------------------------------------
void buzzer_init(void);

/**
 * @brief Turn the buzzer on (active HIGH).
 */
void buzzer_on(void);

/**
 * @brief Turn the buzzer off.
 */
void buzzer_off(void);

#endif // DRIVERS_BUZZER_H
