#include <drivers_reed.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <board_pins.h>

void reed_init(void){
    gpio_init(REED_PIN);
    gpio_set_dir(REED_PIN, GPIO_IN);

    gpio_pull_up(REED_PIN);

    gpio_init(ONBOARD_LED);
    gpio_set_dir(ONBOARD_LED, GPIO_OUT);
}

bool door_closed(void){
    bool reed_level = gpio_get(REED_PIN);
    bool magnet_present = (reed_level == 0);

    return magnet_present;
}
