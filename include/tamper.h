#ifndef TAMPER_HW123_H
#define TAMPER_HW123_H

#include <stdbool.h>
#include <stdint.h>


bool motion_hw123_init(void);

bool motion_hw123_has_moved(void);

bool tamper_detected(void);

#endif 
