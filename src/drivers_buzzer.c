#include "drivers_buzzer.h"

#include "pico/stdlib.h"
#include "pico/time.h"
#include "board_pins.h"

static repeating_timer_t buzzer_timer;
static bool buzzer_timer_active = false;
static bool buzzer_level = false;

// Timer callback: toggle the buzzer pin.
// Runs at a fixed interval: 200 µs -> 2.5 kHz tone.
static bool buzzer_timer_cb(repeating_timer_t *rt) {
    (void)rt;
    buzzer_level = !buzzer_level;
    gpio_put(BUZZER_PIN, buzzer_level);
    return true;  // keep repeating
}

void buzzer_init(void) {
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);
    buzzer_level = false;
    buzzer_timer_active = false;
}

void buzzer_on(void) {
    if (buzzer_timer_active) {
        return; // already on
    }

    // -200 µs = 2.5 kHz (like your working test)
    if (add_repeating_timer_us(-200, buzzer_timer_cb, NULL, &buzzer_timer)) {
        buzzer_timer_active = true;
    }
}

void buzzer_off(void) {
    if (buzzer_timer_active) {
        cancel_repeating_timer(&buzzer_timer);
        buzzer_timer_active = false;
    }
    buzzer_level = false;
    gpio_put(BUZZER_PIN, 0);
}
