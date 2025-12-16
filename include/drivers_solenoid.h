#ifndef DRIVERS_SOLENOID_H
#define DRIVERS_SOLENOID_H

#include <stdbool.h>
#include <stdint.h>

// Initialize the solenoid driver.
void solenoid_init(uint8_t gpio_pin);

// Set the default pulse duration used when engage(0) is called.
void solenoid_set_default_pulse_ms(uint32_t ms);

// Engage the solenoid. If pulse_ms > 0, it will auto-disengage after pulse_ms.
// If pulse_ms == 0, it uses the default set via solenoid_set_default_pulse_ms().
// If you want a continuous hold, call solenoid_engage_hold().
void solenoid_engage(uint32_t pulse_ms);

// Engage and hold (no auto-release) until solenoid_disengage() is called.
void solenoid_engage_hold(void);

// Manually release the solenoid.
void solenoid_disengage(void);

// Must be called periodically (e.g., in main loop). Handles auto-release timing.
void solenoid_update(void);

// Query last commanded state (true = engaged).
bool solenoid_is_engaged(void);

#endif
