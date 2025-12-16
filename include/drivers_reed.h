#ifndef DRIVERS_REED_H
#define DRIVERS_REED_H

#include <stdio.h>
#include <stdbool.h>

void reed_init(void);
bool door_closed(void);

#endif