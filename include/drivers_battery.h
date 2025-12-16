#ifndef DRVIERS_BATTERY_H
#define DRIVERS_BATTERY_H

#include <stdint.h>
#include <stdbool.h>

void battery_init(void);
float battery_read_voltage(void);
float battery_read_percentage(void);
bool OnCooldown(void);

#endif