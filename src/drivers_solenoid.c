#include "drivers_solenoid.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

static uint8_t  s_pin         = 0xFF;
static bool     s_active_high = false;
static bool     s_engaged     = false;

static bool     s_auto_release_enabled = false;
static uint32_t s_default_pulse_ms     = 250;   // safe default
static uint64_t s_release_deadline_ms  = 0;     // absolute time in ms since boot

static inline void drive_hw(bool on) {
    bool level = s_active_high ? on : !on;
    gpio_put(s_pin, level);
    s_engaged = on;
}

void solenoid_init(uint8_t gpio_pin) {
    s_pin = gpio_pin;

    gpio_init(s_pin);
    gpio_set_dir(s_pin, GPIO_OUT);
    gpio_pull_down(s_pin);          // helps keep gate low at boot if wired direct
    drive_hw(false);                // ensure off
}

void solenoid_set_default_pulse_ms(uint32_t ms) {
    s_default_pulse_ms = ms;
}

void solenoid_engage(uint32_t pulse_ms) {
    if (pulse_ms == 0) pulse_ms = s_default_pulse_ms;

    drive_hw(true);

    // Arm auto-release
    s_auto_release_enabled = true;
    s_release_deadline_ms  = to_ms_since_boot(get_absolute_time()) + (uint64_t)pulse_ms;
}

void solenoid_engage_hold(void) {
    drive_hw(true);
    s_auto_release_enabled = false; // no timed release
}

void solenoid_disengage(void) {
    drive_hw(false);
    s_auto_release_enabled = false;
}

void solenoid_update(void) {
    if (!s_engaged || !s_auto_release_enabled) return;

    uint64_t now = to_ms_since_boot(get_absolute_time());
    if (now >= s_release_deadline_ms) {
        drive_hw(false);                // auto-release
        s_auto_release_enabled = false; // disarm
    }
}

bool solenoid_is_engaged(void) {
    return s_engaged;
}
