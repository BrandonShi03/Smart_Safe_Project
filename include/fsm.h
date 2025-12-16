//This is FSM header file for smart safe final project for ece_414 @Lafayette college
//Author:Jackson Kaminski,Haoxuan Shi
//Date: Fall 2025

#ifndef FSM_H
#define FSM_H

#include "pico/stdlib.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_PIN_LENGTH 4

//api functions
void fsm_init(void);
void fsm_tick();
void check_and_update_password(void);
void show_next_update_time(void);

#endif