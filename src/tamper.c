#include "tamper.h"
#include "drivers_reed.h"

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "drivers_hw123.h"

bool last_motion_state = false;

bool tamper_detected(void) {
    bool moved = gyro_motion_has_moved();
    bool event = false;

    if (moved && !last_motion_state) {
        event = true;   // rising edge: device just moved
    }

    last_motion_state = moved;

    if (!event){
        return !door_closed();
    }

    return event;
}
